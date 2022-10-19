#pragma once
#include "core/core.h"
#include "core/std/assert.h"
#include "math/constants.h"
#include "math/math_utils.h"
#include "math/vector3.h"

namespace tundra::math {

template <concepts::number T>
class Vector4 {
public:
    static constexpr usize SIZE = 4;
    using Type = T;

public:
    T x;
    T y;
    T z;
    T w;

public:
    constexpr Vector4() noexcept = default;

    template <concepts::number R>
    constexpr explicit Vector4(const R v) noexcept
        : x(static_cast<T>(v))
        , y(static_cast<T>(v))
        , z(static_cast<T>(v))
        , w(static_cast<T>(v))
    {
    }

    template <concepts::number A, concepts::number B>
    constexpr Vector4(const Vector3<A> v, const B in_w = 1) noexcept
        : x(static_cast<T>(v.x))
        , y(static_cast<T>(v.y))
        , z(static_cast<T>(v.z))
        , w(static_cast<T>(in_w))
    {
    }

    template <concepts::number A, concepts::number B, concepts::number C, concepts::number D>
    constexpr Vector4(const A in_x, const B in_y, const C in_z, const D in_w) noexcept
        : x(static_cast<T>(in_x))
        , y(static_cast<T>(in_y))
        , z(static_cast<T>(in_z))
        , w(static_cast<T>(in_w))
    {
    }

    template <concepts::number R>
    constexpr explicit Vector4(const R (&arr)[4]) noexcept
        : x(static_cast<T>(arr[0]))
        , y(static_cast<T>(arr[1]))
        , z(static_cast<T>(arr[2]))
        , w(static_cast<T>(arr[3]))
    {
    }

    template <concepts::number B>
    constexpr Vector4(const Vector4<B>& rhs) noexcept
        : x(static_cast<T>(rhs.x))
        , y(static_cast<T>(rhs.y))
        , z(static_cast<T>(rhs.z))
        , w(static_cast<T>(rhs.w))
    {
    }

public:
    constexpr Vector4& operator+=(const T s) noexcept
    {
        x += s;
        y += s;
        z += s;
        w += s;
        return *this;
    }

    template <concepts::number R>
    constexpr Vector4& operator+=(const Vector4<R>& v) noexcept
    {
        x += v.x;
        y += v.y;
        z += v.z;
        w += v.w;
        return *this;
    }

public:
    template <concepts::number R>
    constexpr Vector4& operator-=(const Vector4<R>& v) noexcept
    {
        x -= v.x;
        y -= v.y;
        z -= v.z;
        w -= v.w;
        return *this;
    }

public:
    constexpr Vector4& operator*=(const T scale) noexcept
    {
        x *= scale;
        y *= scale;
        z *= scale;
        w *= scale;
        return *this;
    }

    template <concepts::number R>
    constexpr Vector4& operator*=(const Vector4<R>& v) noexcept
    {
        x *= v.x;
        y *= v.y;
        z *= v.z;
        w *= v.w;
        return *this;
    }

public:
    constexpr Vector4& operator/=(const T scale) noexcept
    {
        x /= scale;
        y /= scale;
        z /= scale;
        w /= scale;
        return *this;
    }

    template <concepts::number R>
    constexpr Vector4& operator/=(const Vector4<R>& v) noexcept
    {
        x /= v.x;
        y /= v.y;
        z /= v.z;
        w /= v.w;
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
    [[nodiscard]] constexpr bool operator==(const Vector4<R>& v) const noexcept
    {
        return (x == v.x) && (y == v.y) && (z == v.z) && (w == v.w);
    }

    template <concepts::number R>
    [[nodiscard]] constexpr bool operator!=(const Vector4<R>& v) const noexcept
    {
        return (x != v.x) || (y != v.y) || (z != v.z) || (w != v.w);
    }

    template <concepts::number R>
    [[nodiscard]] constexpr bool operator<(const Vector4<R>& v) const noexcept
    {
        return (x < v.x) && (y < v.y) && (z < v.z) && (w < v.w);
    }

    template <concepts::number R>
    [[nodiscard]] constexpr bool operator>(const Vector4<R>& v) const noexcept
    {
        return (x > v.x) && (y > v.y) && (z > v.z) && (w > v.w);
    }

    template <concepts::number R>
    [[nodiscard]] constexpr bool operator<=(const Vector4<R>& v) const noexcept
    {
        return (x <= v.x) && (y <= v.y) && (z <= v.z) && (w <= v.w);
    }

    template <concepts::number R>
    [[nodiscard]] constexpr bool operator>=(const Vector4<R>& v) const noexcept
    {
        return (x >= v.x) && (y >= v.y) && (x >= v.x) && (w >= v.w);
    }
};

//////////////////////////////////////////////////////////////////////////
// Defines

using Vec4 = Vector4<f32>;
using DVec4 = Vector4<f64>;
using UVec4 = Vector4<u32>;
using IVec4 = Vector4<i32>;

//////////////////////////////////////////////////////////////////////////
// Math operations

template <concepts::number T>
[[nodiscard]] constexpr Vector4<T> operator-(const Vector4<T>& lhs) noexcept
{
    return Vector4<T> {
        -lhs.x,
        -lhs.y,
        -lhs.z,
        -lhs.w,
    };
}

template <concepts::number L, concepts::number R>
[[nodiscard]] constexpr Vector4<L> operator+(const Vector4<L>& lhs, const R s) noexcept
{
    return Vector4<L> {
        lhs.x + s,
        lhs.y + s,
        lhs.z + s,
        lhs.w + s,
    };
}

template <concepts::number L, concepts::number R>
[[nodiscard]] constexpr Vector4<L> operator+(
    const Vector4<L>& lhs, const Vector4<R>& rhs) noexcept
{
    return Vector4<L> {
        lhs.x + rhs.x,
        lhs.y + rhs.y,
        lhs.z + rhs.z,
        lhs.w + rhs.w,
    };
}

template <concepts::number L, concepts::number R>
[[nodiscard]] constexpr Vector4<L> operator-(
    const Vector4<L>& lhs, const Vector4<R>& rhs) noexcept
{
    return Vector4<L> {
        lhs.x - rhs.x,
        lhs.y - rhs.y,
        lhs.z - rhs.z,
        lhs.w - rhs.w,
    };
}

template <concepts::number L, concepts::number R>
[[nodiscard]] constexpr Vector4<L> operator*(const Vector4<L>& lhs, const R scale) noexcept
{
    return Vector4<L> {
        lhs.x * scale,
        lhs.y * scale,
        lhs.z * scale,
        lhs.w * scale,
    };
}

template <concepts::number L, concepts::number R>
[[nodiscard]] constexpr Vector4<L> operator*(const L scale, const Vector4<R>& rhs) noexcept
{
    return Vector4<L> { scale } * rhs;
}

template <concepts::number L, concepts::number R>
[[nodiscard]] constexpr Vector4<L> operator*(
    const Vector4<L>& lhs, const Vector4<R>& rhs) noexcept
{
    return Vector4<L> {
        lhs.x * rhs.x,
        lhs.y * rhs.y,
        lhs.z * rhs.z,
        lhs.w * rhs.w,
    };
}

template <concepts::number L, concepts::number R>
[[nodiscard]] constexpr Vector4<L> operator/(const Vector4<L>& lhs, const R scale) noexcept
{
    return Vector4<L> {
        lhs.x / scale,
        lhs.y / scale,
        lhs.z / scale,
        lhs.w / scale,
    };
}

template <concepts::number L, concepts::number R>
[[nodiscard]] constexpr Vector4<L> operator/(const L scale, const Vector4<R>& rhs) noexcept
{
    return Vector4<L> { scale } / rhs;
}

template <concepts::number L, concepts::number R>
[[nodiscard]] constexpr Vector4<L> operator/(
    const Vector4<L>& lhs, const Vector4<R>& rhs) noexcept
{
    return Vector4<L> {
        lhs.x / rhs.x,
        lhs.y / rhs.y,
        lhs.z / rhs.z,
        lhs.w / rhs.w,
    };
}

template <concepts::number T>
[[nodiscard]] T length(const Vector4<T>& lhs) noexcept
{
    return math::sqrt(
        (lhs.x * lhs.x) + (lhs.y * lhs.y) + (lhs.z * lhs.z) + (lhs.w * lhs.w));
}

template <concepts::number T>
[[nodiscard]] T length3(const Vector4<T>& lhs) noexcept
{
    return math::sqrt((lhs.x * lhs.x) + (lhs.y * lhs.y) + (lhs.z * lhs.z));
}

template <concepts::number T>
[[nodiscard]] Vector4<T> normalize(const Vector4<T>& lhs) noexcept
{
    const T len = length(lhs);
    if (len != 0) {
        return Vector4<T> {
            lhs.x / len,
            lhs.y / len,
            lhs.z / len,
            lhs.w / len,
        };
    } else {
        return Vector4<T> { 0 };
    }
}

template <concepts::number L, concepts::number R>
[[nodiscard]] constexpr Vector4<L> cross(
    const Vector4<L>& lhs, const Vector4<R>& rhs) noexcept
{
    return Vector4<L> {
        lhs.y * rhs.z - lhs.z * rhs.y,
        lhs.z * rhs.x - lhs.x * rhs.z,
        lhs.x * rhs.y - lhs.y * rhs.x,
        0,
    };
}

template <concepts::number L, concepts::number R>
[[nodiscard]] constexpr L dot(const Vector4<L>& lhs, const Vector4<R>& rhs) noexcept
{
    return lhs.x * rhs.x + //
           lhs.y * rhs.y + //
           lhs.z * rhs.z + //
           lhs.w * rhs.w;
}

template <std::floating_point L, std::floating_point R, std::floating_point T>
[[nodiscard]] constexpr Vector4<L> lerp(
    const Vector4<L>& p, const Vector4<R>& q, const T t) noexcept
{
    return ((1 - t) * p) + (t * q);
}

template <concepts::number L, concepts::number R>
[[nodiscard]] constexpr L distance(const Vector4<L>& lhs, const Vector4<R>& rhs) noexcept
{
    const auto a = rhs.x - lhs.x;
    const auto b = rhs.y - lhs.y;
    const auto c = rhs.z - lhs.z;
    const auto d = rhs.w - lhs.w;
    return math::sqrt((a * a) + (b * b) + (c * c) + (d * d));
}

template <std::floating_point T>
[[nodiscard]] constexpr Vector4<T> cos(const Vector4<T>& v) noexcept
{
    return Vector4<T> {
        math::cos(v.x),
        math::cos(v.y),
        math::cos(v.z),
        math::cos(v.w),
    };
}

template <std::floating_point T>
[[nodiscard]] constexpr Vector4<T> sin(const Vector4<T>& v) noexcept
{
    return Vector4<T> {
        math::sin(v.x),
        math::sin(v.y),
        math::sin(v.z),
        math::sin(v.w),
    };
}

template <concepts::number T>
[[nodiscard]] constexpr Vector4<T> abs(const Vector4<T>& v) noexcept
{
    return Vector3<T> {
        math::abs(v.x),
        math::abs(v.y),
        math::sin(v.z),
        math::sin(v.w),
    };
}

template <concepts::number L, concepts::number R>
[[nodiscard]] constexpr bool is_equal(
    const Vector4<L>& lhs,
    const Vector4<R>& rhs,
    const f64 tolerance = constants::VERY_SMALL_NUMBER) noexcept
{
    return (math::abs(lhs.x - rhs.x) <= tolerance) && //
           (math::abs(lhs.y - rhs.y) <= tolerance) && //
           (math::abs(lhs.z - rhs.z) <= tolerance) && //
           (math::abs(lhs.w - rhs.z) <= tolerance);
}

template <concepts::number T>
[[nodiscard]] constexpr bool is_nearly_zero(
    const Vector4<T>& lhs, const f64 tolerance = constants::VERY_SMALL_NUMBER) noexcept
{
    return (math::abs(lhs.x) <= tolerance) && //
           (math::abs(lhs.y) <= tolerance) && //
           (math::abs(lhs.z) <= tolerance) && //
           (math::abs(lhs.w) <= tolerance);
}

template <concepts::number T>
[[nodiscard]] constexpr bool is_zero(const Vector4<T>& lhs) noexcept
{
    return (lhs.x == static_cast<T>(0)) && //
           (lhs.y == static_cast<T>(0)) && //
           (lhs.z == static_cast<T>(0)) && //
           (lhs.w == static_cast<T>(0));
}

} // namespace tundra::math
