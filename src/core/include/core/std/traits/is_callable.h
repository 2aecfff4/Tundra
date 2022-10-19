#pragma once
#include "core/std/traits/declval.h"
#include <type_traits>

namespace tundra::core::traits {

///
template <typename F, typename ReturnType, typename... Args>
concept callable_with = requires(F f)
{
    { f(core::traits::declval<Args>()...) };
    requires std::is_same_v<ReturnType, decltype(f(core::traits::declval<Args>()...))>;
};

///
template <typename F, typename... Args>
concept callable = requires(F f)
{
    { f(core::traits::declval<Args>()...) };
};

} // namespace tundra::core::traits
