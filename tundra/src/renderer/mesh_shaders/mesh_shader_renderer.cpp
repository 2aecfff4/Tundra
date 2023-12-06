#include "renderer/mesh_shaders/mesh_shader_renderer.h"
#include "core/std/shared_ptr.h"
#include "math/matrix4.h"
#include "math/vector4.h"
#include "renderer/frame_graph/frame_graph.h"
#include "renderer/material_pass.h"
#include "renderer/mesh_shaders/instance_culling.h"
#include "renderer/mesh_shaders/mesh_shader.h"
#include "renderer/mesh_shaders/task_dispatch_command_generator.h"
#include "renderer/ubo.h"
#include "rhi/commands/command_encoder.h"
#include "rhi/rhi_context.h"
#include "shader.h"
#include <array>

namespace tundra::renderer::mesh_shaders {

///
RenderOutput mesh_shader_renderer(
    frame_graph::FrameGraph& fg, const RenderInput& input) noexcept
{
    const usize instance_count = input.mesh_instances.size();

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

    const mesh_shaders::InstanceCullingOutput instance_culling =
        mesh_shaders::instance_culling(
            fg,
            mesh_shaders::InstanceCullingInput {
                .ubo_buffer = ubo_data.ubo_buffer,
                .frustum_planes = frustum_planes,
                .num_instances = static_cast<u32>(instance_count),
                .mesh_descriptors = input.gpu_mesh_descriptors,
                .mesh_instances = input.gpu_mesh_instances,
                .mesh_instance_transforms = input.gpu_mesh_instance_transforms,
                .compute_pipelines = input.compute_pipelines,
            });

    const mesh_shaders::TaskDispatchCommandGeneratorOutput
        task_dispatch_command_generator = mesh_shaders::task_dispatch_command_generator(
            fg,
            mesh_shaders::TaskDispatchCommandGeneratorInput {
                .ubo_buffer = ubo_data.ubo_buffer,
                .command_count = instance_culling.command_count,
                .compute_pipelines = input.compute_pipelines,
            });

    const mesh_shaders::MeshShaderOutput mesh_shader = mesh_shaders::mesh_shader(
        fg,
        mesh_shaders::MeshShaderInput {
            .ubo_buffer = ubo_data.ubo_buffer,
            .view_size = input.view_size,
            .view_to_clip = input.view_to_clip,
            .world_to_view = input.world_to_view,
            .frustum_planes = frustum_planes,
            .camera_position = input.camera_position,
            .mesh_descriptors = input.gpu_mesh_descriptors,
            .mesh_instances = input.gpu_mesh_instances,
            .mesh_instance_transforms = input.gpu_mesh_instance_transforms,
            .command_count = instance_culling.command_count,
            .command_buffer = instance_culling.command_buffer,
            .task_shader_dispatch_args = task_dispatch_command_generator
                                             .task_shader_dispatch_args,
            .graphics_pipelines = input.graphics_pipelines,

        });

    return RenderOutput {
        .color_output = mesh_shader.visibility_buffer,
    };

    // const passes::MaterialOutput material_output = passes::material(
    //     fg,
    //     passes::MaterialInput {
    //         .ubo_buffer = ubo_data.ubo_buffer,
    //         .world_to_clip = frustum,
    //         .camera_position = input.camera_position,
    //         .light_position = math::Vec3 { -5, 2, 5 },
    //         // .diffuse_color = math::Vec3 { 0.5f, 0.f, 0.125f },
    //         .diffuse_color = math::Vec3 { 1.0f, 0.765557f, 0.336057f },
    //         .light_color = math::Vec3 { 1, 1, 1 },
    //         .view_size = input.view_size,
    //         .mesh_descriptors = input.gpu_mesh_descriptors,
    //         .mesh_instance_transforms = input.gpu_mesh_instance_transforms,
    //         .visible_meshlets = meshlet_culling.visible_meshlets,
    //         .vis_depth = gpu_rasterizer.vis_texture,
    //         .compute_pipelines = input.compute_pipelines,
    //     });

    // return RenderOutput {
    //     .color_output = material_output.color_texture,
    // };
}

} // namespace tundra::renderer::mesh_shaders
