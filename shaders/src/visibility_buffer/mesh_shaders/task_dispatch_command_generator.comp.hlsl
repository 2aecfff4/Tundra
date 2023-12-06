#include "bindings.hlsli"
#include "defines.hlsli"
#include "shared.hlsli"
#include "templates.hlsli"

///
struct TaskDispatchCommandGeneratorUbo {
    struct {
        uint command_count_srv;
    }
    in_;

    struct {
        uint task_shader_dispatch_args_uav;
    }
    out_;
};

///
[numthreads(1, 1, 1)]
void main(uint thread_id: SV_DispatchThreadID)
{
    const TaskDispatchCommandGeneratorUbo ubo //
        = tundra::load_ubo<TaskDispatchCommandGeneratorUbo>();

    const uint command_count = tundra::buffer_load<false, uint>(
        ubo.in_.command_count_srv, 0, 0);

    tundra::buffer_store<false>(
        ubo.out_.task_shader_dispatch_args_uav,
        0,
        0,
        DrawMeshTasksIndirectCommand::create(
            min(((command_count + 63) / 64), 65535), //
            64,
            1));
}
