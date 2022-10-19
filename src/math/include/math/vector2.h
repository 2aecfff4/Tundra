#pragma once
#include "core/core.h"
#include "core/std/assert.h"
#include "math/constants.h"
#include "math/math_utils.h"

namespace tundra::math {

template <concepts::number T>
class Vector2 {
public:
    static constexpr usize SIZE = 2;
    using Type = T;

public:
    T x;
    T y;

public:
    constexpr Vector2() noexcept = default;

    template <concepts::number A>
    constexpr explicit Vector2(const A v) noexcept
        : x(static_cast<T>(v))
        , y(static_cast<T>(v))
    {
    }

    template <concepts::number A, concepts::number B>
    constexpr Vector2(const A in_x, const B in_y) noexcept
        : x(static_cast<T>(in_x))
        , y(static_cast<T>(in_y))
    {
    }

    template <concepts::number A>
    constexpr explicit Vector2(const A (&arr)[2]) noexcept
        : x(static_cast<T>(arr[0]))
        , y(static_cast<T>(arr[1]))
    {
    }

    template <concepts::number B>
    constexpr Vector2(const Vector2<B>& rhs) noexcept
        : x(static_cast<T>(rhs.x))
        , y(static_cast<T>(rhs.y))
    {
    }

public:
    constexpr Vector2& operator+=(const T s) noexcept
    {
        x += s;
        y += s;
        return *this;
    }

    template <concepts::number R>
    constexpr Vector2& operator+=(const Vector2<R>& v) noexcept
    {
        x += v.x;
        y += v.y;
        return *this;
    }

public:
    template <concepts::number R>
    constexpr Vector2& operator-=(const Vector2<R>& v) noexcept
    {
        x -= v.x;
        y -= v.y;
        return *this;
    }

public:
    constexpr Vector2& operator*=(const T scale) noexcept
    {
        x *= scale;
        y *= scale;
        return *this;
    }

    template <concepts::number R>
    constexpr Vector2& operator*=(const Vector2<R>& v) noexcept
    {
        x *= v.x;
        y *= v.y;
        return *this;
    }

public:
    constexpr Vector2& operator/=(const T scale) noexcept
    {
        x /= scale;
        y /= scale;
        return *this;
    }

    template <concepts::number R>
    constexpr Vector2& operator/=(const Vector2<R>& v) noexcept
    {
        x /= v.x;
        y /= v.y;
        return *this;
    }

public:
    [[nodiscard]] T& operator[](const usize index) noexcept
    {
        tndr_debug_assert(index < SIZE, "Index out of bounds!");
        return (&x)[index];
    }

    [[nodiscard]] const T& operator[](const usize index) const noexcept
    {
        tndr_debug_assert(index < SIZE, "Index out of bounds!");
        return (&x)[index];
    }

public:
    template <concepts::number R>
    [[nodiscard]] constexpr bool operator==(const Vector2<R>& v) const noexcept
    {
        return (x == v.x) && (y == v.y);
    }

    template <concepts::number R>
    [[nodiscard]] constexpr bool operator!=(const Vector2<R>& v) const noexcept
    {
        return (x != v.x) || (y != v.y);
    }

    template <concepts::number R>
    [[nodiscard]] constexpr bool operator<(const Vector2<R>& v) const noexcept
    {
        return (x < v.x) && (y < v.y);
    }

    template <concepts::number R>
    [[nodiscard]] constexpr bool operator>(const Vector2<R>& v) const noexcept
    {
        return (x > v.x) && (y > v.y);
    }

    template <concepts::number R>
    [[nodiscard]] constexpr bool operator<=(const Vector2<R>& v) const noexcept
    {
        return (x <= v.x) && (y <= v.y);
    }

    template <concepts::number R>
    [[nodiscard]] constexpr bool operator>=(const Vector2<R>& v) const noexcept
    {
        return (x >= v.x) && (y >= v.y);
    }
};

//////////////////////////////////////////////////////////////////////////
// Defines

using Vec2 = Vector2<f32>;
using DVec2 = Vector2<f64>;
using UVec2 = Vector2<u32>;
using IVec2 = Vector2<i32>;

//////////////////////////////////////////////////////////////////////////
// Math operations

template <concepts::number T>
[[nodiscard]] constexpr Vector2<T> operator-(const Vector2<T>& lhs) noexcept
{
    return Vector2<T> {
        -lhs.x,
        -lhs.y,
    };
}

template <concepts::number L, concepts::number R>
[[nodiscard]] constexpr Vector2<L> operator+(const Vector2<L>& lhs, const R s) noexcept
{
    return Vector2<L> {
        lhs.x + s,
        lhs.y + s,
    };
}

template <concepts::number L, concepts::number R>
[[nodiscard]] constexpr Vector2<L> operator+(
    const Vector2<L>& lhs, const Vector2<R>& rhs) noexcept
{
    return Vector2<L> {
        lhs.x + rhs.x,
        lhs.y + rhs.y,
    };
}

template <concepts::number L, concepts::number R>
[[nodiscard]] constexpr Vector2<L> operator-(
    const Vector2<L>& lhs, const Vector2<R>& rhs) noexcept
{
    return Vector2<L> {
        lhs.x - rhs.x,
        lhs.y - rhs.y,
    };
}

template <concepts::number L, concepts::number R>
[[nodiscard]] constexpr Vector2<L> operator*(const Vector2<L>& lhs, const R scale) noexcept
{
    return Vector2<L> {
        lhs.x * scale,
        lhs.y * scale,
    };
}

template <concepts::number L, concepts::number R>
[[nodiscard]] constexpr Vector2<L> operator*(const L scale, const Vector2<R>& rhs) noexcept
{
    return Vector2<L> { scale } * rhs;
}

template <concepts::number L, concepts::number R>
[[nodiscard]] constexpr Vector2<L> operator*(
    const Vector2<L>& lhs, const Vector2<R>& rhs) noexcept
{
    return Vector2<L> {
        lhs.x * rhs.x,
        lhs.y * rhs.y,
    };
}

template <concepts::number L, concepts::number R>
[[nodiscard]] constexpr Vector2<L> operator/(const Vector2<L>& lhs, const R scale) noexcept
{
    return Vector2<L> {
        lhs.x / scale,
        lhs.y / scale,
    };
}

template <concepts::number L, concepts::number R>
[[nodiscard]] constexpr Vector2<L> operator/(const L scale, const Vector2<R>& rhs) noexcept
{
    return Vector2<L> { scale } / rhs;
}

template <concepts::number L, concepts::number R>
[[nodiscard]] constexpr Vector2<L> operator/(
    const Vector2<L>& lhs, const Vector2<R>& rhs) noexcept
{
    return Vector2<L> {
        lhs.x / rhs.x,
        lhs.y / rhs.y,
    };
}
template <concepts::number T>
[[nodiscard]] T length(const Vector2<T>& lhs) noexcept
{
    return math::sqrt((lhs.x * lhs.x) + (lhs.y * lhs.y));
}

template <concepts::number T>
[[nodiscard]] Vector2<T> normalize(const Vector2<T>& lhs) noexcept
{
    const T len = length(lhs);
    if (len != 0) {
        return Vector2<T> {
            lhs.x / len,
            lhs.y / len,
        };
    } else {
        return Vector2<T> { 0 };
    }
}

template <concepts::number L, concepts::number R>
[[nodiscard]] constexpr L cross(const Vector2<L>& lhs, const Vector2<R>& rhs) noexcept
{
    return lhs.x * rhs.y - lhs.y * rhs.x;
}

template <concepts::number L, concepts::number R>
[[nodiscard]] constexpr L dot(const Vector2<L>& lhs, const Vector2<R>& rhs) noexcept
{
    return lhs.x * rhs.x + lhs.y * rhs.y;
}

template <std::floating_point L, std::floating_point R, std::floating_point T>
[[nodiscard]] constexpr Vector2<L> lerp(
    const Vector2<L>& p, const Vector2<R>& q, const T t) noexcept
{
    return ((1 - t) * p) + (t * q);
}

template <concepts::number L, concepts::number R>
[[nodiscard]] constexpr L distance(const Vector2<L>& lhs, const Vector2<R>& rhs) noexcept
{
    const auto a = rhs.x - lhs.x;
    const auto b = rhs.y - lhs.y;
    return math::sqrt((a * a) + (b * b));
}

template <std::floating_point T>
[[nodiscard]] constexpr Vector2<T> cos(const Vector2<T>& v) noexcept
{
    return Vector2<T> {
        math::cos(v.x),
        math::cos(v.y),
    };
}

template <std::floating_point T>
[[nodiscard]] constexpr Vector2<T> sin(const Vector2<T>& v) noexcept
{
    return Vector2<T> {
        math::sin(v.x),
        math::sin(v.y),
    };
}

template <concepts::number T>
[[nodiscard]] constexpr Vector2<T> abs(const Vector2<T>& lhs) noexcept
{
    return Vector2 {
        math::abs(lhs.x),
        math::abs(lhs.y),
    };
}

template <concepts::number L, concepts::number R>
[[nodiscard]] constexpr bool is_equal(
    const Vector2<L>& lhs,
    const Vector2<R>& rhs,
    const f64 tolerance = constants::VERY_SMALL_NUMBER) noexcept
{
    return (math::abs(lhs.x - rhs.x) <= tolerance) && //
           (math::abs(lhs.y - rhs.y) <= tolerance);
}

template <concepts::number T>
[[nodiscard]] constexpr bool is_nearly_zero(
    const Vector2<T>& lhs, const f64 tolerance = constants::VERY_SMALL_NUMBER) noexcept
{
    return (math::abs(lhs.x) <= tolerance) && //
           (math::abs(lhs.y) <= tolerance);
}

template <concepts::number T>
[[nodiscard]] constexpr bool is_zero(const Vector2<T>& lhs) noexcept
{
    return (lhs.x == static_cast<T>(0)) && (lhs.y == static_cast<T>(0));
}

} // namespace tundra::math
