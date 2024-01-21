#include "renderer/frame_graph2/resources/buffer.h"

namespace tundra::renderer::frame_graph2 {

BufferResource::BufferResource(
    core::String name,
    const bool is_imported,
    const core::Option<NodeIndex> creator,
    const core::Option<NodeIndex> writer,
    const core::Option<NodeIndex> parent,
    const u16 generation,
    BufferCreateInfo create_info) noexcept
    : ResourceNode(
          ResourceType::Buffer, //
          core::move(name),
          is_imported,
          creator,
          writer,
          parent,
          generation)
    , m_create_info(core::move(create_info))
{
}

void BufferResource::create(rhi::IRHIContext* context, Registry& registry) noexcept
{
}

void BufferResource::destroy(rhi::IRHIContext* context) noexcept
{
}

} // namespace tundra::renderer::frame_graph2
