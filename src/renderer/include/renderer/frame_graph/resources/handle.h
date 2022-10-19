#pragma once
#include "core/core.h"
#include "core/std/hash.h"
#include "renderer/frame_graph/resources/enums.h"
#include <compare>

namespace tundra::renderer::frame_graph {

///
template <u32 Id>
struct Handle {
    ResourceId handle = NULL_RESOURCE_ID;

    [[nodiscard]] bool is_valid() const noexcept
    {
        return handle != NULL_RESOURCE_ID;
    }

    friend constexpr auto operator<=>(const Handle&, const Handle&) noexcept = default;
};

using TextureHandle = Handle<0>;
using TextureViewHandle = Handle<1>;
using BufferHandle = Handle<2>;

} // namespace tundra::renderer::frame_graph

namespace tundra::core {

template <u32 Id>
struct Hash<renderer::frame_graph::Handle<Id>> {
    [[nodiscard]] usize operator()(
        const renderer::frame_graph::Handle<Id>& key) const noexcept
    {
        return core::Hash<renderer::frame_graph::ResourceId> {}(key.handle);
    }
};

} // namespace tundra::core
