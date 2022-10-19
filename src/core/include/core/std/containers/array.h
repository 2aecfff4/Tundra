#pragma once
#include "core/std/hash.h"
#include <vector>

namespace tundra::core {

///
template <typename T>
using Array = std::vector<T>;

template <typename T>
struct Hash<std::vector<T>> {
    [[nodiscard]] usize operator()(const std::vector<T>& key) const noexcept
    {
        return core::hash_range(key.begin(), key.end());
    }
};

///
template <typename T, typename Allocator, typename V>
constexpr typename std::vector<T, Allocator>::size_type erase(
    std::vector<T, Allocator>& container, const V& value)
{
    return std::erase(container, value);
}

///
template <typename T, typename Allocator, typename Pred>
constexpr typename std::vector<T, Allocator>::size_type erase_if(
    std::vector<T, Allocator>& container, Pred pred)
{
    return std::erase_if(container, static_cast<Pred&&>(pred));
}

} // namespace tundra::core
