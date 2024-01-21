#include "renderer/frame_graph2/resources/pass_node.h"
#include "renderer/frame_graph2/resources/node.h"

namespace tundra::renderer::frame_graph2 {

PassNode::PassNode(core::String name, const QueueType queue_type) noexcept
    : m_name(core::move(name))
    , m_queue_type(queue_type)
{
}

} // namespace tundra::renderer::frame_graph2
