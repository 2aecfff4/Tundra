#include "math/transform.h"

namespace tundra::math {

const Transform Transform::IDENTITY = Transform {
    .rotation = math::Quat {},
    .position = math::Vec3 { 0, 0, 0 },
    .scale = 1.f,
};

} // namespace tundra::math
