#include "renderer/frame_graph/resources/texture.h"
#include "renderer/frame_graph/registry.h"
#include "rhi/rhi_context.h"

namespace tundra::renderer::frame_graph {

TextureResource::TextureResource(
    const RenderPassId creator,
    const TextureHandle fg_handle,
    const core::String& name,
    const TextureCreateInfo& create_info) noexcept
    : m_create_info(create_info)
    , m_fg_handle(fg_handle)
    , m_creator(creator)
    , m_name(name)
{
}

void TextureResource::create(rhi::IRHIContext* context, Registry& registry) noexcept
{
    const rhi::TextureHandle rhi_handle = context->create_texture(rhi::TextureCreateInfo {
        .kind = m_create_info.kind,
        .format = m_create_info.format,
        .usage = m_create_info.usage,
        .tiling = m_create_info.tiling,
        .memory_type = m_create_info.memory_type,
        .name = m_name,
    });

    registry.add_texture(m_fg_handle, rhi_handle);
    m_handle = rhi_handle;
}

void TextureResource::destroy(rhi::IRHIContext* context) noexcept
{
    context->destroy_texture(*m_handle);
    m_handle = std::nullopt;
}

const core::String& TextureResource::get_name() const noexcept
{
    return m_name;
}

ResourceType TextureResource::get_resource_type() const noexcept
{
    return ResourceType::Texture;
}

bool TextureResource::is_transient() const noexcept
{
    return m_creator != NULL_RENDER_PASS_ID;
}

rhi::TextureUsageFlags TextureResource::get_usage_flags() const noexcept
{
    return m_create_info.usage;
}

rhi::TextureFormat TextureResource::get_format() const noexcept
{
    return m_create_info.format;
}

rhi::TextureTiling TextureResource::get_tiling() const noexcept
{
    return m_create_info.tiling;
}

const rhi::TextureKind::Kind& TextureResource::get_kind() const noexcept
{
    return m_create_info.kind;
}

} // namespace tundra::renderer::frame_graph
