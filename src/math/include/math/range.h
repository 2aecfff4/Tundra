#pragma once
#include "math/math_utils.h"

namespace tundra::math {

///
template <concepts::number T>
struct Range {
    T from;
    T to;
};

} // namespace tundra::math
