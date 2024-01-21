#include "renderer/frame_graph2/resources/edge.h"

namespace tundra::renderer::frame_graph2 {

Edge::Edge(const ResourceUsage resource_usage) noexcept
    : m_resource_usage(resource_usage)
{
}

} // namespace tundra::renderer::frame_graph2
