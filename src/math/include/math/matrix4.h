#pragma once
#include "core/core.h"
#include "core/std/assert.h"
#include "math/constants.h"
#include "math/math_utils.h"
#include "math/vector3.h"
#include "math/vector4.h"

namespace tundra::math {

///
template <std::floating_point T>
class Matrix4 {
public:
    static constexpr usize SIZE = 4;
    using Type = T;

public:
    using ColumnType = Vector4<T>;
    using RowType = Vector4<T>;

public:
    ColumnType matrix[4];

public:
    constexpr Matrix4() noexcept
        : matrix {
            ColumnType { 1, 0, 0, 0 },
            ColumnType { 0, 1, 0, 0 },
            ColumnType { 0, 0, 1, 0 },
            ColumnType { 0, 0, 0, 1 },
        }
    {
    }

    template <std::floating_point R>
    constexpr explicit Matrix4(const R s) noexcept
        : matrix {
            ColumnType { s, 0, 0, 0 },
            ColumnType { 0, s, 0, 0 },
            ColumnType { 0, 0, s, 0 },
            ColumnType { 0, 0, 0, s },
        }
    {
    }

    constexpr explicit Matrix4(const T (&m)[16]) noexcept
        : matrix {
            ColumnType { m[0], m[1], m[2], m[3] },
            ColumnType { m[4], m[5], m[6], m[7] },
            ColumnType { m[8], m[9], m[10], m[11] },
            ColumnType { m[12], m[13], m[14], m[15] },
        }
    {
    }

    constexpr Matrix4(
        const ColumnType& col0,
        const ColumnType& col1,
        const ColumnType& col2,
        const ColumnType& col3) noexcept
        : matrix { col0, col1, col2, col3 }
    {
    }

public:
    template <std::floating_point R>
    Matrix4& operator+=(const R s) noexcept
    {
        matrix[0] += s;
        matrix[1] += s;
        matrix[2] += s;
        matrix[3] += s;
        return *this;
    }

    template <std::floating_point R>
    Matrix4& operator+=(const Matrix4<R>& rhs) noexcept
    {
        matrix[0] += rhs[0];
        matrix[1] += rhs[1];
        matrix[2] += rhs[2];
        matrix[3] += rhs[3];
        return *this;
    }

    template <std::floating_point R>
    Matrix4& operator-=(const R s) noexcept
    {
        matrix[0] -= s;
        matrix[1] -= s;
        matrix[2] -= s;
        matrix[3] -= s;
        return *this;
    }

    template <std::floating_point R>
    Matrix4& operator-=(const Matrix4<R>& rhs) noexcept
    {
        matrix[0] -= rhs[0];
        matrix[1] -= rhs[1];
        matrix[2] -= rhs[2];
        matrix[3] -= rhs[3];
        return *this;
    }

    template <std::floating_point R>
    Matrix4& operator*=(const R s) noexcept
    {
        matrix[0] *= s;
        matrix[1] *= s;
        matrix[2] *= s;
        matrix[3] *= s;
        return *this;
    }

    template <std::floating_point R>
    Matrix4& operator*=(const Matrix4<R>& rhs) noexcept
    {
        *this = (*this * rhs);
        return *this;
    }

    template <std::floating_point R>
    Matrix4& operator/=(const R s) noexcept
    {
        matrix[0] /= s;
        matrix[1] /= s;
        matrix[2] /= s;
        matrix[3] /= s;
        return *this;
    }

    template <std::floating_point R>
    Matrix4& operator/=(const Matrix4<R>& rhs) noexcept
    {
        *this = (*this * inverse(rhs));
        return *this;
    }

public:
    [[nodiscard]] ColumnType& operator[](const usize index) noexcept
    {
        tndr_debug_assert(index < SIZE, "Index out of bounds!");
        return matrix[index];
    }

    [[nodiscard]] const ColumnType& operator[](const usize index) const noexcept
    {
        tndr_debug_assert(index < SIZE, "Index out of bounds!");
        return matrix[index];
    }

public:
    /// Creates a reversed-Z matrix for a right handed normalized symmetric perspective-view frustum with infinite far plane.
    template <concepts::number A, std::floating_point B, concepts::number C>
    [[nodiscard]] static Matrix4 perspective_infinite(
        const A fov_y_radians, const B aspect_ratio, const C z_near) noexcept
    {
        const T f = T(1) / math::tan(static_cast<T>(fov_y_radians) * T(0.5f));

        // clang-format off
        return Matrix4 {
            ColumnType { f / static_cast<T>(aspect_ratio), 0,     0,                      0, },
            ColumnType { 0,                                f,     0,                      0, },
            ColumnType { 0,                                0,     0,                     -1, },
            ColumnType { 0,                                0,     static_cast<T>(z_near), 0, },
        };
        // clang-format on
    }

    /// Creates a reversed-Z matrix for a right handed normalized symmetric perspective-view frustum with infinite far plane.
    template <concepts::number A, std::floating_point B, concepts::number C>
    [[nodiscard]] static Matrix4 perspective(
        const A fov_y_radians,
        const B aspect_ratio,
        const C z_near,
        const C z_far) noexcept
    {
        const T f = T(1) / math::tan(static_cast<T>(fov_y_radians) * T(0.5f));

        return Matrix4 {
            ColumnType {
                f / static_cast<T>(aspect_ratio),
                0,
                0,
                0,
            },
            ColumnType {
                0,
                f,
                0,
                0,
            },
            ColumnType {
                0,
                0,
                z_near / (z_far - z_near),
                -1,
            },
            ColumnType {
                0,
                0,
                (z_far * z_near) / (z_far - z_near),
                0,
            },
        };
    }

    [[nodiscard]] static Matrix4 orthographic(
        const T left,
        const T right,
        const T bottom,
        const T top,
        const T in_near,
        const T in_far) noexcept
    {
        const T width = right - left;
        const T height = top - bottom;

        return Matrix4 {
            ColumnType {
                T(2) / (right - left),
                0,
                0,
                0,
            },
            ColumnType {
                0,
                T(2) / (top - bottom),
                0,
                0,
            },
            ColumnType {
                0,
                0,
                T(1) / (in_near - in_far),
                0,
            },
            ColumnType {
                -(left + right) * (T(1) / width),
                -(top + bottom) * (T(1) / height),
                (T(1) / (in_near - in_far)) * in_near,
                1,
            },
        };
    }

    /// rh
    [[nodiscard]] static Matrix4 look_at(
        const Vector3<T>& eye, const Vector3<T>& center, const Vector3<T>& up) noexcept
    {
        const Vector3<T> dir = eye - center;
        const Vector3<T> f = math::normalize(dir);
        const Vector3<T> s = math::normalize(math::cross(up, f));
        const Vector3<T> u = math::cross(f, s);

        return Matrix4 {
            ColumnType { s.x, u.x, f.x, 0 },
            ColumnType { s.y, u.y, f.y, 0 },
            ColumnType { s.z, u.z, f.z, 0 },
            ColumnType { -math::dot(s, eye), -math::dot(u, eye), -math::dot(f, eye), 1 },
        };
    }
};

//////////////////////////////////////////////////////////////////////////
// Defines

using Mat4 = Matrix4<f32>;
using DMat4 = Matrix4<f64>;

//////////////////////////////////////////////////////////////////////////
// Math operations

template <std::floating_point T>
[[nodiscard]] constexpr Matrix4<T> inverse(const Matrix4<T>& m) noexcept
{
    const T a0112 = m[1].x * m[2].y - m[1].y * m[2].x;
    const T a0113 = m[1].x * m[3].y - m[1].y * m[3].x;
    const T a0123 = m[2].x * m[3].y - m[2].y * m[3].x;
    const T a0212 = m[1].x * m[2].z - m[1].z * m[2].x;
    const T a0213 = m[1].x * m[3].z - m[1].z * m[3].x;
    const T a0223 = m[2].x * m[3].z - m[2].z * m[3].x;
    const T a0312 = m[1].x * m[2].w - m[1].w * m[2].x;
    const T a0313 = m[1].x * m[3].w - m[1].w * m[3].x;
    const T a0323 = m[2].x * m[3].w - m[2].w * m[3].x;
    const T a1212 = m[1].y * m[2].z - m[1].z * m[2].y;
    const T a1213 = m[1].y * m[3].z - m[1].z * m[3].y;
    const T a1223 = m[2].y * m[3].z - m[2].z * m[3].y;
    const T a1312 = m[1].y * m[2].w - m[1].w * m[2].y;
    const T a1313 = m[1].y * m[3].w - m[1].w * m[3].y;
    const T a1323 = m[2].y * m[3].w - m[2].w * m[3].y;
    const T a2312 = m[1].z * m[2].w - m[1].w * m[2].z;
    const T a2313 = m[1].z * m[3].w - m[1].w * m[3].z;
    const T a2323 = m[2].z * m[3].w - m[2].w * m[3].z;

    T det = m[0].x * (m[1].y * a2323 - m[1].z * a1323 + m[1].w * a1223) -
            m[0].y * (m[1].x * a2323 - m[1].z * a0323 + m[1].w * a0223) +
            m[0].z * (m[1].x * a1323 - m[1].y * a0323 + m[1].w * a0123) -
            m[0].w * (m[1].x * a1223 - m[1].y * a0223 + m[1].z * a0123);
    det = 1.0 / det;

    using Matrix = Matrix4<T>;
    using ColumnType = typename Matrix::ColumnType;

    return Matrix {
        ColumnType {
            det * (m[1].y * a2323 - m[1].z * a1323 + m[1].w * a1223),  // x
            det * -(m[0].y * a2323 - m[0].z * a1323 + m[0].w * a1223), // y
            det * (m[0].y * a2313 - m[0].z * a1313 + m[0].w * a1213),  // x
            det * -(m[0].y * a2312 - m[0].z * a1312 + m[0].w * a1212), // w
        },
        ColumnType {
            det * -(m[1].x * a2323 - m[1].z * a0323 + m[1].w * a0223), // x
            det * (m[0].x * a2323 - m[0].z * a0323 + m[0].w * a0223),  // y
            det * -(m[0].x * a2313 - m[0].z * a0313 + m[0].w * a0213), // x
            det * (m[0].x * a2312 - m[0].z * a0312 + m[0].w * a0212),  // w
        },
        ColumnType {
            det * (m[1].x * a1323 - m[1].y * a0323 + m[1].w * a0123),  // x
            det * -(m[0].x * a1323 - m[0].y * a0323 + m[0].w * a0123), // y
            det * (m[0].x * a1313 - m[0].y * a0313 + m[0].w * a0113),  // x
            det * -(m[0].x * a1312 - m[0].y * a0312 + m[0].w * a0112), // w
        },
        ColumnType {
            det * -(m[1].x * a1223 - m[1].y * a0223 + m[1].z * a0123), // x
            det * (m[0].x * a1223 - m[0].y * a0223 + m[0].z * a0123),  // y
            det * -(m[0].x * a1213 - m[0].y * a0213 + m[0].z * a0113), // x
            det * (m[0].x * a1212 - m[0].y * a0212 + m[0].z * a0112),  // w
        },
    };
}

template <std::floating_point T>
[[nodiscard]] constexpr T determinant(const Matrix4<T>& m) noexcept
{
    const T a0123 = m[2].x * m[3].y - m[2].y * m[3].x;
    const T a0223 = m[2].x * m[3].z - m[2].z * m[3].x;
    const T a0323 = m[2].x * m[3].w - m[2].w * m[3].x;
    const T a1223 = m[2].y * m[3].z - m[2].z * m[3].y;
    const T a1323 = m[2].y * m[3].w - m[2].w * m[3].y;
    const T a2323 = m[2].z * m[3].w - m[2].w * m[3].z;

    return m[0].x * (m[1].y * a2323 - m[1].z * a1323 + m[1].w * a1223) -
           m[0].y * (m[1].x * a2323 - m[1].z * a0323 + m[1].w * a0223) +
           m[0].z * (m[1].x * a1323 - m[1].y * a0323 + m[1].w * a0123) -
           m[0].w * (m[1].x * a1223 - m[1].y * a0223 + m[1].z * a0123);
}

template <std::floating_point T>
[[nodiscard]] constexpr Matrix4<T> transpose(const Matrix4<T>& m) noexcept
{
    Matrix4<T> result;
    for (usize col = 0; col < Matrix4<T>::SIZE; ++col) {
        for (usize row = 0; row < Matrix4<T>::SIZE; ++row) {
            result[col][row] = m[row][col];
        }
    }
    return result;
}

template <std::floating_point L, concepts::number R>
[[nodiscard]] constexpr Matrix4<L> translate(
    const Matrix4<L>& m, const Vector3<R>& v) noexcept
{
    Matrix4<L> result = m;
    result = m[0] * v[0] + m[1] * v[1] + m[2] * v[2] + m[3];
    return result;
}

template <concepts::number L, std::floating_point R>
[[nodiscard]] constexpr Matrix4<R> scale(const Matrix4<L>& m, const Vector3<R>& s) noexcept
{
    return Matrix4<L> {
        m[0] * s[0],
        m[1] * s[1],
        m[2] * s[2],
        m[3],
    };
}

template <std::floating_point L, std::floating_point R>
[[nodiscard]] constexpr Matrix4<L> operator+(
    const Matrix4<L>& lhs, const Matrix4<R>& rhs) noexcept
{
    return Matrix4<L> {
        lhs[0] + rhs[0],
        lhs[1] + rhs[1],
        lhs[2] + rhs[2],
        lhs[3] + rhs[3],
    };
}

template <std::floating_point L, concepts::number R>
[[nodiscard]] constexpr Matrix4<L> operator+(const Matrix4<L>& lhs, const R s) noexcept
{
    return Matrix4<L> {
        lhs[0] + s,
        lhs[1] + s,
        lhs[2] + s,
        lhs[3] + s,
    };
}

template <concepts::number L, std::floating_point R>
[[nodiscard]] constexpr Matrix4<R> operator+(const L s, const Matrix4<R>& rhs) noexcept
{
    return Matrix4<R> {
        s + rhs[0],
        s + rhs[1],
        s + rhs[2],
        s + rhs[3],
    };
}

template <std::floating_point L, std::floating_point R>
[[nodiscard]] constexpr Matrix4<L> operator-(
    const Matrix4<L>& lhs, const Matrix4<R>& rhs) noexcept
{
    return Matrix4<L> {
        lhs[0] - rhs[0],
        lhs[1] - rhs[1],
        lhs[2] - rhs[2],
        lhs[3] - rhs[3],
    };
}

template <std::floating_point L, concepts::number R>
[[nodiscard]] constexpr Matrix4<L> operator-(const Matrix4<L>& lhs, const R s) noexcept
{
    return Matrix4<L> {
        lhs[0] - s,
        lhs[1] - s,
        lhs[2] - s,
        lhs[3] - s,
    };
}

template <concepts::number L, std::floating_point R>
[[nodiscard]] constexpr Matrix4<R> operator-(const L s, const Matrix4<R>& rhs) noexcept
{
    return Matrix4<R> {
        s - rhs[0],
        s - rhs[1],
        s - rhs[2],
        s - rhs[3],
    };
}

template <std::floating_point L, std::floating_point R>
[[nodiscard]] constexpr Matrix4<L> operator*(
    const Matrix4<L>& lhs, const Matrix4<R>& rhs) noexcept
{
    return Matrix4<L> {
        lhs[0] * rhs[0][0] + lhs[1] * rhs[0][1] + lhs[2] * rhs[0][2] + lhs[3] * rhs[0][3],
        lhs[0] * rhs[1][0] + lhs[1] * rhs[1][1] + lhs[2] * rhs[1][2] + lhs[3] * rhs[1][3],
        lhs[0] * rhs[2][0] + lhs[1] * rhs[2][1] + lhs[2] * rhs[2][2] + lhs[3] * rhs[2][3],
        lhs[0] * rhs[3][0] + lhs[1] * rhs[3][1] + lhs[2] * rhs[3][2] + lhs[3] * rhs[3][3],
    };
}

template <std::floating_point L, concepts::number R>
[[nodiscard]] constexpr Matrix4<L> operator*(const Matrix4<L>& lhs, const R s) noexcept
{
    return Matrix4<L> {
        lhs[0] * s,
        lhs[1] * s,
        lhs[2] * s,
        lhs[3] * s,
    };
}

template <concepts::number L, std::floating_point R>
[[nodiscard]] constexpr Matrix4<R> operator*(const L s, const Matrix4<R>& rhs) noexcept
{
    return Matrix4<R> {
        s * rhs[0],
        s * rhs[1],
        s * rhs[2],
        s * rhs[3],
    };
}

template <std::floating_point T>
[[nodiscard]] constexpr typename Matrix4<T>::ColumnType operator*(
    const Matrix4<T>& lhs, const typename Matrix4<T>::RowType& v) noexcept
{
    return typename Matrix4<T>::ColumnType {
        lhs[0][0] * v[0] + lhs[1][0] * v[1] + lhs[2][0] * v[2] + lhs[3][0] * v[3],
        lhs[0][1] * v[0] + lhs[1][1] * v[1] + lhs[2][1] * v[2] + lhs[3][1] * v[3],
        lhs[0][2] * v[0] + lhs[1][2] * v[1] + lhs[2][2] * v[2] + lhs[3][2] * v[3],
        lhs[0][3] * v[0] + lhs[1][3] * v[1] + lhs[2][3] * v[2] + lhs[3][3] * v[3],
    };
}

template <std::floating_point L, std::floating_point R>
[[nodiscard]] constexpr typename Matrix4<L>::RowType operator*(
    const typename Matrix4<L>::ColumnType& v, const Matrix4<L>& rhs) noexcept
{
    return typename Matrix4<L>::RowType {
        rhs[0][0] * v[0] + rhs[0][1] * v[1] + rhs[0][2] * v[2] + rhs[0][3] * v[3],
        rhs[1][0] * v[0] + rhs[1][1] * v[1] + rhs[1][2] * v[2] + rhs[1][3] * v[3],
        rhs[2][0] * v[0] + rhs[2][1] * v[1] + rhs[2][2] * v[2] + rhs[2][3] * v[3],
        rhs[3][0] * v[0] + rhs[3][1] * v[1] + rhs[3][2] * v[2] + rhs[3][3] * v[3],
    };
}

template <std::floating_point L, std::floating_point R>
[[nodiscard]] constexpr Matrix4<L> operator/(
    const Matrix4<L>& lhs, const Matrix4<R>& rhs) noexcept
{
    return lhs * inverse(rhs);
}

template <std::floating_point L, concepts::number R>
[[nodiscard]] constexpr Matrix4<L> operator/(const Matrix4<L>& lhs, const R s) noexcept
{
    return Matrix4<L> {
        lhs[0] / s,
        lhs[1] / s,
        lhs[2] / s,
        lhs[3] / s,
    };
}

template <concepts::number L, std::floating_point R>
[[nodiscard]] constexpr Matrix4<R> operator/(const L s, const Matrix4<R>& rhs) noexcept
{
    return Matrix4<R> {
        s / rhs[0],
        s / rhs[1],
        s / rhs[2],
        s / rhs[3],
    };
}

template <std::floating_point T>
[[nodiscard]] constexpr typename Matrix4<T>::ColumnType operator/(
    const Matrix4<T>& lhs, const typename Matrix4<T>::RowType& v) noexcept
{
    return inverse(lhs) * v;
}

template <std::floating_point T>
[[nodiscard]] constexpr typename Matrix4<T>::RowType operator/(
    const typename Matrix4<T>::ColumnType& v, const Matrix4<T>& rhs) noexcept
{
    return v * inverse(rhs);
}

} // namespace tundra::math
