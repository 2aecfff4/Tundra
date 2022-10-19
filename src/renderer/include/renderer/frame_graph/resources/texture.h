#pragma once
#include "renderer/renderer_export.h"
#include "core/core.h"
#include "core/std/option.h"
#include "renderer/frame_graph/render_pass.h"
#include "renderer/frame_graph/resources/base_resource.h"
#include "renderer/frame_graph/resources/handle.h"
#include "rhi/resources/handle.h"
#include "rhi/resources/texture.h"

namespace tundra::renderer::frame_graph {

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
class RENDERER_API TextureResource : public IBaseResource {
private:
    TextureCreateInfo m_create_info;
    TextureHandle m_fg_handle;
    core::Option<rhi::TextureHandle> m_handle;
    RenderPassId m_creator;
    core::String m_name;

public:
    TextureResource(
        const RenderPassId creator,
        const TextureHandle fg_handle,
        const core::String& name,
        const TextureCreateInfo& create_info) noexcept;

public:
    virtual void create(rhi::IRHIContext* context, Registry& registry) noexcept final;
    virtual void destroy(rhi::IRHIContext* context) noexcept final;
    [[nodiscard]] virtual const core::String& get_name() const noexcept final;
    [[nodiscard]] virtual ResourceType get_resource_type() const noexcept final;
    [[nodiscard]] virtual bool is_transient() const noexcept final;

public:
    [[nodiscard]] rhi::TextureUsageFlags get_usage_flags() const noexcept;
    [[nodiscard]] rhi::TextureFormat get_format() const noexcept;
    [[nodiscard]] rhi::TextureTiling get_tiling() const noexcept;
    [[nodiscard]] const rhi::TextureKind::Kind& get_kind() const noexcept;
};

} // namespace tundra::renderer::frame_graph
