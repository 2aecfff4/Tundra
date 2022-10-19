#pragma once
#include "core/core.h"
#include "math/constants.h"
#include <cmath>
#include <concepts>
#include <type_traits>

namespace tundra::math {

namespace concepts {

template <typename T>
concept number = std::floating_point<T> || std::integral<T>;

} // namespace concepts

template <concepts::number T>
[[nodiscard]] constexpr T min(const T a, const T b) noexcept
{
    return (a <= b) ? a : b;
}

template <concepts::number T>
[[nodiscard]] constexpr T max(const T a, const T b) noexcept
{
    return (a >= b) ? a : b;
}

template <concepts::number T>
[[nodiscard]] constexpr T clamp(const T x, const T min, const T max) noexcept
{
    return (x < min) ? min : (x < max) ? x : max;
}

template <std::floating_point T, std::floating_point U>
[[nodiscard]] constexpr T lerp(const T a, const T b, const U alpha) noexcept
{
    return static_cast<T>(a + alpha * (b - a));
}

template <concepts::number T>
[[nodiscard]] constexpr T abs(const T value) noexcept
{
    return (value >= static_cast<T>(0)) ? value : -value;
}

template <std::integral T>
[[nodiscard]] constexpr T is_power_of_two(const T value) noexcept
{
    return ((value > static_cast<T>(0)) && ((value & (value - 1)) == static_cast<T>(0)));
}

template <concepts::number T>
[[nodiscard]] constexpr T square(const T value) noexcept
{
    return value * value;
}

template <std::floating_point T>
[[nodiscard]] inline T sqrt(const T value) noexcept
{
    return std::sqrt(value);
}

template <std::floating_point T>
[[nodiscard]] inline T pow(const T value, const T exp) noexcept
{
    return std::pow(value);
}

template <std::floating_point T>
[[nodiscard]] inline T cos(const T value) noexcept
{
    return std::cos(value);
}

template <std::floating_point T>
[[nodiscard]] inline T sin(const T value) noexcept
{
    return std::sin(value);
}

template <std::floating_point T>
[[nodiscard]] inline T acos(const T value) noexcept
{
    return std::acos(value);
}

template <std::floating_point T>
[[nodiscard]] inline T asin(const T value) noexcept
{
    return std::asin(value);
}

template <std::floating_point T>
[[nodiscard]] inline T tan(const T value) noexcept
{
    return std::tan(value);
}

template <std::floating_point T>
[[nodiscard]] inline T atan(const T value) noexcept
{
    return std::atan(value);
}

template <std::floating_point T>
[[nodiscard]] constexpr bool is_nan(const T value) noexcept
{
    return std::isnan(value);
}

template <std::floating_point T>
[[nodiscard]] constexpr bool is_finite(const T value) noexcept
{
    return std::isfinite(value);
}

template <std::floating_point T>
[[nodiscard]] constexpr T copysign(const T number, const T sign) noexcept
{
    return std::copysign(number, sign);
}

template <std::floating_point T>
[[nodiscard]] constexpr T ceil(const T value) noexcept
{
    return std::ceil(value);
}

/// Converts degrees to radians.
template <std::floating_point T>
[[nodiscard]] constexpr T to_radians(const T value) noexcept
{
    constexpr T pi = static_cast<T>(constants::PI);
    return value * (pi / static_cast<T>(180));
}

enum NoInit { NO_INIT };

} // namespace tundra::math
