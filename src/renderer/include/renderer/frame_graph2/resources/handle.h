#pragma once
#include "renderer/renderer_export.h"
#include "core/core.h"
#include "renderer/frame_graph2/graph.h"
#include "renderer/frame_graph2/resources/usage_flags.h"

namespace tundra::renderer::frame_graph2 {

///
enum class HandleResourceType : u8 {
    Buffer,
    Texture,
    TextureView,
};

///
template <HandleResourceType type, typename ResourceUsage>
class Handle {
private:
    NodeIndex m_node_index;

public:
    explicit constexpr Handle(const NodeIndex node_index) noexcept
        : m_node_index(node_index)
    {
    }

public:
    [[nodiscard]] constexpr NodeIndex node_index() const noexcept { return m_node_index; }
    friend constexpr auto operator<=>(const Handle&, const Handle&) noexcept = default;
};

using TextureHandle = Handle<HandleResourceType::Texture, TextureResourceUsage>;
using TextureViewHandle = Handle<HandleResourceType::TextureView, TextureResourceUsage>;
using BufferHandle = Handle<HandleResourceType::Buffer, BufferResourceUsage>;

} // namespace tundra::renderer::frame_graph2
