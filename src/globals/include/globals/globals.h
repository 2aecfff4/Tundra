#pragma once
#include "globals/globals_export.h"

namespace tundra::rhi {
class IRHIContext;
}

namespace tundra::globals {

/// Global pointer to the rhi context. May be nullptr.
extern GLOBALS_API rhi::IRHIContext* g_rhi_context;

GLOBALS_API bool is_exit_requested() noexcept;
GLOBALS_API void request_exit() noexcept;

} // namespace tundra::globals
