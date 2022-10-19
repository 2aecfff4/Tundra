#pragma once
#include "math/math_export.h"
#include "math/math_utils.h"
#include "math/quat.h"
#include "math/vector3.h"

namespace tundra::math {

///
struct MATH_API Transform {
    static constexpr math::Vec3 RIGHT = math::Vec3 { 1, 0, 0 };
    static constexpr math::Vec3 UP = math::Vec3 { 0, 1, 0 };
    static constexpr math::Vec3 FORWARD = math::Vec3 { 0, 0, -1 };

    math::Quat rotation {};
    math::Vec3 position {};
    f32 scale = 1.f;

    [[nodiscard]] constexpr math::Vec3 right() const noexcept
    {
        return rotation * RIGHT;
    }

    [[nodiscard]] constexpr math::Vec3 up() const noexcept
    {
        return rotation * UP;
    }

    [[nodiscard]] constexpr math::Vec3 forward() const noexcept
    {
        return rotation * FORWARD;
    }

    static const Transform IDENTITY;
};

} // namespace tundra::math
