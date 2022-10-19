#pragma once
#include "core/core.h"
#include "core/std/assert.h"
#include "math/constants.h"
#include "math/math_utils.h"
#include "math/vector2.h"

namespace tundra::math {

template <concepts::number T>
class Vector3 {
public:
    static constexpr usize SIZE = 3;
    using Type = T;

public:
    T x;
    T y;
    T z;

public:
    constexpr Vector3() noexcept = default;

    template <concepts::number A>
    explicit constexpr Vector3(const A v) noexcept
        : x(static_cast<T>(v))
        , y(static_cast<T>(v))
        , z(static_cast<T>(v))
    {
    }

    template <concepts::number A, concepts::number B>
    constexpr Vector3(const Vector2<A> v, const B in_z = 0) noexcept
        : x(static_cast<T>(v.x))
        , y(static_cast<T>(v.y))
        , z(static_cast<T>(in_z))
    {
    }

    template <concepts::number A, concepts::number B, concepts::number C>
    constexpr Vector3(const A in_x, const B in_y, const C in_z) noexcept
        : x(static_cast<T>(in_x))
        , y(static_cast<T>(in_y))
        , z(static_cast<T>(in_z))
    {
    }

    template <concepts::number A>
    constexpr explicit Vector3(const A (&arr)[3]) noexcept
        : x(static_cast<T>(arr[0]))
        , y(static_cast<T>(arr[1]))
        , z(static_cast<T>(arr[2]))
    {
    }

    template <concepts::number B>
    constexpr Vector3(const Vector3<B>& rhs) noexcept
        : x(static_cast<T>(rhs.x))
        , y(static_cast<T>(rhs.y))
        , z(static_cast<T>(rhs.z))
    {
    }

public:
    constexpr Vector3& operator+=(const T s) noexcept
    {
        x += s;
        y += s;
        z += s;
        return *this;
    }

    template <concepts::number R>
    constexpr Vector3& operator+=(const Vector3<R>& v) noexcept
    {
        x += v.x;
        y += v.y;
        z += v.z;
        return *this;
    }

public:
    template <concepts::number R>
    constexpr Vector3& operator-=(const Vector3<R>& v) noexcept
    {
        x -= v.x;
        y -= v.y;
        z -= v.z;
        return *this;
    }

public:
    constexpr Vector3& operator*=(const T scale) noexcept
    {
        x *= scale;
        y *= scale;
        z *= scale;
        return *this;
    }

    template <concepts::number R>
    constexpr Vector3& operator*=(const Vector3<R>& v) noexcept
    {
        x *= v.x;
        y *= v.y;
        z *= v.z;
        return *this;
    }

public:
    template <concepts::number R>
    constexpr Vector3& operator/=(const R scale) noexcept
    {
        x /= static_cast<R>(scale);
        y /= static_cast<R>(scale);
        z /= static_cast<R>(scale);
        return *this;
    }

    template <concepts::number R>
    constexpr Vector3& operator/=(const Vector3<R>& v) noexcept
    {
        x /= v.x;
        y /= v.y;
        z /= v.z;
        return *this;
    }

    [[nodiscard]] constexpr Vector3 operator-() const noexcept
    {
        return Vector3 {
            -x,
            -y,
            -z,
        };
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
    [[nodiscard]] constexpr bool operator==(const Vector3<R>& v) const noexcept
    {
        return (x == v.x) && (y == v.y) && (z == v.z);
    }

    template <concepts::number R>
    [[nodiscard]] constexpr bool operator!=(const Vector3<R>& v) const noexcept
    {
        return (x != v.x) || (y != v.y) || (z != v.z);
    }

    template <concepts::number R>
    [[nodiscard]] constexpr bool operator<(const Vector3<R>& v) const noexcept
    {
        return (x < v.x) && (y < v.y) && (z < v.z);
    }

    template <concepts::number R>
    [[nodiscard]] constexpr bool operator>(const Vector3<R>& v) const noexcept
    {
        return (x > v.x) && (y > v.y) && (z > v.z);
    }

    template <concepts::number R>
    [[nodiscard]] constexpr bool operator<=(const Vector3<R>& v) const noexcept
    {
        return (x <= v.x) && (y <= v.y) && (z <= v.z);
    }

    template <concepts::number R>
    [[nodiscard]] constexpr bool operator>=(const Vector3<R>& v) const noexcept
    {
        return (x >= v.x) && (y >= v.y) && (x >= v.x);
    }
};

//////////////////////////////////////////////////////////////////////////
// Defines

using Vec3 = Vector3<f32>;
using DVec3 = Vector3<f64>;
using UVec3 = Vector3<u32>;
using IVec3 = Vector3<i32>;

//////////////////////////////////////////////////////////////////////////
// Math operations

template <concepts::number T>
[[nodiscard]] constexpr Vector3<T> operator-(const Vector3<T>& lhs) noexcept
{
    return Vector3<T> {
        -lhs.x,
        -lhs.y,
        -lhs.z,
    };
}

template <concepts::number L, concepts::number R>
[[nodiscard]] constexpr Vector3<L> operator+(const Vector3<L>& lhs, const R s) noexcept
{
    return Vector3<L> {
        lhs.x + s,
        lhs.y + s,
        lhs.z + s,
    };
}

template <concepts::number L, concepts::number R>
[[nodiscard]] constexpr Vector3<L> operator+(
    const Vector3<L>& lhs, const Vector3<R>& rhs) noexcept
{
    return Vector3<L> {
        lhs.x + rhs.x,
        lhs.y + rhs.y,
        lhs.z + rhs.z,
    };
}

template <concepts::number L, concepts::number R>
[[nodiscard]] constexpr Vector3<L> operator-(
    const Vector3<L>& lhs, const Vector3<R>& rhs) noexcept
{
    return Vector3<L> {
        lhs.x - rhs.x,
        lhs.y - rhs.y,
        lhs.z - rhs.z,
    };
}

template <concepts::number L, concepts::number R>
[[nodiscard]] constexpr Vector3<L> operator*(const Vector3<L>& lhs, const R scale) noexcept
{
    return Vector3<L> {
        lhs.x * scale,
        lhs.y * scale,
        lhs.z * scale,
    };
}

template <concepts::number L, concepts::number R>
[[nodiscard]] constexpr Vector3<L> operator*(const L scale, const Vector3<R>& rhs) noexcept
{
    return Vector3<L> { scale } * rhs;
}

template <concepts::number L, concepts::number R>
[[nodiscard]] constexpr Vector3<L> operator*(
    const Vector3<L>& lhs, const Vector3<R>& rhs) noexcept
{
    return Vector3<L> {
        lhs.x * rhs.x,
        lhs.y * rhs.y,
        lhs.z * rhs.z,
    };
}

template <concepts::number L, concepts::number R>
[[nodiscard]] constexpr Vector3<L> operator/(const Vector3<L>& lhs, const R scale) noexcept
{
    return Vector3<L> {
        lhs.x / scale,
        lhs.y / scale,
        lhs.z / scale,
    };
}

template <concepts::number L, concepts::number R>
[[nodiscard]] constexpr Vector3<L> operator/(const L scale, const Vector3<R>& rhs) noexcept
{
    return Vector3<L> { scale } / rhs;
}

template <concepts::number L, concepts::number R>
[[nodiscard]] constexpr Vector3<L> operator/(
    const Vector3<L>& lhs, const Vector3<R>& rhs) noexcept
{
    return Vector3<L> {
        lhs.x / rhs.x,
        lhs.y / rhs.y,
        lhs.z / rhs.z,
    };
}

template <concepts::number T>
[[nodiscard]] T length(const Vector3<T>& lhs) noexcept
{
    return math::sqrt((lhs.x * lhs.x) + (lhs.y * lhs.y) + (lhs.z * lhs.z));
}

template <concepts::number T>
[[nodiscard]] Vector3<T> normalize(const Vector3<T>& lhs) noexcept
{
    const T len = length(lhs);
    if (len != 0) {
        return Vector3<T> {
            lhs.x / len,
            lhs.y / len,
            lhs.z / len,
        };
    } else {
        return Vector3<T> { 0 };
    }
}

template <concepts::number L, concepts::number R>
[[nodiscard]] constexpr Vector3<L> cross(
    const Vector3<L>& lhs, const Vector3<R>& rhs) noexcept
{
    return Vector3<L> {
        lhs.y * rhs.z - lhs.z * rhs.y,
        lhs.z * rhs.x - lhs.x * rhs.z,
        lhs.x * rhs.y - lhs.y * rhs.x,
    };
}

template <std::floating_point L, std::floating_point R, std::floating_point T>
[[nodiscard]] constexpr Vector3<L> lerp(
    const Vector3<L>& p, const Vector3<R>& q, const T t) noexcept
{
    return ((1 - t) * p) + (t * q);
}

template <concepts::number L, concepts::number R>
[[nodiscard]] constexpr L dot(const Vector3<L>& lhs, const Vector3<R>& rhs) noexcept
{
    return lhs.x * rhs.x + //
           lhs.y * rhs.y + //
           lhs.z * rhs.z;
}

template <concepts::number L, concepts::number R>
[[nodiscard]] constexpr L distance(const Vector3<L>& lhs, const Vector3<R>& rhs) noexcept
{
    const auto a = rhs.x - lhs.x;
    const auto b = rhs.y - lhs.y;
    const auto c = rhs.z - lhs.z;
    return math::sqrt((a * a) + (b * b) + (c * c));
}

template <std::floating_point T>
[[nodiscard]] constexpr Vector3<T> cos(const Vector3<T>& v) noexcept
{
    return Vector3<T> {
        math::cos(v.x),
        math::cos(v.y),
        math::cos(v.z),
    };
}

template <std::floating_point T>
[[nodiscard]] constexpr Vector3<T> sin(const Vector3<T>& v) noexcept
{
    return Vector3<T> {
        math::sin(v.x),
        math::sin(v.y),
        math::sin(v.z),
    };
}

template <concepts::number T>
[[nodiscard]] constexpr Vector3<T> abs(const Vector3<T>& v) noexcept
{
    return Vector3<T> {
        math::abs(v.x),
        math::abs(v.y),
        math::sin(v.z),
    };
}

/// Converts degrees to radians.
template <std::floating_point T>
[[nodiscard]] constexpr Vector3<T> to_radians(const Vector3<T>& v) noexcept
{
    return Vector3<T> {
        math::to_radians(v.x),
        math::to_radians(v.y),
        math::to_radians(v.z),
    };
}

template <concepts::number L, concepts::number R>
[[nodiscard]] constexpr bool is_equal(
    const Vector3<L>& lhs,
    const Vector3<R>& rhs,
    const f64 tolerance = constants::VERY_SMALL_NUMBER) noexcept
{
    return (math::abs(lhs.x - rhs.x) <= tolerance) && //
           (math::abs(lhs.y - rhs.y) <= tolerance) && //
           (math::abs(lhs.z - rhs.z) <= tolerance);
}

template <concepts::number T>
[[nodiscard]] constexpr bool is_nearly_zero(
    const Vector3<T>& lhs, const f64 tolerance = constants::VERY_SMALL_NUMBER) noexcept
{
    return (math::abs(lhs.x) <= tolerance) && //
           (math::abs(lhs.y) <= tolerance) && //
           (math::abs(lhs.z) <= tolerance);
}

template <concepts::number T>
[[nodiscard]] constexpr bool is_zero(const Vector3<T>& lhs) noexcept
{
    return (lhs.x == static_cast<T>(0)) && //
           (lhs.y == static_cast<T>(0)) && //
           (lhs.z == static_cast<T>(0));
}

} // namespace tundra::math
