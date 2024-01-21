#include "renderer/frame_graph2/resources/texture.h"
#include "renderer/frame_graph2/resources/resource_node.h"

namespace tundra::renderer::frame_graph2 {

TextureResource::TextureResource(
    core::String name,
    const bool is_imported,
    const core::Option<NodeIndex> creator,
    const core::Option<NodeIndex> writer,
    const core::Option<NodeIndex> parent,
    const u16 generation,
    TextureCreateInfo create_info) noexcept
    : ResourceNode(
          ResourceType::Texture, //
          core::move(name),
          is_imported,
          creator,
          writer,
          parent,
          generation)
    , m_create_info(core::move(create_info))
{
}

void TextureResource::create(rhi::IRHIContext* context, Registry& registry) noexcept
{
}

void TextureResource::destroy(rhi::IRHIContext* context) noexcept
{
}

rhi::TextureUsageFlags TextureResource::usage_flags() const noexcept
{
    return m_create_info.usage;
}

rhi::TextureFormat TextureResource::format() const noexcept
{
    return m_create_info.format;
}

rhi::TextureTiling TextureResource::tiling() const noexcept
{
    return m_create_info.tiling;
}

const rhi::TextureKind::Kind& TextureResource::kind() const noexcept
{
    return m_create_info.kind;
}

} // namespace tundra::renderer::frame_graph2
