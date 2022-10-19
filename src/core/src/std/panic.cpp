#include "core/std/panic.h"
#include <fmt/core.h>
#include <exception>
#include <iostream>

namespace tundra::core {
namespace tndr_panic_internal {

CORE_API void panic_internal(const std::string& msg) noexcept
{
    // #TODO: Proper callstack etc
    fmt::print("Panic!\n{}", msg);
    std::terminate();
}

} // namespace tndr_panic_internal

void unreachable() noexcept
{
    std::cout << "Internal error: entered unreachable code!\n";
    std::terminate();
}

} // namespace tundra::core
