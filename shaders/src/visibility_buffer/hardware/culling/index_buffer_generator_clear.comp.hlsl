#include "bindings.hlsli"
#include "defines.hlsli"
#include "templates.hlsli"

///
struct IndexBufferGeneratorClearUbo {
    uint visible_indices_count_uav;
};

///
[numthreads(1, 1, 1)] void main() {
    const IndexBufferGeneratorClearUbo ubo =
        tundra::load_ubo<IndexBufferGeneratorClearUbo>();

    tundra::buffer_store<false>(ubo.visible_indices_count_uav, 0, 0, 0);
}
