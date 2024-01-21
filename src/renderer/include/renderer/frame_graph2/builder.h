#pragma once
#include "renderer/renderer_export.h"
#include "core/core.h"
#include "renderer/frame_graph2/resources/handle.h"

namespace tundra::renderer::frame_graph2 {

class FrameGraph;
class IPassNode;
struct TextureCreateInfo;
struct BufferCreateInfo;

///
class RENDERER_API Builder {
private:
    FrameGraph& m_frame_graph;
    NodeIndex m_pass_node;

public:
    Builder(FrameGraph& fg, const NodeIndex pass_node) noexcept;
    Builder(const Builder&) = delete;
    Builder(Builder&&) = delete;
    Builder& operator=(const Builder&) = delete;
    Builder& operator=(Builder&&) = delete;
    ~Builder() noexcept = default;

public:
    ///
    [[nodiscard]]
    TextureHandle create_texture(
        core::String name, TextureCreateInfo create_info) noexcept;

    ///
    [[nodiscard]]
    BufferHandle create_buffer(core::String name, BufferCreateInfo create_info) noexcept;

public:
    ///
    template <HandleResourceType Type, typename ResourceUsage>
    [[nodiscard]]
    Handle<Type, ResourceUsage> read(
        const Handle<Type, ResourceUsage> handle,
        const ResourceUsage resource_usage) noexcept
    {
        const NodeIndex node_index //
            = this->read_impl(
                handle.node_index(), //
                Type,
                to_resource_usage(resource_usage));
        return Handle<Type, ResourceUsage> { node_index };
    }

    ///
    template <HandleResourceType Type, typename ResourceUsage>
    [[nodiscard]]
    Handle<Type, ResourceUsage> write(
        const Handle<Type, ResourceUsage> handle,
        const ResourceUsage resource_usage) noexcept
    {
        const NodeIndex node_index //
            = this->write_impl(
                handle.node_index(), //
                Type,
                to_resource_usage(resource_usage));
        return Handle<Type, ResourceUsage> { node_index };
    }

public:
    /// Mark the current pass as a leaf to prevent it from being culled.
    void side_effect() noexcept;

private:
    ///
    [[nodiscard]]
    NodeIndex read_impl(
        const NodeIndex node_index,
        const HandleResourceType resource_type,
        const ResourceUsage resource_usage) noexcept;

    ///
    [[nodiscard]]
    NodeIndex write_impl(
        const NodeIndex node_index,
        const HandleResourceType resource_type,
        const ResourceUsage resource_usage) noexcept;
};

} // namespace tundra::renderer::frame_graph2
