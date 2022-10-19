#include "renderer/software/passes/gpu_rasterizer.h"
#include "pipelines.h"
#include "renderer/config.h"
#include "renderer/helpers.h"
#include "rhi/rhi_context.h"

namespace tundra::renderer::passes {

namespace ubo {

///
struct GPURasterizeUBO {
    math::Mat4 world_to_clip = math::Mat4 {};
    math::UVec2 view_size = math::UVec2 {};

    struct {
        u32 mesh_instance_transforms_srv = config::INVALID_SHADER_HANDLE;
        u32 mesh_descriptors_srv = config::INVALID_SHADER_HANDLE;
        u32 visible_meshlets_srv = config::INVALID_SHADER_HANDLE;
    } in_;

    struct {
        u32 output_texture_uav = config::INVALID_SHADER_HANDLE;
    } out_;
};

} // namespace ubo

///
[[nodiscard]] GpuRasterizerOutput gpu_rasterizer_pass(
    frame_graph::FrameGraph& fg, const GpuRasterizerInput& input) noexcept
{
    const u64 ubo_buffer_offset = input.ubo_buffer_offset + sizeof(ubo::GPURasterizeUBO);

    struct Data {
        frame_graph::BufferHandle ubo_buffer;
        u64 ubo_buffer_offset = 0;

        frame_graph::BufferHandle visible_meshlets;
        frame_graph::BufferHandle dispatch_indirect_args;
        frame_graph::TextureHandle vis_texture;
    };

    const Data data = fg.add_pass(
        frame_graph::QueueType::Graphics,
        "gpu_rasterizer_pass",
        [&](frame_graph::Builder& builder) {
            Data data {};

            data.ubo_buffer = input.ubo_buffer;
            data.ubo_buffer_offset = input.ubo_buffer_offset;

            data.visible_meshlets = builder.read(
                input.visible_meshlets, frame_graph::ResourceUsage::SHADER_COMPUTE);

            data.dispatch_indirect_args = builder.read(
                input.dispatch_indirect_args,
                frame_graph::ResourceUsage::INDIRECT_BUFFER);

            data.vis_texture = input.vis_texture;
            data.vis_texture = builder.read(
                data.vis_texture, frame_graph::ResourceUsage::SHADER_COMPUTE);
            data.vis_texture = builder.write(
                data.vis_texture, frame_graph::ResourceUsage::SHADER_COMPUTE);

            return data;
        },
        [=](rhi::IRHIContext* rhi,
            const frame_graph::Registry& registry,
            rhi::CommandEncoder& encoder,
            const Data& data) {
            const rhi::BufferHandle ubo_buffer = registry.get_buffer(data.ubo_buffer);
            const rhi::BufferHandle visible_meshlets = registry.get_buffer(
                data.visible_meshlets);
            const rhi::BufferHandle dispatch_indirect_args = registry.get_buffer(
                data.dispatch_indirect_args);
            const rhi::TextureHandle vis_texture = registry.get_texture(data.vis_texture);

            const ubo::GPURasterizeUBO ubo {
                .world_to_clip = input.world_to_clip,
                .view_size = input.view_size,
                .in_ = {
                    .mesh_instance_transforms_srv = input.mesh_instance_transforms.get_srv(),
                    .mesh_descriptors_srv = input.mesh_descriptors.get_srv(),
                    .visible_meshlets_srv = visible_meshlets.get_srv(),
                },
                .out_ = {
                    .output_texture_uav = vis_texture.get_uav(),
                },
            };

            rhi->update_buffer(
                ubo_buffer,
                {
                    rhi::BufferUpdateRegion {
                        .src = core::as_byte_span(ubo),
                        .dst_offset = data.ubo_buffer_offset,
                    },
                });

            encoder.push_constants(ubo_buffer, data.ubo_buffer_offset);
            encoder.dispatch_indirect(
                helpers::get_pipeline(
                    pipelines::software::passes::GPU_RASTERIZE_PASS_NAME,
                    input.compute_pipelines),
                dispatch_indirect_args,
                0);
        });

    return GpuRasterizerOutput {
        .ubo_buffer = data.ubo_buffer,
        .ubo_buffer_offset = ubo_buffer_offset,
        .vis_texture = data.vis_texture,
    };
}

} // namespace tundra::renderer::passes
