#include "bindings.hlsli"
#include "defines.hlsli"
#include "templates.hlsli"
#include "visibility_buffer/inc/commands.hlsli"

///
struct IndexBufferGeneratorInitUbo {
    struct {
        uint num_loops;
        uint num_visible_meshlets_srv;
    } in_;

    struct {
        uint meshlet_offsets_uav;
        uint index_generator_dispatch_indirect_commands_uav;
    } out_;
};

///
[numthreads(1, 1, 1)] void main() {
    const IndexBufferGeneratorInitUbo ubo //
        = tundra::load_ubo<IndexBufferGeneratorInitUbo>();

    const uint num_visible_meshlets //
        = tundra::buffer_load<false, uint>(ubo.in_.num_visible_meshlets_srv, 0, 0);

    const uint NUM_MESHLETS = (8192 * 4);

    const uint num_iterations = num_visible_meshlets / NUM_MESHLETS;
    const uint reminder = num_visible_meshlets % NUM_MESHLETS;

    uint start = 0;
    uint count = 0;
    for (uint i = 0; i < num_iterations; ++i) {
        const uint end = start + NUM_MESHLETS;
        tundra::buffer_store<false>(
            ubo.out_.index_generator_dispatch_indirect_commands_uav,
            0,
            count,
            DispatchIndirectCommand::create(NUM_MESHLETS, 1, 1));

        tundra::buffer_store<false>(
            ubo.out_.meshlet_offsets_uav, //
            0,
            count,
            start);

        start = end;
        count += 1;
    }

    if (reminder > 0) {
        const uint end = start + reminder;
        tundra::buffer_store<false>(
            ubo.out_.index_generator_dispatch_indirect_commands_uav,
            0,
            count,
            DispatchIndirectCommand::create(reminder, 1, 1));

        tundra::buffer_store<false>(
            ubo.out_.meshlet_offsets_uav, //
            0,
            count,
            start);
        start = end;
        count += 1;
    }
}
