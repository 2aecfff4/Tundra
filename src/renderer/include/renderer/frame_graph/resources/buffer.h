#pragma once
#include "renderer/renderer_export.h"
#include "core/core.h"
#include "core/std/option.h"
#include "renderer/frame_graph/render_pass.h"
#include "renderer/frame_graph/resources/base_resource.h"
#include "renderer/frame_graph/resources/handle.h"
#include "rhi/resources/buffer.h"
#include "rhi/resources/handle.h"

namespace tundra::renderer::frame_graph {

///
using BufferUsageFlags = rhi::BufferUsageFlags;

///
using MemoryType = rhi::MemoryType;

///
struct RENDERER_API BufferCreateInfo {
    BufferUsageFlags usage = BufferUsageFlags::SRV;
    MemoryType memory_type = MemoryType::GPU;
    /// `size` **must** be greater than `0`.
    u64 size = 0;
};

///
class RENDERER_API BufferResource : public IBaseResource {
private:
    BufferCreateInfo m_create_info;
    BufferHandle m_fg_handle;
    core::Option<rhi::BufferHandle> m_handle;
    RenderPassId m_creator;
    core::String m_name;

public:
    BufferResource(
        const RenderPassId creator,
        const BufferHandle fg_handle,
        const core::String& name,
        const BufferCreateInfo& create_info) noexcept;

public:
    virtual void create(rhi::IRHIContext* context, Registry& registry) noexcept final;
    virtual void destroy(rhi::IRHIContext* context) noexcept final;
    [[nodiscard]] virtual const core::String& get_name() const noexcept final;
    [[nodiscard]] virtual ResourceType get_resource_type() const noexcept final;
    [[nodiscard]] virtual bool is_transient() const noexcept final;
};

} // namespace tundra::renderer::frame_graph
