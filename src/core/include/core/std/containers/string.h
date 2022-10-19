#pragma once
#include "core/std/hash.h"
#include <string>

namespace tundra::core {

///
using String = std::string;

///
template <>
struct Hash<String> {
    [[nodiscard]] usize operator()(const String& str) const noexcept
    {
        return core::hash_range(str.begin(), str.end());
    }
};

} // namespace tundra::core
