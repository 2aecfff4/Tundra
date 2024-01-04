#include "renderer/material_pass.h"
#include "math/vector4.h"
#include "pipelines.h"
#include "renderer/config.h"
#include "renderer/frame_graph/resources/enums.h"
#include "renderer/helpers.h"
#include "rhi/commands/command_encoder.h"
#include "rhi/rhi_context.h"

namespace tundra::renderer::passes {

namespace ubo {

///
struct MaterialUbo {
    math::Mat4 world_to_clip = math::Mat4 {};
    // #TODO:
    math::Vec4 camera_position = math::Vec4 {};
    math::Vec4 light_position = math::Vec4 {};
    math::Vec4 diffuse_color = math::Vec4 {};
    math::Vec4 light_color = math::Vec4 {};
    alignas(8) math::UVec2 view_size = math::UVec2 {};

    struct {
        u32 mesh_descriptors_srv = config::INVALID_SHADER_HANDLE;
        u32 mesh_instance_transforms_srv = config::INVALID_SHADER_HANDLE;
        u32 visible_meshlets_srv = config::INVALID_SHADER_HANDLE;
        u32 visibility_buffer_srv = config::INVALID_SHADER_HANDLE;
    } in_;

    struct {
        u32 color_texture_uav = config::INVALID_SHADER_HANDLE;
    } out_;
};

} // namespace ubo

///
[[nodiscard]] MaterialOutput material(
    frame_graph::FrameGraph& fg, const MaterialInput& input) noexcept
{
    struct Data {
        core::SharedPtr<UboBuffer> ubo_buffer;

        frame_graph::BufferHandle visible_meshlets;
        frame_graph::TextureHandle vis_texture;

        frame_graph::TextureHandle color_texture;
    };

    const Data data = fg.add_pass(
        frame_graph::QueueType::Graphics,
        "material-pass",
        [&](frame_graph::Builder& builder) {
            Data data {};

            data.ubo_buffer = input.ubo_buffer;

            data.visible_meshlets = builder.read(
                input.visible_meshlets,
                frame_graph::BufferResourceUsage::COMPUTE_STORAGE_BUFFER);

            data.vis_texture = builder.read(
                input.vis_depth,
                frame_graph::TextureResourceUsage::COMPUTE_STORAGE_IMAGE);

            data.color_texture = builder.create_texture(
                "material-pass.color_texture",
                frame_graph::TextureCreateInfo {
                    .kind =
                        frame_graph::TextureKind::Texture2D {
                            .width = input.view_size.x,
                            .height = input.view_size.y,
                        },
                    .memory_type = frame_graph::MemoryType::GPU,
                    .format = frame_graph::TextureFormat::R16_G16_B16_A16_FLOAT,
                    .usage = frame_graph::TextureUsageFlags::UAV |
                             frame_graph::TextureUsageFlags::PRESENT,
                    .tiling = frame_graph::TextureTiling::Optimal,
                });

            data.color_texture = builder.write(
                data.color_texture,
                frame_graph::TextureResourceUsage::COMPUTE_STORAGE_IMAGE);

            return data;
        },
        [=](rhi::IRHIContext* rhi,
            const frame_graph::Registry& registry,
            rhi::CommandEncoder& encoder,
            const Data& data) {
            const rhi::BufferHandle ubo_buffer = registry.get_buffer(
                data.ubo_buffer->buffer());
            const rhi::BufferHandle visible_meshlets = registry.get_buffer(
                data.visible_meshlets);
            const rhi::TextureHandle vis_texture = registry.get_texture(data.vis_texture);

            const rhi::TextureHandle color_texture = registry.get_texture(
                data.color_texture);

            const ubo::MaterialUbo ubo {
                .world_to_clip = input.world_to_clip,
                .camera_position = math::Vec4(input.camera_position, 1),
                .light_position = math::Vec4(input.light_position, 1),
                .diffuse_color = math::Vec4(input.diffuse_color, 1),
                .light_color = math::Vec4(input.light_color, 1),
                .view_size = input.view_size,
                .in_ = {
                    .mesh_descriptors_srv = input.mesh_descriptors.get_srv(),
                    .mesh_instance_transforms_srv = input.mesh_instance_transforms.get_srv(),
                    .visible_meshlets_srv = visible_meshlets.get_srv(),
                    .visibility_buffer_srv = vis_texture.get_uav(),
                },
                .out_ = {
                    .color_texture_uav = color_texture.get_uav(),
                },
            };

            const auto ubo_ref = data.ubo_buffer->allocate<ubo::MaterialUbo>();

            rhi->update_buffer(
                ubo_buffer,
                {
                    rhi::BufferUpdateRegion {
                        .src = core::as_byte_span(ubo),
                        .dst_offset = ubo_ref.offset,
                    },
                });

            encoder.push_constants(ubo_buffer, ubo_ref.offset);
            encoder.global_barrier(rhi::GlobalBarrier {
                .previous_access = rhi::GlobalAccessFlags::ALL,
                .next_access = rhi::GlobalAccessFlags::ALL,
            });
            encoder.dispatch(
                helpers::get_pipeline(
                    pipelines::passes::MATERIAL_PASS_NAME, input.compute_pipelines),
                rhi::CommandEncoder::get_group_count(input.view_size.x, 8),
                rhi::CommandEncoder::get_group_count(input.view_size.y, 8),
                1);
            encoder.global_barrier(rhi::GlobalBarrier {
                .previous_access = rhi::GlobalAccessFlags::ALL,
                .next_access = rhi::GlobalAccessFlags::ALL,
            });
        });

    return MaterialOutput {
        .color_texture = data.color_texture,
    };
}

} // namespace tundra::renderer::passes
