#include "bindings.hlsli"
#include "defines.hlsli"
#include "shared.hlsli"
#include "templates.hlsli"

///
struct InstanceCullingInitUbo {
    uint command_count_uav;
};

///
[numthreads(1, 1, 1)]
void main()
{
    const InstanceCullingInitUbo ubo = tundra::load_ubo<InstanceCullingInitUbo>();

    tundra::buffer_store<false>(ubo.command_count_uav, 0, 0, 0);
}
