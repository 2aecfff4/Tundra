#include "renderer/software/software_rasterizer.h"
#include "math/matrix4.h"
#include "math/vector4.h"
#include "renderer/common/culling/instance_culling_and_lod.h"
#include "renderer/common/culling/meshlet_culling.h"
#include "renderer/frame_graph/frame_graph.h"
#include "renderer/software/passes/gpu_rasterize_debug_pass.h"
#include "renderer/software/passes/gpu_rasterizer.h"
#include "renderer/software/passes/gpu_rasterizer_init.h"
#include "rhi/commands/command_encoder.h"
#include "rhi/rhi_context.h"
#include "shader.h"
#include <array>
#include <cstddef>

namespace tundra::renderer {

///
[[nodiscard]] RenderOutput render(
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

    const std::array<math::Vec4, 6> frustum_planes {
        math::normalize(-frustum_t[3] - frustum_t[0]), //
        math::normalize(-frustum_t[3] + frustum_t[0]), //
        math::normalize(-frustum_t[3] - frustum_t[1]), //
        math::normalize(-frustum_t[3] + frustum_t[1]), //
        math::normalize(-frustum_t[3] - frustum_t[2]), //
        math::normalize(-frustum_t[3] + frustum_t[2]),
    };

    struct UboData {
        frame_graph::BufferHandle ubo_buffer;
        u64 ubo_buffer_offset = 0;
    };

    const UboData ubo_data = fg.add_pass(
        frame_graph::QueueType::Graphics,
        "create_ubo",
        [&](frame_graph::Builder& builder) {
            UboData data {};

            data.ubo_buffer = builder.create_buffer(
                "ubo_buffer",
                frame_graph::BufferCreateInfo {
                    .usage = frame_graph::BufferUsageFlags::SRV,
                    .memory_type = frame_graph::MemoryType::Dynamic,
                    .size = static_cast<u64>(1024 * 1024),
                });

            builder.write(
                data.ubo_buffer,
                frame_graph::ResourceUsage::SHADER_COMPUTE |
                    frame_graph::ResourceUsage::SHADER_GRAPHICS);

            return data;
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
                .frustum_planes = frustum_planes,
                .instance_count = static_cast<u32>(instance_count),
                .mesh_descriptors = input.gpu_mesh_descriptors,
                .mesh_instances = input.gpu_mesh_instances,
                .mesh_instance_transforms = input.gpu_mesh_instance_transforms,
                .ubo_buffer = ubo_data.ubo_buffer,
                .ubo_buffer_offset = ubo_data.ubo_buffer_offset,
                .compute_pipelines = input.compute_pipelines,
            });

    const common::culling::MeshletCullingOutput meshlet_culling =
        common::culling::meshlet_culling(
            fg,
            common::culling::MeshletCullingInput {
                .frustum_planes = frustum_planes,
                .world_to_view = input.world_to_view,
                .max_meshlet_count = max_meshlet_count,
                .mesh_descriptors = input.gpu_mesh_descriptors,
                .mesh_instance_transforms = input.gpu_mesh_instance_transforms,
                .ubo_buffer = instance_culling.ubo_buffer,
                .ubo_buffer_offset = instance_culling.ubo_buffer_offset,
                .visible_instances = instance_culling.visible_instances,
                .meshlet_culling_dispatch_args = instance_culling
                                                     .meshlet_culling_dispatch_args,
                .compute_pipelines = input.compute_pipelines,
            });

    const passes::GpuRasterizerInitOutput gpu_rasterizer_init =
        passes::gpu_rasterizer_init_pass(
            fg,
            passes::GpuRasterizerInitInput {
                .view_size = input.view_size,
                .ubo_buffer = meshlet_culling.ubo_buffer,
                .ubo_buffer_offset = meshlet_culling.ubo_buffer_offset,
                .visible_meshlets_count = meshlet_culling.visible_meshlets_count,
                .compute_pipelines = input.compute_pipelines,
            });

    const passes::GpuRasterizerOutput gpu_rasterizer = passes::gpu_rasterizer_pass(
        fg,
        passes::GpuRasterizerInput {
            .world_to_clip = frustum,
            .view_size = input.view_size,
            .mesh_descriptors = input.gpu_mesh_descriptors,
            .mesh_instance_transforms = input.gpu_mesh_instance_transforms,
            .ubo_buffer = gpu_rasterizer_init.ubo_buffer,
            .ubo_buffer_offset = gpu_rasterizer_init.ubo_buffer_offset,
            .visible_meshlets = meshlet_culling.visible_meshlets,
            .dispatch_indirect_args = gpu_rasterizer_init.gpu_rasterizer_dispatch_args,
            .vis_texture = gpu_rasterizer_init.vis_texture,
            .compute_pipelines = input.compute_pipelines,
        });

    const passes::GpuRasterizeDebugOutput debug_output = passes::gpu_rasterize_debug_pass(
        fg,
        passes::GpuRasterizeDebugInput {
            .view_size = input.view_size,
            .ubo_buffer = gpu_rasterizer.ubo_buffer,
            .ubo_buffer_offset = gpu_rasterizer.ubo_buffer_offset,
            .vis_depth = gpu_rasterizer.vis_texture,
            .compute_pipelines = input.compute_pipelines,
        });

    return RenderOutput {
        .color_output = debug_output.debug_texture,
    };
}

} // namespace tundra::renderer
