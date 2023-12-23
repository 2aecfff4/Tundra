#include "bindings.hlsli"
#include "defines.hlsli"
#include "templates.hlsli"

///
struct MeshletCullingInitUbo {
    uint visible_meshlets_count_uav;
};

///
[numthreads(1, 1, 1)] void main() {
    const MeshletCullingInitUbo ubo = tundra::load_ubo<MeshletCullingInitUbo>();

    tundra::buffer_store<false>(ubo.visible_meshlets_count_uav, 0, 0, 0);
}
