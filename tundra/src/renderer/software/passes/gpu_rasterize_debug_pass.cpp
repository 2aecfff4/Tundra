#include "renderer/software/passes/gpu_rasterize_debug_pass.h"
#include "pipelines.h"
#include "renderer/config.h"
#include "renderer/helpers.h"
#include "rhi/commands/command_encoder.h"
#include "rhi/rhi_context.h"

namespace tundra::renderer::passes {

namespace ubo {

///
struct GpuRasterizeDebugUBO {
    math::UVec2 size = math::UVec2 {};
    u32 input_srv = config::INVALID_SHADER_HANDLE;
    u32 output_uav = config::INVALID_SHADER_HANDLE;
};

} // namespace ubo

///
GpuRasterizeDebugOutput gpu_rasterize_debug_pass(
    frame_graph::FrameGraph& fg, const GpuRasterizeDebugInput& input) noexcept
{
    struct Data {
        core::SharedPtr<UboBuffer> ubo_buffer;

        frame_graph::TextureHandle vis_depth;
        frame_graph::TextureHandle debug_texture;
    };

    const Data data = fg.add_pass(
        frame_graph::QueueType::Graphics,
        "gpu_rasterize_debug_pass",
        [&](frame_graph::Builder& builder) {
            Data data {};

            data.ubo_buffer = input.ubo_buffer;

            data.vis_depth = builder.read(
                input.vis_depth, frame_graph::ResourceUsage::SHADER_COMPUTE);

            data.debug_texture = builder.create_texture(
                "gpu_rasterize_debug_pass.debug_texture",
                frame_graph::TextureCreateInfo {
                    .kind =
                        frame_graph::TextureKind::Texture2D {
                            .width = input.view_size.x,
                            .height = input.view_size.y,
                        },
                    .memory_type = frame_graph::MemoryType::GPU,
                    .format = frame_graph::TextureFormat::R8_G8_B8_A8_UNORM,
                    .usage = frame_graph::TextureUsageFlags::UAV |
                             frame_graph::TextureUsageFlags::PRESENT,
                    .tiling = frame_graph::TextureTiling::Optimal,
                });

            data.debug_texture = builder.write(
                data.debug_texture, frame_graph::ResourceUsage::SHADER_COMPUTE);

            return data;
        },
        [=](rhi::IRHIContext* rhi,
            const frame_graph::Registry& registry,
            rhi::CommandEncoder& encoder,
            const Data& data) {
            const rhi::BufferHandle ubo_buffer = registry.get_buffer(
                data.ubo_buffer->buffer());
            const rhi::TextureHandle vis_depth = registry.get_texture(data.vis_depth);
            const rhi::TextureHandle debug_texture = registry.get_texture(
                data.debug_texture);

            const ubo::GpuRasterizeDebugUBO ubo {
                .size = input.view_size,
                .input_srv = vis_depth.get_uav(),
                .output_uav = debug_texture.get_uav(),
            };

            const auto ubo_ref = data.ubo_buffer->allocate<ubo::GpuRasterizeDebugUBO>();

            rhi->update_buffer(
                ubo_buffer,
                {
                    rhi::BufferUpdateRegion {
                        .src = core::as_byte_span(ubo),
                        .dst_offset = ubo_ref.offset,
                    },
                });

            encoder.push_constants(ubo_buffer, ubo_ref.offset);
            encoder.dispatch(
                helpers::get_pipeline(
                    pipelines::software::passes::GPU_RASTERIZE_DEBUG_PASS_NAME,
                    input.compute_pipelines),
                rhi::CommandEncoder::get_group_count(input.view_size.x, 16),
                rhi::CommandEncoder::get_group_count(input.view_size.x, 16),
                1);
        });

    return GpuRasterizeDebugOutput {
        .debug_texture = data.debug_texture,
    };
}

} // namespace tundra::renderer::passes
