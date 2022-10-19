#pragma once
#include "core/std/hash.h"
#include "math/matrix4.h"
#include "math/quat.h"
#include "math/vector2.h"
#include "math/vector3.h"
#include "math/vector4.h"

namespace tundra::core {

template <typename T>
struct Hash<math::Vector2<T>> {
    [[nodiscard]] usize operator()(const math::Vector2<T>& vec) const noexcept
    {
        usize seed = 0;
        core::hash_and_combine(seed, vec.x);
        core::hash_and_combine(seed, vec.y);
        return seed;
    }
};

template <typename T>
struct Hash<math::Vector3<T>> {
    [[nodiscard]] usize operator()(const math::Vector3<T>& vec) const noexcept
    {
        usize seed = 0;
        core::hash_and_combine(seed, vec.x);
        core::hash_and_combine(seed, vec.y);
        core::hash_and_combine(seed, vec.z);
        return seed;
    }
};

template <typename T>
struct Hash<math::Vector4<T>> {
    [[nodiscard]] usize operator()(const math::Vector4<T>& vec) const noexcept
    {
        usize seed = 0;
        core::hash_and_combine(seed, vec.x);
        core::hash_and_combine(seed, vec.y);
        core::hash_and_combine(seed, vec.z);
        core::hash_and_combine(seed, vec.w);
        return seed;
    }
};

template <typename T>
struct Hash<math::Quaternion<T>> {
    [[nodiscard]] usize operator()(const math::Quaternion<T>& q) const noexcept
    {
        usize seed = 0;
        core::hash_and_combine(seed, q.x);
        core::hash_and_combine(seed, q.y);
        core::hash_and_combine(seed, q.z);
        core::hash_and_combine(seed, q.w);
        return seed;
    }
};

template <typename T>
struct Hash<math::Matrix4<T>> {
    [[nodiscard]] usize operator()(const math::Matrix4<T>& m) const noexcept
    {
        usize seed = 0;
        for (const auto& v : m.matrix) {
            core::hash_and_combine(seed, v);
        }
        return seed;
    }
};

} // namespace tundra::core
