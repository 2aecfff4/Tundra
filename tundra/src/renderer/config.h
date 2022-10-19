#pragma once
#include "core/core.h"

namespace tundra::renderer::config {

inline constexpr u32 MAX_INSTANCE_COUNT = 1 << 14;
inline constexpr u32 MAX_VISIBLE_MESHLETS_COUNT = (1 << 20) * 3;
inline constexpr u32 NUM_MESHLETS_PER_INDEX_BUFFER = 8192;
inline constexpr u32 INDEX_BUFFER_BATCH_SIZE = NUM_MESHLETS_PER_INDEX_BUFFER * 128u * 3u;
inline constexpr u32 NUM_INDEX_BUFFERS_IN_FIGHT = 4;

} // namespace tundra::renderer::config
