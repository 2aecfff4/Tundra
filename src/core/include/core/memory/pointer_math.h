#pragma once
#include "core/core.h"
#include "core/std/assert.h"
#include <type_traits>

namespace tundra::core::pointer_math {

template <typename T>
[[nodiscard]] constexpr usize get_alignment(const usize count) noexcept
{
    const usize max_align = alignof(std::max_align_t);
    const usize size = (sizeof(T) * count);
    const usize rem = size % max_align;
    const usize align = rem != 0 ? max_align - rem : 0;
    return size + align;
}

[[nodiscard]] constexpr usize get_alignment(
    const usize type_size, const usize count) noexcept
{
    const usize max_align = alignof(std::max_align_t);
    const usize size = (type_size * count);
    const usize rem = size % max_align;
    const usize align = rem != 0 ? max_align - rem : 0;
    return size + align;
}

template <typename T, typename V>
[[nodiscard]] inline T* add(T* const ptr, const V value) noexcept //
    requires(std::is_integral_v<V> || std::is_pointer_v<V>)       //
{
    return reinterpret_cast<T*>(std::uintptr_t(ptr) + std::uintptr_t(value));
}

template <typename T, typename V>
[[nodiscard]] inline T* subtract(T* const ptr, const V value) noexcept //
    requires(std::is_integral_v<V> || std::is_pointer_v<V>)            //
{
    return reinterpret_cast<T*>(std::uintptr_t(ptr) - std::uintptr_t(value));
}

template <typename T>
[[nodiscard]] inline T* align(T* const ptr, const usize alignment) noexcept
{
    tndr_debug_assert(
        alignment && !(alignment & (alignment - 1)), "`alignment` must be power of two!");
    return reinterpret_cast<T*>((std::uintptr_t(ptr) + alignment - 1) & ~(alignment - 1));
}

} // namespace tundra::core::pointer_math
