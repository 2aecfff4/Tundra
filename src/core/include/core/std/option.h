#pragma once
#include "core/std/hash.h"
#include <optional>

namespace tundra::core {

///
template <typename T>
using Option = std::optional<T>;

using std::nullopt;

///
template <typename T>
struct Hash<Option<T>> {
    [[nodiscard]] usize operator()(const Option<T>& o) const noexcept
    {
        if (o.has_value()) {
            return Hash<T> {}(*o);
        }
        return 0;
    }
};

} // namespace tundra::core
