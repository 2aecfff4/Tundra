#pragma once
#include "renderer/renderer_export.h"
#include "core/core.h"
#include "core/std/option.h"
#include "renderer/frame_graph2/resources/resource_node.h"
#include "rhi/resources/buffer.h"
#include "rhi/resources/handle.h"

namespace tundra::renderer::frame_graph2 {

using BufferUsageFlags = rhi::BufferUsageFlags;
using MemoryType = rhi::MemoryType;

///
struct RENDERER_API BufferCreateInfo {
    BufferUsageFlags usage = BufferUsageFlags::STORAGE_BUFFER;
    MemoryType memory_type = MemoryType::GPU;
    /// `size` **must** be greater than `0`.
    u64 size = 0;
};

///
class RENDERER_API BufferResource : public ResourceNode {
private:
    BufferCreateInfo m_create_info;
    core::Option<rhi::BufferHandle> m_handle;

public:
    BufferResource(
        core::String name,
        const bool is_imported,
        const core::Option<NodeIndex> creator,
        const core::Option<NodeIndex> writer,
        const core::Option<NodeIndex> parent,
        const u16 generation,
        BufferCreateInfo create_info) noexcept;

public:
    void create(rhi::IRHIContext* const context, Registry& registry) noexcept final;
    void destroy(rhi::IRHIContext* const context) noexcept final;
};

template <>
inline constexpr ResourceType resource_type<BufferResource> = ResourceType::Buffer;

} // namespace tundra::renderer::frame_graph2
