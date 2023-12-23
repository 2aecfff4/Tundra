#include "bindings.hlsli"
#include "defines.hlsli"
#include "templates.hlsli"
#include "visibility_buffer/inc/commands.hlsli"

///
struct InstanceCullingInitUbo {
    uint meshlet_culling_dispatch_args_uav;
};

///
[numthreads(128, 1, 1)] void main() {
    const InstanceCullingInitUbo ubo = tundra::load_ubo<InstanceCullingInitUbo>();

    tundra::buffer_store<false>(
        ubo.meshlet_culling_dispatch_args_uav,
        0,
        0,
        DispatchIndirectCommand::create(0, 1, 1));
}
