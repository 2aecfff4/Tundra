#pragma once
#include "core/core_export.h"
#include "core/core.h"
#include "core/std/utils.h"
#include <fmt/core.h>

namespace tundra::core {

namespace tndr_panic_internal {

///
[[noreturn]] CORE_API void panic_internal(const std::string& msg) noexcept;

} // namespace tndr_panic_internal

///
template <typename... Args>
[[noreturn]] void panic(const char* msg, Args&&... args) noexcept
{
    const auto formatted_msg = fmt::format(msg, core::forward<Args>(args)...);
    tndr_panic_internal::panic_internal(formatted_msg);
}

///
[[noreturn]] CORE_API void unreachable() noexcept;

} // namespace tundra::core
