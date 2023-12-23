#include "renderer/software/software_rasterizer.h"
#include "core/std/shared_ptr.h"
#include "math/matrix4.h"
#include "math/vector4.h"
#include "renderer/common/culling/instance_culling_and_lod.h"
#include "renderer/common/culling/meshlet_culling.h"
#include "renderer/frame_graph/frame_graph.h"
#include "renderer/material_pass.h"
#include "renderer/software/passes/gpu_rasterize_debug_pass.h"
#include "renderer/software/passes/gpu_rasterizer.h"
#include "renderer/software/passes/gpu_rasterizer_init.h"
#include "renderer/ubo.h"
#include "rhi/commands/command_encoder.h"
#include "rhi/rhi_context.h"
#include "shader.h"
#include <array>
#include <cstddef>

namespace tundra::renderer::software {

///
RenderOutput software_rasterizer(
    frame_graph::FrameGraph& fg, const RenderInput& input) noexcept
{
    const usize instance_count = input.mesh_instances.size();

    const u32 max_meshlet_count = [&] {
        u32 count = 0;
        for (const MeshInstance& mesh_instance : input.mesh_instances) {
            count += input.mesh_descriptors[mesh_instance.mesh_descriptor_index]
                         .meshlet_count;
        }

        return count;
    }();

    const math::Mat4 frustum = input.view_to_clip * input.world_to_view;
    const math::Mat4 frustum_t = math::transpose(frustum);

    // Fast Extraction of Viewing Frustum Planes from the World-View-Projection Matrix - Gil Gribb and Klaus Hartmann
    const std::array<math::Vec4, 6> frustum_planes {
        math::normalize(-frustum_t[3] - frustum_t[0]), //
        math::normalize(-frustum_t[3] + frustum_t[0]), //
        math::normalize(-frustum_t[3] - frustum_t[1]), //
        math::normalize(-frustum_t[3] + frustum_t[1]), //
        math::normalize(-frustum_t[3] - frustum_t[2]), //
        math::normalize(-frustum_t[3] + frustum_t[2]),
    };

    struct UboData {
        core::SharedPtr<UboBuffer> ubo_buffer;
    };

    const UboData ubo_data = fg.add_pass(
        frame_graph::QueueType::Graphics,
        "create_ubo",
        [&](frame_graph::Builder& builder) {
            constexpr u64 ubo_size = 1024ull * 1024;
            const auto ubo_buffer = builder.create_buffer(
                "ubo_buffer",
                frame_graph::BufferCreateInfo {
                    .usage = frame_graph::BufferUsageFlags::SRV,
                    .memory_type = frame_graph::MemoryType::Dynamic,
                    .size = ubo_size,
                });

            builder.write(
                ubo_buffer,
                frame_graph::ResourceUsage::SHADER_COMPUTE |
                    frame_graph::ResourceUsage::SHADER_GRAPHICS);

            return UboData {
                .ubo_buffer = core::make_shared<UboBuffer>(ubo_buffer, ubo_size),
            };
        },
        [=](rhi::IRHIContext*,
            const frame_graph::Registry&,
            rhi::CommandEncoder&,
            const UboData&) {

        });

    const common::culling::InstanceCullingOutput instance_culling =
        common::culling::instance_culling_and_lod(
            fg,
            common::culling::InstanceCullingInput {
                .ubo_buffer = ubo_data.ubo_buffer,
                .frustum_planes = frustum_planes,
                .instance_count = static_cast<u32>(instance_count),
                .mesh_descriptors = input.gpu_mesh_descriptors,
                .mesh_instances = input.gpu_mesh_instances,
                .mesh_instance_transforms = input.gpu_mesh_instance_transforms,
                .compute_pipelines = input.compute_pipelines,
            });

    const common::culling::MeshletCullingOutput meshlet_culling =
        common::culling::meshlet_culling(
            fg,
            common::culling::MeshletCullingInput {
                .ubo_buffer = ubo_data.ubo_buffer,
                .frustum_planes = frustum_planes,
                .camera_position = input.camera_position,
                .world_to_view = input.world_to_view,
                .max_meshlet_count = max_meshlet_count,
                .mesh_descriptors = input.gpu_mesh_descriptors,
                .mesh_instance_transforms = input.gpu_mesh_instance_transforms,
                .visible_instances = instance_culling.visible_instances,
                .meshlet_culling_dispatch_args = instance_culling
                                                     .meshlet_culling_dispatch_args,
                .compute_pipelines = input.compute_pipelines,
            });

    const passes::GpuRasterizerInitOutput gpu_rasterizer_init =
        passes::gpu_rasterizer_init_pass(
            fg,
            passes::GpuRasterizerInitInput {
                .ubo_buffer = ubo_data.ubo_buffer,
                .view_size = input.view_size,
                .visible_meshlets_count = meshlet_culling.visible_meshlets_count,
                .compute_pipelines = input.compute_pipelines,
            });

    const passes::GpuRasterizerOutput gpu_rasterizer = passes::gpu_rasterizer_pass(
        fg,
        passes::GpuRasterizerInput {
            .ubo_buffer = ubo_data.ubo_buffer,
            .world_to_clip = frustum,
            .view_size = input.view_size,
            .mesh_descriptors = input.gpu_mesh_descriptors,
            .mesh_instance_transforms = input.gpu_mesh_instance_transforms,
            .visible_meshlets = meshlet_culling.visible_meshlets,
            .visible_meshlets_count = meshlet_culling.visible_meshlets_count,
            .dispatch_indirect_args = gpu_rasterizer_init.gpu_rasterizer_dispatch_args,
            .vis_texture = gpu_rasterizer_init.vis_texture,
            .compute_pipelines = input.compute_pipelines,
        });

    if (input.show_meshlets) {
        const passes::GpuRasterizeDebugOutput debug_output =
            passes::gpu_rasterize_debug_pass(
                fg,
                passes::GpuRasterizeDebugInput {
                    .ubo_buffer = ubo_data.ubo_buffer,
                    .view_size = input.view_size,
                    .vis_depth = gpu_rasterizer.vis_texture,
                    .compute_pipelines = input.compute_pipelines,
                });
        return RenderOutput {
            .color_output = debug_output.debug_texture,
        };
    } else {
        const passes::MaterialOutput material_output = passes::material(
            fg,
            passes::MaterialInput {
                .ubo_buffer = ubo_data.ubo_buffer,
                .world_to_clip = frustum,
                .camera_position = input.camera_position,
                .light_position = math::Vec3 { -5, 2, 5 },
                // .diffuse_color = math::Vec3 { 0.5f, 0.f, 0.125f },
                .diffuse_color = math::Vec3 { 1.0f, 0.765557f, 0.336057f },
                .light_color = math::Vec3 { 1, 1, 1 },
                .view_size = input.view_size,
                .mesh_descriptors = input.gpu_mesh_descriptors,
                .mesh_instance_transforms = input.gpu_mesh_instance_transforms,
                .visible_meshlets = meshlet_culling.visible_meshlets,
                .vis_depth = gpu_rasterizer.vis_texture,
                .compute_pipelines = input.compute_pipelines,
            });

        return RenderOutput {
            .color_output = material_output.color_texture,
        };
    }
}

} // namespace tundra::renderer::software
