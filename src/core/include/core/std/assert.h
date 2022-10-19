#pragma once
#include "core/core_export.h"
#include "core/core.h"
#include <cassert>

namespace tundra::core {

/// Asserts if expr == false
#define tndr_assert(expr, msg) assert((expr) && !!(msg))

#if TNDR_DEBUG
/// Asserts if expr == false
#define tndr_debug_assert(expr, msg) tndr_assert(expr, msg)
#else
/// Asserts if expr == false
#define tndr_debug_assert(expr, msg)
#endif

} // namespace tundra::core
