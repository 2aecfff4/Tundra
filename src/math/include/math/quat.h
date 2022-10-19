#pragma once
#include "core/core.h"
#include "core/std/assert.h"
#include "math/constants.h"
#include "math/math_utils.h"
#include "math/vector3.h"
#include "math/vector4.h"

namespace tundra::math {

template <std::floating_point T>
class Quaternion {
public:
    static constexpr usize SIZE = 4;
    using Type = T;

public:
    T x;
    T y;
    T z;
    T w;

public:
    constexpr Quaternion() noexcept
        : x(0)
        , y(0)
        , z(0)
        , w(1)

    {
    }

    template <concepts::number A, concepts::number B, concepts::number C, concepts::number D>
    constexpr Quaternion(const A in_w, const B in_x, const C in_y, const D in_z) noexcept
        : x(static_cast<T>(in_x))
        , y(static_cast<T>(in_y))
        , z(static_cast<T>(in_z))
        , w(static_cast<T>(in_w))

    {
    }

    template <concepts::number A>
    constexpr explicit Quaternion(const A (&arr)[4]) noexcept
        : w(static_cast<T>(arr[0]))
        , x(static_cast<T>(arr[1]))
        , y(static_cast<T>(arr[2]))
        , z(static_cast<T>(arr[3]))

    {
    }

    template <concepts::number A, concepts::number B>
    constexpr Quaternion(const A s, const Vector3<B>& v) noexcept
        : w(static_cast<T>(s))
        , x(static_cast<T>(v.x))
        , y(static_cast<T>(v.y))
        , z(static_cast<T>(v.z))

    {
    }

    template <concepts::number A>
    constexpr Quaternion(const Vector3<A>& v) noexcept
        : w(static_cast<T>(1))
        , x(static_cast<T>(v.x))
        , y(static_cast<T>(v.y))
        , z(static_cast<T>(v.z))

    {
    }

    template <concepts::number A>
    constexpr Quaternion(const Quaternion<A>& q) noexcept
        : w(static_cast<T>(q.w))
        , x(static_cast<T>(q.x))
        , y(static_cast<T>(q.y))
        , z(static_cast<T>(q.z))

    {
    }

public:
    template <std::floating_point R>
    constexpr Quaternion& operator+=(const Quaternion<R>& q) noexcept
    {
        w += q.w;
        x += q.x;
        y += q.y;
        z += q.z;
        return *this;
    }

public:
    template <std::floating_point R>
    constexpr Quaternion& operator-=(const Quaternion<R>& q) noexcept
    {
        w -= q.w;
        x -= q.x;
        y -= q.y;
        z -= q.z;
        return *this;
    }

public:
    constexpr Quaternion& operator*=(const T scale) noexcept
    {
        w *= scale;
        x *= scale;
        y *= scale;
        z *= scale;
        return *this;
    }

    template <std::floating_point R>
    constexpr Quaternion& operator*=(const Quaternion<R>& rhs) noexcept
    {
        const Quaternion& lhs = *this;
        *this = Quaternion {
            lhs.w * rhs.w - lhs.x * rhs.x - lhs.y * rhs.y - lhs.z * rhs.z,
            lhs.w * rhs.x + lhs.x * rhs.w + lhs.y * rhs.z - lhs.z * rhs.y,
            lhs.w * rhs.y + lhs.y * rhs.w + lhs.z * rhs.x - lhs.x * rhs.z,
            lhs.w * rhs.z + lhs.z * rhs.w + lhs.x * rhs.y - lhs.y * rhs.x,
        };

        return *this;
    }

public:
    [[nodiscard]] T& operator[](const usize index) noexcept
    {
        tndr_debug_assert(index < SIZE, "");
        return (&x)[index];
    }

    [[nodiscard]] const T& operator[](const usize index) const noexcept
    {
        tndr_debug_assert(index < SIZE, "");
        return (&x)[index];
    }

public:
    template <concepts::number R>
    [[nodiscard]] constexpr bool operator==(const Quaternion<R>& v) const noexcept
    {
        return (x == v.x) && (y == v.y) && (z == v.z) && (w == v.w);
    }

    template <concepts::number R>
    [[nodiscard]] constexpr bool operator!=(const Quaternion<R>& v) const noexcept
    {
        return (x != v.x) || (y != v.y) || (z != v.z) || (w != v.w);
    }

public:
    [[nodiscard]] constexpr bool is_normalized() const noexcept
    {
        const auto length = math::sqrt((x * x) + (y * y) + (z * z) + (w * w));
        return (length - static_cast<T>(1)) <= static_cast<T>(constants::SMALL_NUMBER);
    }

public:
    template <concepts::number R>
    [[nodiscard]] static constexpr Quaternion from_angle(const Vector3<R>& angle) noexcept
    {
        const Vector3<R> s = math::sin(angle * static_cast<R>(0.5));
        const Vector3<R> c = math::cos(angle * static_cast<R>(0.5));

        return Quaternion {
            c.x * c.y * c.z + s.x * s.y * s.z,
            s.x * c.y * c.z - c.x * s.y * s.z,
            c.x * s.y * c.z + s.x * c.y * s.z,
            c.x * c.y * s.z - s.x * s.y * c.z,
        };
    }
};

//////////////////////////////////////////////////////////////////////////
// Defines

/// f32 quat
using Quat = Quaternion<f32>;

/// f64 quat
using DQuat = Quaternion<f64>;

//////////////////////////////////////////////////////////////////////////
// Math operations

template <std::floating_point L, std::floating_point R>
[[nodiscard]] constexpr L dot(const Quaternion<L>& lhs, const Quaternion<R>& rhs) noexcept
{
    return lhs.x * rhs.x + //
           lhs.y * rhs.y + //
           lhs.z * rhs.z + //
           lhs.w * rhs.w;
}

template <std::floating_point L, std::floating_point R>
[[nodiscard]] constexpr Quaternion<L> cross(
    const Quaternion<L>& lhs, const Quaternion<R>& rhs) noexcept
{
    return Quaternion<L> {
        lhs.w * rhs.w - lhs.x * rhs.x - lhs.y * rhs.y - lhs.z * rhs.z,
        lhs.w * rhs.x + lhs.x * rhs.w + lhs.y * rhs.z - lhs.z * rhs.y,
        lhs.w * rhs.y + lhs.y * rhs.w + lhs.z * rhs.x - lhs.x * rhs.z,
        lhs.w * rhs.z + lhs.z * rhs.w + lhs.x * rhs.y - lhs.y * rhs.x,
    };
}

template <std::floating_point T>
[[nodiscard]] constexpr Quaternion<T> conjugate(const Quaternion<T>& q) noexcept
{
    return Quaternion<T> {
        q.w,
        -q.x,
        -q.y,
        -q.z,
    };
}

template <std::floating_point T>
[[nodiscard]] constexpr Quaternion<T> inverse(const Quaternion<T>& q) noexcept
{
    return conjugate(q) / dot(q, q);
}

template <std::floating_point L, std::floating_point R, std::floating_point T>
[[nodiscard]] constexpr Quaternion<L> lerp(
    const Quaternion<L>& p, const Quaternion<R>& q, const T t) noexcept
{
    return ((1 - t) * p) + (t * q);
}

template <std::floating_point L, std::floating_point R>
[[nodiscard]] constexpr Quaternion<L> operator+(
    const Quaternion<L>& lhs, const Quaternion<R>& rhs) noexcept
{
    return Quaternion<L> {
        lhs.w + rhs.w,
        lhs.x + rhs.x,
        lhs.y + rhs.y,
        lhs.z + rhs.z,
    };
}

template <std::floating_point L, std::floating_point R>
[[nodiscard]] constexpr Quaternion<L> operator-(
    const Quaternion<L>& lhs, const Quaternion<R>& rhs) noexcept
{
    return Quaternion<L> {
        lhs.w - rhs.w,
        lhs.x - rhs.x,
        lhs.y - rhs.y,
        lhs.z - rhs.z,
    };
}

template <std::floating_point L, std::floating_point R>
[[nodiscard]] constexpr Quaternion<L> operator*(
    const Quaternion<L>& lhs, const Quaternion<R>& rhs) noexcept
{
    return Quaternion<L> {
        lhs.w * rhs.w - lhs.x * rhs.x - lhs.y * rhs.y - lhs.z * rhs.z,
        lhs.w * rhs.x + lhs.x * rhs.w + lhs.y * rhs.z - lhs.z * rhs.y,
        lhs.w * rhs.y + lhs.y * rhs.w + lhs.z * rhs.x - lhs.x * rhs.z,
        lhs.w * rhs.z + lhs.z * rhs.w + lhs.x * rhs.y - lhs.y * rhs.x,
    };
}

template <std::floating_point L, concepts::number R>
[[nodiscard]] constexpr Vector3<L> operator*(
    const Quaternion<L>& q, const Vector3<R>& v) noexcept
{
    tndr_debug_assert(q.is_normalized(), "Quaternion must be normalized!");

    const Vector3<L> quat_xyz {
        q.x,
        q.y,
        q.z,
    };

    const Vector3<L> uv(math::cross(quat_xyz, v));
    const Vector3<L> uuv(math::cross(quat_xyz, uv));

    return v + ((uv * q.w) + uuv) * static_cast<L>(2);
}

template <concepts::number L, std::floating_point R>
[[nodiscard]] constexpr Vector3<R> operator*(
    const Vector3<L>& v, const Quaternion<R>& q) noexcept
{
    return math::inverse(q) * v;
}

template <std::floating_point L, concepts::number R>
[[nodiscard]] constexpr Vector4<L> operator*(
    const Quaternion<L>& q, const Vector4<R>& v) noexcept
{
    return Vector4<L> {
        q * Vector3<R> { v.x, v.y, v.z },
        v.w,
    };
}

template <concepts::number L, std::floating_point R>
[[nodiscard]] constexpr Vector4<R> operator*(
    const Vector4<L>& v, const Quaternion<R>& q) noexcept
{
    return math::inverse(q) * v;
}

template <std::floating_point L, concepts::number R>
[[nodiscard]] constexpr Quaternion<L> operator*(const Quaternion<L>& q, const R s) noexcept
{
    return Quaternion<L> {
        q.w * s,
        q.x * s,
        q.y * s,
        q.z * s,
    };
}

template <concepts::number L, std::floating_point R>
[[nodiscard]] constexpr Quaternion<L> operator*(const L s, const Quaternion<R>& q) noexcept
{
    return q * s;
}

template <std::floating_point L, concepts::number R>
[[nodiscard]] constexpr Quaternion<L> operator/(const Quaternion<L>& q, const R s) noexcept
{
    return Quaternion<L> {
        q.w / s,
        q.x / s,
        q.y / s,
        q.z / s,
    };
}

template <std::floating_point L>
[[nodiscard]] constexpr Quaternion<L> operator+(const Quaternion<L>& q) noexcept
{
    return Quaternion<L> {
        +q.w,
        +q.x,
        +q.y,
        +q.z,
    };
}

template <std::floating_point L>
[[nodiscard]] constexpr Quaternion<L> operator-(const Quaternion<L>& q) noexcept
{
    return Quaternion<L> {
        -q.w,
        -q.x,
        -q.y,
        -q.z,
    };
}

template <std::floating_point T>
[[nodiscard]] constexpr T length(const Quaternion<T>& q) noexcept
{
    return math::sqrt(math::dot(q, q));
}

template <std::floating_point T>
[[nodiscard]] constexpr Quaternion<T> normalize(const Quaternion<T>& q) noexcept
{
    const T len = length(q);
    if (len != 0) {
        return q / len;
    } else {
        return Quaternion<T> {};
    }
}

} // namespace tundra::math
