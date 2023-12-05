#pragma once
#include "core/core.h"
#include "core/utils/enum_flags.h"

namespace tundra::rhi {

///
enum class GraphicsAPI : u8 {
    Vulkan,
    None,
};

///
enum class QueueType : u8 {
    Compute,
    Graphics,
    Transfer,
    Present,
};

///
enum class SynchronizationStage : u16 {
    TOP_OF_PIPE = 1 << 0,
    BOTTOM_OF_PIPE = 1 << 1,
    EARLY_FRAGMENT_TESTS = 1 << 2,
    LATE_FRAGMENT_TESTS = 1 << 3,
    VERTEX_SHADER = 1 << 4,
    FRAGMENT_SHADER = 1 << 5,
    TASK_SHADER = 1 << 6,
    MESH_SHADER = 1 << 7,
    COMPUTE_SHADER = 1 << 8,
    TRANSFER = 1 << 9,
    ALL_GRAPHICS = TOP_OF_PIPE | BOTTOM_OF_PIPE | EARLY_FRAGMENT_TESTS |
                   LATE_FRAGMENT_TESTS | VERTEX_SHADER | FRAGMENT_SHADER | TASK_SHADER |
                   MESH_SHADER,
    NONE = 0,
};

TNDR_ENUM_CLASS_FLAGS(SynchronizationStage)

} // namespace tundra::rhi
