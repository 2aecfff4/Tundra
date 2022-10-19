#pragma once
#include "core/core.h"

namespace tundra::rhi {

///
struct DrawIndexedIndirectCommand {
    u32 index_count = 0;
    u32 instance_count = 0;
    u32 first_index = 0;
    i32 vertex_offset = 0;
    u32 first_instance = 0;
};

} // namespace tundra::rhi
