#pragma once
#include "core/core.h"
#include <type_traits>

namespace tundra::core {

namespace endian_swap_private {

template <typename T, usize size = sizeof(T)>
struct EndianSwapper;

template <typename T>
struct EndianSwapper<T, 1> {
    [[nodiscard]] static constexpr T swap(const T value) noexcept
    {
        return value;
    }
};

template <typename T>
struct EndianSwapper<T, 2> {
    [[nodiscard]] static constexpr T swap(const T value) noexcept
    {
        return (((value & 0x00FF) << 8) | ((value & 0xFF00) >> 8));
    }
};

template <typename T>
struct EndianSwapper<T, 4> {
    [[nodiscard]] static constexpr T swap(const T value) noexcept
    {
        return (
            ((value & 0x000000FF) << 24) | ((value & 0x0000FF00) << 8) |
            ((value & 0x00FF0000) >> 8) | ((value & 0xFF000000) >> 24));
    }
};

template <typename T>
struct EndianSwapper<T, 8> {
    [[nodiscard]] static constexpr T swap(const T value) noexcept
    {
        return (
            ((value & 0x00000000000000FF) << 56) | ((value & 0x000000000000FF00) << 40) |
            ((value & 0x0000000000FF0000) << 24) | ((value & 0x00000000FF000000) << 8) |
            ((value & 0x000000FF00000000) >> 8) | ((value & 0x0000FF0000000000) >> 24) |
            ((value & 0x00FF000000000000) >> 40) | ((value & 0xFF00000000000000) >> 56));
    }
};

} // namespace endian_swap_private

template <typename T>
[[nodiscard]] constexpr T endian_swap(const T value) noexcept
    requires(std::is_integral_v<T> || std::is_enum_v<T>)
{
    return endian_swap_private::EndianSwapper<T>::swap(value);
}

[[nodiscard]] constexpr bool is_little_endian() noexcept
{
    return TNDR_LITTLE_ENDIAN == 1;
}

} // namespace tundra::core
