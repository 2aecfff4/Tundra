#include "renderer/mesh_shaders/task_dispatch_command_generator.h"
#include "pipelines.h"
#include "renderer/helpers.h"
#include "rhi/commands/dispatch_indirect.h"
#include "rhi/commands/draw_mesh_tasks_indirect.h"
#include "rhi/rhi_context.h"
#include <array>

namespace tundra::renderer::mesh_shaders {

namespace ubo {

///
struct TaskDispatchCommandGeneratorUbo {
    struct {
        u32 command_count_srv = config::INVALID_SHADER_HANDLE;
    } in_;

    struct {
        u32 task_shader_dispatch_args_uav = config::INVALID_SHADER_HANDLE;
    } out_;
};

} // namespace ubo

///
TaskDispatchCommandGeneratorOutput task_dispatch_command_generator(
    frame_graph::FrameGraph& fg, const TaskDispatchCommandGeneratorInput& input) noexcept
{
    struct Data {
        core::SharedPtr<UboBuffer> ubo_buffer;

        frame_graph::BufferHandle command_count;
        frame_graph::BufferHandle task_shader_dispatch_args;
    };

    const Data data = fg.add_pass(
        frame_graph::QueueType::Graphics,
        "task_dispatch_command_generator",
        [&](frame_graph::Builder& builder) {
            Data data {};

            data.ubo_buffer = input.ubo_buffer;

            data.command_count = input.command_count;
            builder.read(
                data.command_count,
                frame_graph::BufferResourceUsage::COMPUTE_STORAGE_BUFFER);

            data.task_shader_dispatch_args = builder.create_buffer(
                "task_dispatch_command_generator.task_shader_dispatch_args",
                frame_graph::BufferCreateInfo {
                    .usage = frame_graph::BufferUsageFlags::INDIRECT_BUFFER |
                             frame_graph::BufferUsageFlags::STORAGE_BUFFER,
                    .memory_type = frame_graph::MemoryType::GPU,
                    .size = sizeof(rhi::DrawMeshTasksIndirectCommand),
                });

            builder.write(
                data.task_shader_dispatch_args,
                frame_graph::BufferResourceUsage::COMPUTE_STORAGE_BUFFER);

            return data;
        },
        [=](rhi::IRHIContext* rhi,
            const frame_graph::Registry& registry,
            rhi::CommandEncoder& encoder,
            const Data& data) {
            //
            const rhi::BufferHandle ubo_buffer //
                = registry.get_buffer(data.ubo_buffer->buffer());
            const rhi::BufferHandle command_count //
                = registry.get_buffer(data.command_count);
            const rhi::BufferHandle task_shader_dispatch_args //
                = registry.get_buffer(data.task_shader_dispatch_args);

            const ubo::TaskDispatchCommandGeneratorUbo ubo {
                .in_ = {
                    .command_count_srv = command_count.get_srv(),
                },
                .out_ = {
                    .task_shader_dispatch_args_uav = task_shader_dispatch_args.get_uav(),
                },
            };

            const auto ubo_ref = data //
                                     .ubo_buffer
                                     ->allocate<ubo::TaskDispatchCommandGeneratorUbo>();
            rhi->update_buffer(
                ubo_buffer,
                {
                    rhi::BufferUpdateRegion {
                        .src = core::as_byte_span(ubo),
                        .dst_offset = ubo_ref.offset,
                    },
                });
            encoder.push_constants(ubo_buffer, static_cast<u32>(ubo_ref.offset));
            encoder.dispatch(
                helpers::get_pipeline(
                    pipelines::mesh_shaders::passes::TASK_DISPATCH_COMMAND_GENERATOR_NAME,
                    input.compute_pipelines),
                1,
                1,
                1);
        });

    return TaskDispatchCommandGeneratorOutput {
        .task_shader_dispatch_args = data.task_shader_dispatch_args,
    };
}

} // namespace tundra::renderer::mesh_shaders
