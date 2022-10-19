#include "renderer/frame_graph/resources/buffer.h"
#include "renderer/frame_graph/registry.h"
#include "renderer/frame_graph/resources/enums.h"
#include "rhi/rhi_context.h"

namespace tundra::renderer::frame_graph {

BufferResource::BufferResource(
    const RenderPassId creator,
    const BufferHandle fg_handle,
    const core::String& name,
    const BufferCreateInfo& create_info) noexcept
    : m_create_info(create_info)
    , m_fg_handle(fg_handle)
    , m_creator(creator)
    , m_name(name)
{
}

void BufferResource::create(rhi::IRHIContext* context, Registry& registry) noexcept
{
    const rhi::BufferHandle rhi_handle = context->create_buffer(rhi::BufferCreateInfo {
        .usage = m_create_info.usage,
        .memory_type = m_create_info.memory_type,
        .size = m_create_info.size,
        .name = m_name,
    });

    registry.add_buffer(m_fg_handle, rhi_handle);
    m_handle = rhi_handle;
}

void BufferResource::destroy(rhi::IRHIContext* context) noexcept
{
    context->destroy_buffer(*m_handle);
    m_handle = std::nullopt;
}

const core::String& BufferResource::get_name() const noexcept
{
    return m_name;
}

ResourceType BufferResource::get_resource_type() const noexcept
{
    return ResourceType::Buffer;
}

bool BufferResource::is_transient() const noexcept
{
    return m_creator != NULL_RENDER_PASS_ID;
}

} // namespace tundra::renderer::frame_graph
