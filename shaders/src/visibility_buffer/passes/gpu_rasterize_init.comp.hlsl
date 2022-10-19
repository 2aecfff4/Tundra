#include "bindings.hlsli"
#include "defines.hlsli"
#include "hash.hlsli"
#include "math/quat.hlsli"
#include "templates.hlsli"
#include "utils/thread_group_tiling.hlsli"
#include "visibility_buffer/inc/commands.hlsli"

///
struct GpuRasterizerInitUBO {
    uint2 view_size;
    uint2 dispatch_grid_dim;
    uint visible_meshlets_count_srv;
    uint dispatch_args_uav;
    uint out_texture_uav;
};

///
struct Input {
    uint2 thread_id : SV_DispatchThreadID;
    uint2 group_thread_id : SV_GroupThreadID;
    uint2 group_id : SV_GroupID;
};

///
[numthreads(16, 16, 1)] void main(const Input input) {
    const GpuRasterizerInitUBO ubo = tundra::load_ubo<GpuRasterizerInitUBO>();

    if (all(input.thread_id == uint2(0, 0))) {
        const uint num_visible_meshlets = tundra::buffer_load<false, uint>(
            ubo.visible_meshlets_count_srv, 0, 0);

        tundra::buffer_store<false, DispatchIndirectCommand>(
            ubo.dispatch_args_uav,
            0,
            0,
            DispatchIndirectCommand::create(
                num_visible_meshlets, //
                1,
                1));
    }

    const uint2 idx = tundra::utils::thread_group_tiling(
        ubo.dispatch_grid_dim, //
        uint2(16, 16),
        16,
        input.group_thread_id,
        input.group_id);

    if (any(idx >= ubo.view_size)) {
        return;
    }

    RWTexture2D<uint64_t> tex = g_rw_textures2D_uint64[ubo.out_texture_uav];
    tex[idx] = 0;
}
