#pragma once
#include "renderer/renderer_export.h"
#include "core/core.h"
#include "core/std/containers/string.h"
#include "renderer/frame_graph/render_pass.h"
#include "renderer/frame_graph/resources/buffer.h"
#include "renderer/frame_graph/resources/handle.h"
#include "renderer/frame_graph/resources/texture.h"

namespace tundra::renderer::frame_graph {

class FrameGraph;

///
class RENDERER_API Builder {
private:
    FrameGraph& m_frame_graph;
    RenderPassId m_render_pass;

public:
    Builder(FrameGraph& fg, const RenderPassId render_pass) noexcept;

public:
    [[nodiscard]] TextureHandle create_texture(
        const core::String& name, const TextureCreateInfo& create_info) noexcept;
    [[nodiscard]] BufferHandle create_buffer(
        const core::String& name, const BufferCreateInfo& create_info) noexcept;

public:
    template <u32 Id>
    Handle<Id> read(const Handle<Id> handle, const ResourceUsage resource_usage) noexcept
    {
        this->read_impl(handle.handle, resource_usage);
        return handle;
    }

    template <u32 Id>
    Handle<Id> write(const Handle<Id> handle, const ResourceUsage resource_usage) noexcept
    {
        this->write_impl(handle.handle, resource_usage);
        return handle;
    }

private:
    void read_impl(const ResourceId resource, const ResourceUsage resource_usage) noexcept;
    void write_impl(
        const ResourceId resource, const ResourceUsage resource_usage) noexcept;
};

} // namespace tundra::renderer::frame_graph
