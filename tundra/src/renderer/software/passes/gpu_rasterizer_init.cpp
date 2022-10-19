#include "renderer/software/passes/gpu_rasterizer_init.h"
#include "pipelines.h"
#include "renderer/config.h"
#include "renderer/helpers.h"
#include "rhi/commands/dispatch_indirect.h"
#include "rhi/rhi_context.h"

namespace tundra::renderer::passes {

namespace ubo {

///
struct GpuRasterizeInitUBO {
    math::UVec2 view_size = math::UVec2 {};
    math::UVec2 dispatch_grid_dim = math::UVec2 {};
    u32 visible_meshlets_count_srv = config::INVALID_SHADER_HANDLE;
    u32 dispatch_args_uav = config::INVALID_SHADER_HANDLE;
    u32 out_texture_uav = config::INVALID_SHADER_HANDLE;
};

} // namespace ubo

///
GpuRasterizerInitOutput gpu_rasterizer_init_pass(
    frame_graph::FrameGraph& fg, const GpuRasterizerInitInput& input) noexcept
{
    const u64 ubo_buffer_offset = input.ubo_buffer_offset +
                                  sizeof(ubo::GpuRasterizeInitUBO);

    struct Data {
        frame_graph::BufferHandle ubo_buffer;
        u64 ubo_buffer_offset = 0;

        frame_graph::BufferHandle visible_meshlets_count;

        frame_graph::TextureHandle vis_texture;
        frame_graph::BufferHandle gpu_rasterizer_dispatch_args;
    };

    const Data data = fg.add_pass(
        frame_graph::QueueType::Graphics,
        "gpu_rasterize_init_pass",
        [&](frame_graph::Builder& builder) {
            Data data {};

            data.ubo_buffer = input.ubo_buffer;
            data.ubo_buffer_offset = input.ubo_buffer_offset;

            data.visible_meshlets_count = builder.read(
                input.visible_meshlets_count, frame_graph::ResourceUsage::SHADER_COMPUTE);

            data.vis_texture = builder.create_texture(
                "gpu_rasterizer.vis_texture",
                frame_graph::TextureCreateInfo {
                    .kind =
                        frame_graph::TextureKind::Texture2D {
                            .width = input.view_size.x,
                            .height = input.view_size.y,
                        },
                    .memory_type = frame_graph::MemoryType::GPU,
                    .format = frame_graph::TextureFormat::R64_UINT,
                    .usage = frame_graph::TextureUsageFlags::UAV,
                    .tiling = frame_graph::TextureTiling::Optimal,
                });
            data.vis_texture = builder.write(
                data.vis_texture, frame_graph::ResourceUsage::SHADER_COMPUTE);

            data.gpu_rasterizer_dispatch_args = builder.create_buffer(
                "gpu_rasterize_init_pass.gpu_rasterizer_dispatch_args",
                frame_graph::BufferCreateInfo {
                    .usage = frame_graph::BufferUsageFlags::UAV |
                             frame_graph::BufferUsageFlags::INDIRECT_BUFFER,
                    .memory_type = frame_graph::MemoryType::GPU,
                    .size = sizeof(rhi::DispatchIndirectCommand),
                });
            builder.write(
                data.gpu_rasterizer_dispatch_args,
                frame_graph::ResourceUsage::SHADER_COMPUTE);

            return data;
        },
        [=](rhi::IRHIContext* rhi,
            const frame_graph::Registry& registry,
            rhi::CommandEncoder& encoder,
            const Data& data) {
            //
            const rhi::BufferHandle ubo_buffer = registry.get_buffer(data.ubo_buffer);
            const rhi::BufferHandle visible_meshlets_count = registry.get_buffer(
                data.visible_meshlets_count);
            const rhi::BufferHandle gpu_rasterizer_dispatch_args = registry.get_buffer(
                data.gpu_rasterizer_dispatch_args);
            const rhi::TextureHandle vis_texture = registry.get_texture(data.vis_texture);

            const math::UVec2 dispatch_grid_dim {
                rhi::CommandEncoder::get_group_count(input.view_size.x, 16),
                rhi::CommandEncoder::get_group_count(input.view_size.y, 16),
            };

            const ubo::GpuRasterizeInitUBO ubo {
                .view_size = input.view_size,
                .dispatch_grid_dim = dispatch_grid_dim,
                .visible_meshlets_count_srv = visible_meshlets_count.get_srv(),
                .dispatch_args_uav = gpu_rasterizer_dispatch_args.get_uav(),
                .out_texture_uav = vis_texture.get_uav(),
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
            encoder.dispatch(
                helpers::get_pipeline(
                    pipelines::software::passes::GPU_RASTERIZE_INIT_NAME,
                    input.compute_pipelines),
                dispatch_grid_dim.x,
                dispatch_grid_dim.y,
                1);
        });

    return GpuRasterizerInitOutput {
        .ubo_buffer = data.ubo_buffer,
        .ubo_buffer_offset = ubo_buffer_offset,
        .vis_texture = data.vis_texture,
        .gpu_rasterizer_dispatch_args = data.gpu_rasterizer_dispatch_args,
    };
}

} // namespace tundra::renderer::passes
