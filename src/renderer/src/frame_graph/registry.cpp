#include "renderer/frame_graph/registry.h"
#include "core/std/assert.h"

namespace tundra::renderer::frame_graph {

void Registry::add_texture(
    const TextureHandle fg_handle, const rhi::TextureHandle rhi_handle) noexcept
{
    m_textures.insert({ fg_handle, rhi_handle });
}

void Registry::add_buffer(
    const BufferHandle fg_handle, const rhi::BufferHandle rhi_handle) noexcept
{
    m_buffers.insert({ fg_handle, rhi_handle });
}

void Registry::clear() noexcept
{
    m_textures.clear();
    m_buffers.clear();
}

rhi::TextureHandle Registry::get_texture(const TextureHandle fg_handle) const noexcept
{
    const auto it = m_textures.find(fg_handle);
    tndr_assert(it != m_textures.end(), "");
    return it->second;
}

rhi::BufferHandle Registry::get_buffer(const BufferHandle fg_handle) const noexcept
{
    const auto it = m_buffers.find(fg_handle);
    tndr_assert(it != m_buffers.end(), "");
    return it->second;
}

} // namespace tundra::renderer::frame_graph
