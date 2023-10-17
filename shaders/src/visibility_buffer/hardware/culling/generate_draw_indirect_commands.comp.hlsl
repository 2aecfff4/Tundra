#include "bindings.hlsli"
#include "defines.hlsli"
#include "templates.hlsli"
#include "visibility_buffer/inc/commands.hlsli"

///
struct GenerateIndirectCommandsUbo {
    struct {
        uint visible_indices_count_srv;
    } in_;

    struct {
        uint draw_meshlets_draw_args_uav;
        uint draw_count_uav;
    } out_;
};

#define NUM_MESHLETS_PER_INDEX_BUFFER 8192
#define INDEX_BUFFER_BATCH_SIZE (NUM_MESHLETS_PER_INDEX_BUFFER * 128u * 3u)

///
[numthreads(1, 1, 1)] void main(uint thread_id
                                : SV_DispatchThreadID) {
    const GenerateIndirectCommandsUbo ubo //
        = tundra::load_ubo<GenerateIndirectCommandsUbo>();

    const uint visible_indices_count //
        = tundra::buffer_load<false, uint>(ubo.in_.visible_indices_count_srv, 0, 0);

    const uint num_iterations = visible_indices_count / INDEX_BUFFER_BATCH_SIZE;
    const uint reminder = visible_indices_count % INDEX_BUFFER_BATCH_SIZE;

    // def split_work(X, N):
    //     num_iterations = X // N
    //     reminder = X % N
    //     start = 0
    //
    //     result = []
    //     for _ in range(num_iterations):
    //         end = start + N
    //         result.append((start, N))
    //         start = end
    //
    //     if reminder > 0:
    //         result.append((start, reminder))
    //
    //     return result
    uint start = 0;
    uint count = 0;
    for (uint i = 0; i < num_iterations; ++i) {
        const uint end = start + INDEX_BUFFER_BATCH_SIZE;
        tundra::buffer_store<false>(
            ubo.out_.draw_meshlets_draw_args_uav,
            0,
            count,
            DrawIndexedIndirectCommand::create(
                INDEX_BUFFER_BATCH_SIZE, //
                1,
                start,
                0,
                0));
        start = end;
        count += 1;
    }

    if (reminder > 0) {
        tundra::buffer_store<false>(
            ubo.out_.draw_meshlets_draw_args_uav,
            0,
            count,
            DrawIndexedIndirectCommand::create(
                reminder, //
                1,
                start,
                0,
                0));
        count += 1;
    }

    tundra::buffer_store<false>(ubo.out_.draw_count_uav, 0, 0, count);
}
