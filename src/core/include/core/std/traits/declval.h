#pragma once
#include <type_traits>

namespace tundra::core::traits {

///
template <typename T>
std::add_rvalue_reference_t<T> declval() noexcept;

} // namespace tundra::core::traits
