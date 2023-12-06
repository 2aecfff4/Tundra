#pragma once
#include "core/core.h"

namespace tundra::rhi {

///
struct DrawMeshTasksIndirectCommand {
    u32 x;
    u32 y;
    u32 z;
};

} // namespace tundra::rhi
