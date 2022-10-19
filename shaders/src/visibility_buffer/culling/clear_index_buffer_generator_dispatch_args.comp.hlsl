#include "bindings.hlsli"
#include "defines.hlsli"
#include "templates.hlsli"

///
struct ClearIndexBufferGeneratorDispatchArgs {
    struct {
        uint meshlet_offset_uav;
    } out_;
};

///
[numthreads(1, 1, 1)] void main() {
    const ClearIndexBufferGeneratorDispatchArgs ubo =
        tundra::load_ubo<ClearIndexBufferGeneratorDispatchArgs>();
    tundra::buffer_store<false, uint>(ubo.out_.meshlet_offset_uav, 0, 0, 0);
}
