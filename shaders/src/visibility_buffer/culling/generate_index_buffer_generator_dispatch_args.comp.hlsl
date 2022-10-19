#include "bindings.hlsli"
#include "defines.hlsli"
#include "templates.hlsli"
#include "visibility_buffer/inc/commands.hlsli"

///
struct GenerateDispatchArgsUBO {
    uint num_index_buffers_in_fight;
    uint num_meshlets_per_index_buffer;

    struct {
        uint num_visible_meshlets_srv;
    } in_;

    struct {
        uint index_buffer_generator_dispatch_args_uav;
        uint index_buffer_generator_meshlet_offsets_uav;
        uint draw_meshlets_draw_counts_uav;
        uint draw_meshlets_draw_args_uav;
    } out_;

    struct {
        uint meshlet_offset_uav;
    } inout_;
};

///
[numthreads(1, 1, 1)] void main() {
    const GenerateDispatchArgsUBO ubo = tundra::load_ubo<GenerateDispatchArgsUBO>();

    const uint num_visible_meshlets = tundra::buffer_load<false, uint>(
        ubo.in_.num_visible_meshlets_srv, 0, 0);
    const uint meshlet_offset = tundra::buffer_load<false, uint>(
        ubo.inout_.meshlet_offset_uav, 0, 0);

    for (uint i = 0; i < ubo.num_index_buffers_in_fight; ++i) {
        tundra::buffer_store<false, DispatchIndirectCommand>(
            ubo.out_.index_buffer_generator_dispatch_args_uav,
            sizeof(DispatchIndirectCommand) * i,
            0,
            DispatchIndirectCommand::create(0, 0, 0));

        tundra::buffer_store<false>(
            ubo.out_.draw_meshlets_draw_args_uav,
            sizeof(DrawIndexedIndirectCommand) * i,
            0,
            DrawIndexedIndirectCommand::create(0, 0, 0, 0, 0));

        tundra::buffer_store<false, uint>(
            ubo.out_.index_buffer_generator_meshlet_offsets_uav, sizeof(uint) * i, 0, 0);
        tundra::buffer_store<false, uint>(
            ubo.out_.draw_meshlets_draw_counts_uav, sizeof(uint) * i, 0, 0);
    }

    if (meshlet_offset >= num_visible_meshlets) {
        return;
    }

    uint offset = meshlet_offset;
    for (uint i = 0; i < ubo.num_index_buffers_in_fight; ++i) {
        const uint from = offset;
        const uint to = offset + ubo.num_meshlets_per_index_buffer;
        offset = to;

        const uint num_meshlets = num_visible_meshlets - from;
        const uint num_meshlets_dispatch = clamp(
            num_meshlets, 0, ubo.num_meshlets_per_index_buffer);

        if ((from + num_meshlets_dispatch) > num_visible_meshlets) {
            return;
        }

        tundra::buffer_store<false, DispatchIndirectCommand>(
            ubo.out_.index_buffer_generator_dispatch_args_uav,
            sizeof(DispatchIndirectCommand) * i,
            0,
            DispatchIndirectCommand::create(
                num_meshlets_dispatch, //
                1,
                1));

        tundra::buffer_store<false, uint>(
            ubo.out_.index_buffer_generator_meshlet_offsets_uav,
            sizeof(uint) * i,
            0,
            from);

        tundra::buffer_store<false, uint>(
            ubo.inout_.meshlet_offset_uav, 0, 0, from + num_meshlets_dispatch);

        tundra::buffer_store<false, uint>(
            ubo.out_.draw_meshlets_draw_counts_uav, sizeof(uint) * i, 0, 1);
    }
}
