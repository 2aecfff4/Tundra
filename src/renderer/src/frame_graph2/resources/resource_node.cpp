#include "renderer/frame_graph2/resources/resource_node.h"
#include "renderer/frame_graph2/resources/node.h"

namespace tundra::renderer::frame_graph2 {

//////////////////////////////////////////////////////////////////////////////////////////
// ResourceNode

ResourceNode::ResourceNode(
    const ResourceType resource_type,
    core::String name,
    const bool is_imported,
    const core::Option<NodeIndex> creator,
    const core::Option<NodeIndex> writer,
    const core::Option<NodeIndex> parent,
    const u16 generation) noexcept
    : m_resource_type(resource_type)
    , m_name(core::move(name))
    , m_is_imported(is_imported)
    , m_creator(creator)
    , m_writer(writer)
    , m_parent(parent)
    , m_generation(generation)
{
}

//////////////////////////////////////////////////////////////////////////////////////////
// ResourceNodeRef

ResourceNodeRef::ResourceNodeRef(
    const NodeIndex ref,
    const ResourceType resource_type,
    core::String name,
    const bool is_imported,
    const core::Option<NodeIndex> creator,
    const core::Option<NodeIndex> writer,
    const core::Option<NodeIndex> parent,
    const u16 generation) noexcept
    : ResourceNode(
          resource_type, //
          core::move(name),
          is_imported,
          creator,
          writer,
          parent,
          generation)
    , m_ref(ref)
{
}

void ResourceNodeRef::create(rhi::IRHIContext* const context, Registry& registry) noexcept
{
}

void ResourceNodeRef::destroy(rhi::IRHIContext* const context) noexcept
{
}

} // namespace tundra::renderer::frame_graph2
