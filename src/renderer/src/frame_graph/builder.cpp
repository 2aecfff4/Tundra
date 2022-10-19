#include "renderer/frame_graph/builder.h"
#include "renderer/frame_graph/frame_graph.h"

namespace tundra::renderer::frame_graph {

Builder::Builder(FrameGraph& fg, const RenderPassId render_pass) noexcept
    : m_frame_graph(fg)
    , m_render_pass(render_pass)
{
}

TextureHandle Builder::create_texture(
    const core::String& name, const TextureCreateInfo& create_info) noexcept
{
    const TextureHandle handle = m_frame_graph.create_texture(
        m_render_pass, name, create_info);
    FrameGraph::RenderPassResources& resources = m_frame_graph.get_render_pass_resources(
        m_render_pass);
    resources.creates.insert(handle.handle);
    return handle;
}

BufferHandle Builder::create_buffer(
    const core::String& name, const BufferCreateInfo& create_info) noexcept
{
    tndr_assert(create_info.size > 0, "`create_info.size` must be greater than `0`!");

    const BufferHandle handle = m_frame_graph.create_buffer(
        m_render_pass, name, create_info);
    FrameGraph::RenderPassResources& resources = m_frame_graph.get_render_pass_resources(
        m_render_pass);
    resources.creates.insert(handle.handle);
    return handle;
}

void Builder::read_impl(
    const ResourceId resource, const ResourceUsage resource_usage) noexcept
{
    FrameGraph::RenderPassResources& resources = m_frame_graph.get_render_pass_resources(
        m_render_pass);

    if (auto it = resources.reads.find(resource); it != resources.reads.end()) {
        it->second |= resource_usage;
    } else {
        resources.reads.insert({ resource, resource_usage });
    }
}

void Builder::write_impl(
    const ResourceId resource, const ResourceUsage resource_usage) noexcept
{
    FrameGraph::RenderPassResources& resources = m_frame_graph.get_render_pass_resources(
        m_render_pass);

    if (auto it = resources.writes.find(resource); it != resources.writes.end()) {
        it->second |= resource_usage;
    } else {
        resources.writes.insert({ resource, resource_usage });
    }
}

} // namespace tundra::renderer::frame_graph
