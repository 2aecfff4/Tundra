#pragma once
#include "math/math_export.h"
#include "math/math_utils.h"
#include "math/quat.h"
#include "math/vector3.h"

namespace tundra::math {

///
struct MATH_API Transform {
    math::Quat rotation {};
    math::Vec3 position {};
    f32 scale = 1.f;

    [[nodiscard]] constexpr math::Vec3 right() const noexcept
    {
        return rotation * math::Vec3 { 1, 0, 0 };
    }

    [[nodiscard]] constexpr math::Vec3 up() const noexcept
    {
        return rotation * math::Vec3 { 0, 0, 1 };
    }

    [[nodiscard]] constexpr math::Vec3 forward() const noexcept
    {
        return rotation * math::Vec3 { 0, 1, 0 };
    }

    static const Transform IDENTITY;
};

} // namespace tundra::math
