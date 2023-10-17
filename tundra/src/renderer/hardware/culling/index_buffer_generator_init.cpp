#include "renderer/hardware/culling/index_buffer_generator_init.h"
#include "pipelines.h"
#include "renderer/config.h"
#include "renderer/frame_graph/resources/buffer.h"
#include "renderer/frame_graph/resources/enums.h"
#include "renderer/frame_graph/resources/handle.h"
#include "renderer/helpers.h"
#include "rhi/commands/dispatch_indirect.h"
#include "rhi/commands/draw_indirect.h"
#include "rhi/resources/handle.h"
#include "rhi/rhi_context.h"

namespace tundra::renderer::hardware::culling {

namespace ubo {

///
struct IndexBufferGeneratorInitUbo {
    struct {
        u32 num_loops = 0;
        u32 num_visible_meshlets_srv = config::INVALID_SHADER_HANDLE;
    } in_;

    struct {
        u32 meshlet_offsets_uav = config::INVALID_SHADER_HANDLE;
        u32 index_generator_dispatch_indirect_commands_uav = config::INVALID_SHADER_HANDLE;
    } out_;
};

} // namespace ubo

///
IndexBufferGeneratorInitOutput index_buffer_generator_init(
    frame_graph::FrameGraph& fg, //
    const IndexBufferGeneratorInitInput& input) noexcept
{
    struct Data {
        core::SharedPtr<UboBuffer> ubo_buffer;

        frame_graph::BufferHandle num_visible_meshlets;

        frame_graph::BufferHandle meshlet_offsets;
        frame_graph::BufferHandle index_buffer;
        frame_graph::BufferHandle visible_indices_count;

        frame_graph::BufferHandle draw_meshlets_draw_args;
        frame_graph::BufferHandle draw_count;
        frame_graph::BufferHandle index_generator_dispatch_indirect_commands;
    };

    const Data data = fg.add_pass(
        frame_graph::QueueType::Graphics,
        "index_buffer_generator_init",
        [&](frame_graph::Builder& builder) {
            Data data {};

            data.ubo_buffer = input.ubo_buffer;

            data.num_visible_meshlets = builder.read(
                input.num_visible_meshlets, //
                frame_graph::ResourceUsage::SHADER_COMPUTE);

            data.meshlet_offsets = builder.write(
                builder.create_buffer(
                    "meshlet_offsets",
                    frame_graph::BufferCreateInfo {
                        .usage = frame_graph::BufferUsageFlags::UAV |
                                 frame_graph::BufferUsageFlags::SRV,
                        .memory_type = frame_graph::MemoryType::GPU,
                        .size = sizeof(u32) * input.num_loops,
                    }),
                frame_graph::ResourceUsage::SHADER_COMPUTE);

            data.index_buffer = builder.write(
                builder.create_buffer(
                    "index_buffer",
                    frame_graph::BufferCreateInfo {
                        .usage = frame_graph::BufferUsageFlags::UAV |
                                 frame_graph::BufferUsageFlags::SRV |
                                 frame_graph::BufferUsageFlags::INDEX_BUFFER,
                        .memory_type = frame_graph::MemoryType::GPU,
                        .size = config::NUM_INDEX_BUFFERS_IN_FIGHT *
                                config::INDEX_BUFFER_BATCH_SIZE * sizeof(u32),
                    }),
                frame_graph::ResourceUsage::SHADER_COMPUTE);

            data.visible_indices_count = builder.write(
                builder.create_buffer(
                    "visible_indices_count",
                    frame_graph::BufferCreateInfo {
                        .usage = frame_graph::BufferUsageFlags::SRV |
                                 frame_graph::BufferUsageFlags::UAV,
                        .memory_type = frame_graph::MemoryType::GPU,
                        .size = sizeof(u32),
                    }),
                frame_graph::ResourceUsage::SHADER_COMPUTE);

            data.draw_meshlets_draw_args = builder.write(
                builder.create_buffer(
                    "draw_meshlets_draw_args",
                    frame_graph::BufferCreateInfo {
                        .usage = frame_graph::BufferUsageFlags::SRV |
                                 frame_graph::BufferUsageFlags::UAV |
                                 frame_graph::BufferUsageFlags::INDIRECT_BUFFER,
                        .memory_type = frame_graph::MemoryType::GPU,
                        .size = sizeof(rhi::DrawIndexedIndirectCommand) * 8,
                    }),
                frame_graph::ResourceUsage::SHADER_COMPUTE);

            data.draw_count = builder.write(
                builder.create_buffer(
                    "draw_count",
                    frame_graph::BufferCreateInfo {
                        .usage = frame_graph::BufferUsageFlags::SRV |
                                 frame_graph::BufferUsageFlags::UAV |
                                 frame_graph::BufferUsageFlags::INDIRECT_BUFFER,
                        .memory_type = frame_graph::MemoryType::GPU,
                        .size = sizeof(u32),
                    }),
                frame_graph::ResourceUsage::SHADER_COMPUTE);

            data.index_generator_dispatch_indirect_commands = builder.write(
                builder.create_buffer(
                    "index_generator_dispatch_indirect_commands",
                    frame_graph::BufferCreateInfo {
                        .usage = frame_graph::BufferUsageFlags::SRV |
                                 frame_graph::BufferUsageFlags::UAV |
                                 frame_graph::BufferUsageFlags::INDIRECT_BUFFER,
                        .memory_type = frame_graph::MemoryType::GPU,
                        .size = sizeof(rhi::DispatchIndirectCommand) * input.num_loops,
                    }),
                frame_graph::ResourceUsage::SHADER_COMPUTE);

            return data;
        },
        [=](rhi::IRHIContext* rhi,
            const frame_graph::Registry& registry,
            rhi::CommandEncoder& encoder,
            const Data& data) {
            //
            const rhi::BufferHandle ubo_buffer //
                = registry.get_buffer(data.ubo_buffer->buffer());

            const rhi::BufferHandle num_visible_meshlets = registry.get_buffer(
                data.num_visible_meshlets);
            const rhi::BufferHandle meshlet_offsets = registry.get_buffer(
                data.meshlet_offsets);
            const rhi::BufferHandle index_generator_dispatch_indirect_commands //
                = registry.get_buffer(data.index_generator_dispatch_indirect_commands);

            const ubo::IndexBufferGeneratorInitUbo ubo {
                .in_ {
                    .num_loops = input.num_loops,
                    .num_visible_meshlets_srv = num_visible_meshlets.get_srv(),
                },
                .out_ {
                    .meshlet_offsets_uav = meshlet_offsets.get_uav(),
                    .index_generator_dispatch_indirect_commands_uav =
                        index_generator_dispatch_indirect_commands.get_uav(),
                }
            };

            const auto ubo_ref = data.ubo_buffer
                                     ->allocate<ubo::IndexBufferGeneratorInitUbo>();

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
                    pipelines::hardware::culling::INDEX_BUFFER_GENERATOR_INIT_NAME,
                    input.compute_pipelines),
                1,
                1,
                1);
        });

    return IndexBufferGeneratorInitOutput {
        .meshlet_offsets = data.meshlet_offsets,
        .index_buffer = data.index_buffer,
        .visible_indices_count = data.visible_indices_count,

        .draw_meshlets_draw_args = data.draw_meshlets_draw_args,
        .draw_count = data.draw_count,
        .index_generator_dispatch_indirect_commands =
            data.index_generator_dispatch_indirect_commands,
    };
}

} // namespace tundra::renderer::hardware::culling
