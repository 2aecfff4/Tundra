#pragma once
#include "core/core.h"

namespace tundra::math {

/// log2_indices<3>
/// 0 1 2 3 4 5 6 7 8 9 ...
/// | | | | | | | | | |
/// 0 | 0 | 0 | 0 | 0 |
///   1   |   1   |   1
///       2       |
///               3
[[nodiscard]] constexpr u64 log2_indices(
    const u64 max_value, const u64 frame_index) noexcept
{
    const u64 mod = 1ull << max_value;
    const u64 new_frame_index = (frame_index % mod) + 1;
    u64 v = new_frame_index & ~(new_frame_index - 1);
    u64 result = 0;
    while (v >>= 1u) {
        result += 1;
    }
    return result;
}

} // namespace tundra::math
