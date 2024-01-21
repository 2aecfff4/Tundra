#include "renderer/frame_graph2/builder.h"
#include "renderer/frame_graph2/frame_graph.h"
#include "renderer/frame_graph2/resources/buffer.h"
#include "renderer/frame_graph2/resources/pass_node.h"
#include "renderer/frame_graph2/resources/texture.h"

namespace tundra::renderer::frame_graph2 {

Builder::Builder(FrameGraph& fg, const NodeIndex pass_node) noexcept
    : m_frame_graph(fg)
    , m_pass_node(pass_node)
{
}

TextureHandle Builder::create_texture(
    core::String name, TextureCreateInfo create_info) noexcept
{
    return m_frame_graph.create_texture(
        m_pass_node, //
        core::move(name),
        core::move(create_info));
}

BufferHandle Builder::create_buffer(
    core::String name, BufferCreateInfo create_info) noexcept
{
    return m_frame_graph.create_buffer(
        m_pass_node, //
        core::move(name),
        core::move(create_info));
}

void Builder::side_effect() noexcept
{
    m_frame_graph.mark_uncullable(m_pass_node);
}

NodeIndex Builder::read_impl(
    const NodeIndex node_index,
    const HandleResourceType resource_type,
    const ResourceUsage resource_usage) noexcept
{
    return m_frame_graph.read_impl(
        m_pass_node, //
        node_index,
        resource_type,
        resource_usage);
}

NodeIndex Builder::write_impl(
    const NodeIndex node_index,
    const HandleResourceType resource_type,
    const ResourceUsage resource_usage) noexcept
{
    return m_frame_graph.write_impl(
        m_pass_node, //
        node_index,
        resource_type,
        resource_usage);
}

} // namespace tundra::renderer::frame_graph2
