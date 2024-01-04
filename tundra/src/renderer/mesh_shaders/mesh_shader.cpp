#include "renderer/mesh_shaders/mesh_shader.h"
#include "pipelines.h"
#include "renderer/frame_graph/resources/enums.h"
#include "renderer/helpers.h"
#include "rhi/commands/draw_mesh_tasks_indirect.h"
#include "rhi/resources/handle.h"
#include "rhi/rhi_context.h"
#include <array>

namespace tundra::renderer::mesh_shaders {

namespace ubo {

///
struct TaskShaderUbo {
    math::Mat4 view_to_clip = math::Mat4 {};
    math::Mat4 world_to_view = math::Mat4 {};
    std::array<math::Vec4, config::NUM_PLANES> frustum_planes = {};
    math::Vec3 camera_position = {};

    struct {
        u32 mesh_descriptors_srv = config::INVALID_SHADER_HANDLE;
        u32 mesh_instances_srv = config::INVALID_SHADER_HANDLE;
        u32 mesh_instance_transforms_srv = config::INVALID_SHADER_HANDLE;

        u32 command_count_srv = config::INVALID_SHADER_HANDLE;
        u32 command_buffer_srv = config::INVALID_SHADER_HANDLE;
    } in_;
};

} // namespace ubo

///
MeshShaderOutput mesh_shader(
    frame_graph::FrameGraph& fg, const MeshShaderInput& input) noexcept
{
    struct Data {
        core::SharedPtr<UboBuffer> ubo_buffer;

        frame_graph::BufferHandle command_count;
        frame_graph::BufferHandle command_buffer;
        frame_graph::BufferHandle task_shader_dispatch_args;

        frame_graph::TextureHandle visibility_buffer;
        frame_graph::TextureHandle depth_buffer;
    };

    const Data data = fg.add_render_pass(
        frame_graph::QueueType::Graphics,
        "mesh_shader",
        [&](frame_graph::Builder& builder, frame_graph::RenderPass& render_pass) {
            Data data {};

            data.ubo_buffer = input.ubo_buffer;

            data.command_count = input.command_count;
            data.command_buffer = input.command_buffer;
            data.task_shader_dispatch_args = input.task_shader_dispatch_args;

            data.command_count //
                = builder.read(
                    data.command_count,
                    frame_graph::BufferResourceUsage::COMPUTE_STORAGE_BUFFER);
            data.command_buffer //
                = builder.read(
                    data.command_buffer,
                    frame_graph::BufferResourceUsage::COMPUTE_STORAGE_BUFFER);
            data.task_shader_dispatch_args //
                = builder.read(
                    data.task_shader_dispatch_args,
                    frame_graph::BufferResourceUsage::INDIRECT_BUFFER);

            data.visibility_buffer = builder.write(
                builder.create_texture(
                    "mesh_shader.visibility_buffer",
                    frame_graph::TextureCreateInfo {
                        .kind =
                            frame_graph::TextureKind::Texture2D {
                                .width = input.view_size.x,
                                .height = input.view_size.y,
                            },
                        .memory_type = frame_graph::MemoryType::GPU,
                        .format = frame_graph::TextureFormat::R8_G8_B8_A8_UNORM,
                        .usage = frame_graph::TextureUsageFlags::COLOR_ATTACHMENT |
                                 frame_graph::TextureUsageFlags::TRANSFER_SOURCE |
                                 frame_graph::TextureUsageFlags::PRESENT,
                        .tiling = frame_graph::TextureTiling::Optimal,
                    }),
                frame_graph::TextureResourceUsage::COLOR_ATTACHMENT);

            data.depth_buffer = builder.write(
                builder.create_texture(
                    "mesh_shader.depth_buffer",
                    frame_graph::TextureCreateInfo {
                        .kind =
                            frame_graph::TextureKind::Texture2D {
                                .width = input.view_size.x,
                                .height = input.view_size.y,
                            },
                        .memory_type = frame_graph::MemoryType::GPU,
                        .format = frame_graph::TextureFormat::D32_FLOAT_S8_UINT,
                        .usage = frame_graph::TextureUsageFlags::DEPTH_ATTACHMENT,
                        .tiling = frame_graph::TextureTiling::Optimal,
                    }),
                frame_graph::TextureResourceUsage::DEPTH_STENCIL_ATTACHMENT);

            render_pass.color_attachments.push_back(frame_graph::ColorAttachment {
                .ops = frame_graph::AttachmentOps::INIT,
                .texture = data.visibility_buffer,
                .clear_value = math::Vec4 { 0, 0, 0, 0 },
            });
            render_pass.depth_stencil_attachment = frame_graph::DepthStencilAttachment {
                .ops = frame_graph::AttachmentOps::INIT,
                .texture = data.depth_buffer,
                .clear_value =
                    rhi::ClearDepthStencil {
                        .depth = 0.0f,
                        .stencil = 0,
                    },
            };

            return data;
        },
        [=](rhi::IRHIContext* rhi,
            const frame_graph::Registry& registry,
            rhi::CommandEncoder& encoder,
            const Data& data,
            const rhi::RenderPass& render_pass) {
            //
            const rhi::BufferHandle ubo_buffer //
                = registry.get_buffer(data.ubo_buffer->buffer());
            const rhi::BufferHandle command_count //
                = registry.get_buffer(data.command_count);
            const rhi::BufferHandle command_buffer //
                = registry.get_buffer(data.command_buffer);
            const rhi::BufferHandle task_shader_dispatch_args //
                = registry.get_buffer(data.task_shader_dispatch_args);

            const ubo::TaskShaderUbo ubo {
                .view_to_clip = input.view_to_clip,
                .world_to_view = input.world_to_view,
                .frustum_planes = input.frustum_planes,
                .camera_position = input.camera_position, 
                .in_ = {
                    .mesh_descriptors_srv = input.mesh_descriptors.get_srv(),
                    .mesh_instances_srv = input.mesh_instances.get_srv(),
                    .mesh_instance_transforms_srv = input.mesh_instance_transforms.get_srv(),

                    .command_count_srv = command_count.get_srv(),
                    .command_buffer_srv = command_buffer.get_srv(),
                },
            };

            auto ubo_ref = data.ubo_buffer->allocate<ubo::TaskShaderUbo>();
            rhi->update_buffer(
                ubo_buffer,
                {
                    rhi::BufferUpdateRegion {
                        .src = core::as_byte_span(ubo),
                        .dst_offset = ubo_ref.offset,
                    },
                });

            encoder.push_constants(ubo_buffer, ubo_ref.offset);

            encoder.set_viewport(rhi::Viewport {
                .rect =
                    rhi::Rect {
                        .offset = { 0, 0 },
                        .extent = input.view_size,
                    },
            });

            encoder.set_scissor(rhi::Scissor {
                .offset = { 0, 0 },
                .extent = input.view_size,
            });

            encoder.set_culling_mode(rhi::CullingMode::Back);
            encoder.begin_render_pass(
                rhi::Rect {
                    .offset = { 0, 0 },
                    .extent = input.view_size,
                },
                render_pass);

            encoder.bind_graphics_pipeline(helpers::get_pipeline(
                pipelines::mesh_shaders::passes::MESH_SHADER_NAME,
                input.graphics_pipelines));

            encoder.draw_mesh_tasks_indirect(
                task_shader_dispatch_args,
                0,
                1,
                sizeof(rhi::DrawMeshTasksIndirectCommand));

            encoder.end_render_pass();
        });

    return MeshShaderOutput {
        .visibility_buffer = data.visibility_buffer,
        .depth_buffer = data.depth_buffer,
    };
}

} // namespace tundra::renderer::mesh_shaders
