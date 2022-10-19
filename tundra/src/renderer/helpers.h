#pragma once
#include "core/std/assert.h"

namespace tundra::renderer::helpers {

///
template <typename Map>
[[nodiscard]] auto get_pipeline(const char* name, const Map& map) noexcept
{
    const auto it = map.find(name);
    tndr_assert(it != map.end(), "");
    return it->second;
}

} // namespace tundra::renderer::helpers
