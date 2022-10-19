#pragma once
#include "core/std/hash.h"
#include <robin_hood/robin_hood.h>

namespace tundra::core {

///
template <
    typename Key,
    typename Hash = core::Hash<Key>,
    typename KeyEqual = std::equal_to<Key>,
    size_t MaxLoadFactor100 = 80>
using HashSet = robin_hood::unordered_set<Key, Hash, KeyEqual, MaxLoadFactor100>;

///
template <
    bool is_flat,
    size_t load_factor,
    typename Key,
    typename T,
    typename Hash,
    typename KeyEqual,
    typename Pred>
auto erase_if(
    robin_hood::detail::Table<is_flat, load_factor, Key, T, Hash, KeyEqual>& hash_set,
    Pred pred) requires(std::is_same_v<T, void>)
{
    auto first = hash_set.begin();
    const auto last = hash_set.end();
    const auto old_size = hash_set.size();
    while (first != last) {
        if (pred(*first)) {
            first = hash_set.erase(first);
        } else {
            ++first;
        }
    }
    return old_size - hash_set.size();
}

} // namespace tundra::core
