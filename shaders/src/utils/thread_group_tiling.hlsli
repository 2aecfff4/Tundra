#ifndef TNDR_SHADERS_UTILS_THREAD_GROUP_TILING_H
#define TNDR_SHADERS_UTILS_THREAD_GROUP_TILING_H

namespace tundra {
namespace utils {

/// Divide the 2D-Dispatch_Grid into tiles of dimension [N, DipatchGridDim.y]
/// “CTA” (Cooperative Thread Array) == Thread Group in DirectX terminology
///
/// @param dispatch_grid_dim Arguments of the Dispatch call.
/// @param cta_dim Already known in HLSL, eg:[numthreads(8, 8, 1)] -> uint2(8, 8).
/// @param max_tile_width User parameter (N). Recommended values: 8, 16 or 32.
/// @param group_thread_id SV_GroupThreadID.
/// @param group_id SV_GroupID.
///
/// https://developer.nvidia.com/blog/optimizing-compute-shaders-for-l2-locality-using-thread-group-id-swizzling/
uint2 thread_group_tiling(
    const uint2 dispatch_grid_dim,
    const uint2 cta_dim,
    const uint max_tile_width,
    const uint2 group_thread_id,
    const uint2 group_id)
{
    // A perfect tile is one with dimensions = [max_tile_width, dispatch_grid_dim.y]
    const uint number_of_ctas_in_a_perfect_tile = max_tile_width * dispatch_grid_dim.y;

    // Possible number of perfect tiles
    const uint number_of_perfect_tiles = dispatch_grid_dim.x / max_tile_width;

    // Total number of CTAs present in the perfect tiles
    const uint total_ctas_in_all_perfect_tiles = number_of_perfect_tiles *
                                                 max_tile_width * dispatch_grid_dim.y;
    const uint thread_group_id_flattened = dispatch_grid_dim.x * group_id.y + group_id.x;

    // tile_id_of_current_cta : current CTA to TILE-ID mapping.
    const uint tile_id_of_current_cta = thread_group_id_flattened /
                                        number_of_ctas_in_a_perfect_tile;
    const uint local_cta_id_within_current_tile = thread_group_id_flattened %
                                                  number_of_ctas_in_a_perfect_tile;
    uint local_cta_id_y_within_current_tile;
    uint local_cta_id_x_within_current_tile;

    if (total_ctas_in_all_perfect_tiles <= thread_group_id_flattened) {
        // Path taken only if the last tile has imperfect dimensions and CTAs from the last tile are launched.
        const uint x_dimension_of_last_tile = dispatch_grid_dim.x % max_tile_width;
        local_cta_id_y_within_current_tile = local_cta_id_within_current_tile /
                                             x_dimension_of_last_tile;
        local_cta_id_x_within_current_tile = local_cta_id_within_current_tile %
                                             x_dimension_of_last_tile;
    } else {
        local_cta_id_y_within_current_tile = local_cta_id_within_current_tile /
                                             max_tile_width;
        local_cta_id_x_within_current_tile = local_cta_id_within_current_tile %
                                             max_tile_width;
    }

    const uint swizzled_v_thread_group_id_flattened = tile_id_of_current_cta *
                                                          max_tile_width +
                                                      local_cta_id_y_within_current_tile *
                                                          dispatch_grid_dim.x +
                                                      local_cta_id_x_within_current_tile;

    uint2 swizzled_thread_group_id;
    swizzled_thread_group_id.y = swizzled_v_thread_group_id_flattened /
                                 dispatch_grid_dim.x;
    swizzled_thread_group_id.x = swizzled_v_thread_group_id_flattened %
                                 dispatch_grid_dim.x;

    uint2 swizzled_thread_id;
    swizzled_thread_id.x = cta_dim.x * swizzled_thread_group_id.x + group_thread_id.x;
    swizzled_thread_id.y = cta_dim.y * swizzled_thread_group_id.y + group_thread_id.y;

    return swizzled_thread_id.xy;
}

} // namespace utils
} // namespace tundra

#endif // TNDR_SHADERS_UTILS_THREAD_GROUP_TILING_H
