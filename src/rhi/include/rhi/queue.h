#pragma once
#include "rhi/rhi_export.h"
#include "core/core.h"

namespace tundra::rhi {

/// This struct allows to determine if physical queues are the same.
/// Useful for ownership transfers.
struct RHI_API QueueFamilyIndices {
    u32 graphics_queue;
    u32 compute_queue;
    u32 transfer_queue;
    u32 present_queue;
};

} // namespace tundra::rhi
