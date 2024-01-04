#pragma once
#include "core/core.h"
#include "core/std/hash.h"
#include "renderer/frame_graph/resources/enums.h"
#include <compare>

namespace tundra::renderer::frame_graph {

///
template <ResourceType type, typename ResourceUsage>
struct Handle {
    ResourceId handle = NULL_RESOURCE_ID;

    [[nodiscard]] bool is_valid() const noexcept
    {
        return handle != NULL_RESOURCE_ID;
    }

    friend constexpr auto operator<=>(const Handle&, const Handle&) noexcept = default;
};

using TextureHandle = Handle<ResourceType::Texture, TextureResourceUsage>;
using TextureViewHandle = Handle<ResourceType::TextureView, TextureResourceUsage>;
using BufferHandle = Handle<ResourceType::Buffer, BufferResourceUsage>;

} // namespace tundra::renderer::frame_graph

namespace tundra::core {

template <renderer::frame_graph::ResourceType Id, typename ResourceUsage>
struct Hash<renderer::frame_graph::Handle<Id, ResourceUsage>> {
    [[nodiscard]] usize operator()(
        const renderer::frame_graph::Handle<Id, ResourceUsage>& key) const noexcept
    {
        return core::Hash<renderer::frame_graph::ResourceId> {}(key.handle);
    }
};

} // namespace tundra::core
