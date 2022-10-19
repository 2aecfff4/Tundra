#include "bindings.hlsli"
#include "defines.hlsli"
#include "templates.hlsli"
#include "visibility_buffer/inc/commands.hlsli"

///
struct GpuRasterizeGenerateDispatchArgsUBO {
    uint num_visible_meshlets_srv;
    uint dispatch_args_uav;
};

///
[numthreads(1, 1, 1)] void main() {
    const GpuRasterizeGenerateDispatchArgsUBO ubo =
        tundra::load_ubo<GpuRasterizeGenerateDispatchArgsUBO>();

    const uint num_visible_meshlets = tundra::buffer_load<false, uint>(
        ubo.num_visible_meshlets_srv, 0, 0);

    tundra::buffer_store<false, DispatchIndirectCommand>(
        ubo.dispatch_args_uav,
        0,
        0,
        DispatchIndirectCommand::create(
            num_visible_meshlets, //
            1,
            1));
}
