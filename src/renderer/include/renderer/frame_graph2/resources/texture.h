#pragma once
#include "renderer/renderer_export.h"
#include "core/core.h"
#include "core/std/option.h"
#include "renderer/frame_graph2/resources/resource_node.h"
#include "rhi/resources/handle.h"
#include "rhi/resources/texture.h"

namespace tundra::renderer::frame_graph2 {

using TextureKind = rhi::TextureKind;
using MemoryType = rhi::MemoryType;
using TextureFormat = rhi::TextureFormat;
using TextureUsageFlags = rhi::TextureUsageFlags;
using TextureTiling = rhi::TextureTiling;
using SampleCount = rhi::SampleCount;

///
struct RENDERER_API TextureCreateInfo {
    TextureKind::Kind kind;
    MemoryType memory_type = MemoryType::GPU;
    TextureFormat format = TextureFormat::R8_G8_B8_A8_UNORM;
    TextureUsageFlags usage = TextureUsageFlags::SRV;
    TextureTiling tiling = TextureTiling::Optimal;
};

///
class RENDERER_API TextureResource : public ResourceNode {
private:
    TextureCreateInfo m_create_info;
    core::Option<rhi::TextureHandle> m_handle;

public:
    TextureResource(
        core::String name,
        const bool is_imported,
        const core::Option<NodeIndex> creator,
        const core::Option<NodeIndex> writer,
        const core::Option<NodeIndex> parent,
        const u16 generation,
        TextureCreateInfo create_info) noexcept;

public:
    void create(rhi::IRHIContext* const context, Registry& registry) noexcept final;
    void destroy(rhi::IRHIContext* const context) noexcept final;

public:
    [[nodiscard]] rhi::TextureUsageFlags usage_flags() const noexcept;
    [[nodiscard]] rhi::TextureFormat format() const noexcept;
    [[nodiscard]] rhi::TextureTiling tiling() const noexcept;
    [[nodiscard]] const rhi::TextureKind::Kind& kind() const noexcept;
};

template <>
inline constexpr ResourceType resource_type<TextureResource> = ResourceType::Texture;

} // namespace tundra::renderer::frame_graph2
