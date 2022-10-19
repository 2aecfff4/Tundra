#pragma once
#include "core/core.h"

namespace tundra::rhi::config {

/// 8MiB
inline constexpr usize COMMAND_BUFFER_SIZE = 1024u * 1024u * 8u;

///
inline constexpr u32 MAX_FRAMES_IN_FLIGHT = 2;

///
inline constexpr u32 MAX_NUM_ATTACHMENTS = 6;

///
inline constexpr u32 MAX_NUM_COLOR_ATTACHMENTS = MAX_NUM_ATTACHMENTS - 1;

} // namespace tundra::rhi::config
