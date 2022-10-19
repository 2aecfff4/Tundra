#pragma once
#include "core/core.h"
#include "core/std/utils.h"

namespace tundra::core {

///
template <typename Func>
struct DeferScope {
    Func func;

    DeferScope(Func&& f) noexcept
        : func(core::move(f))
    {
    }

    ~DeferScope()
    {
        func();
    }

    DeferScope(const DeferScope&) noexcept = delete;
    DeferScope(DeferScope&&) noexcept = delete;
    DeferScope& operator=(const DeferScope&) noexcept = delete;
    DeferScope& operator=(DeferScope&&) noexcept = delete;
};

///
struct DeferHelper {
    template <typename Func>
    [[nodiscard]] DeferScope<Func> operator<<=(Func&& func) noexcept
    {
        return DeferScope<Func> { core::forward<Func>(func) };
    }
};

/// # Example
///
/// ```
/// tndr_defer {
///     delete p;
/// };
/// ```
#define tndr_defer                                                                       \
    [[maybe_unused]] const auto TNDR_APPEND(                                             \
        tndr__defer__scope__, __LINE__) = tundra::core::DeferHelper {} <<= [&]

} // namespace tundra::core
