#include "renderer/frame_graph2/frame_graph.h"
#include "core/std/containers/hash_map.h"
#include "core/std/option.h"
#include "core/std/unique_ptr.h"
#include "core/std/utils.h"
#include "core/std/variant.h"
#include "renderer/frame_graph2/graph.h"
#include "renderer/frame_graph2/resources/buffer.h"
#include "renderer/frame_graph2/resources/handle.h"
#include "renderer/frame_graph2/resources/node.h"
#include "renderer/frame_graph2/resources/pass_node.h"
#include "renderer/frame_graph2/resources/resource_node.h"
#include "renderer/frame_graph2/resources/texture.h"
#include "renderer/frame_graph2/resources/usage_flags.h"
#include <stack>

namespace tundra::renderer::frame_graph2 {

FrameGraph::FrameGraph(const rhi::IRHIContext* context) noexcept
{
}

void FrameGraph::compile() noexcept
{
    this->cull_nodes();
    const auto sort_result = this->topological_sort();
}

void FrameGraph::cull_nodes() noexcept
{
    const auto nodes = m_graph.nodes();

    // If something referenced the node, add references
    for (auto [node_index, weight] : nodes) {
        auto outgoing_nodes = m_graph.neighbors_directed<Direction::Outgoing>(node_index);

        for ([[maybe_unused]] const auto& _ : outgoing_nodes) {
            weight.add_ref();
        }
    }

    // Find all unreferenced nodes, and push them on the stack.
    std::stack<NodeIndex> stack;
    for (const auto& [node_index, weight] : nodes) {
        if (weight.is_culled()) {
            stack.push(node_index);
        }
    }

    // Iterate through nodes that have not been referenced,
    // and identify all edges pointing towards the unreferenced node.
    // If there are any nodes pointing towards the node currently being processed,
    // decrement the reference count.
    while (!stack.empty()) {
        const NodeIndex node_index = stack.top();
        stack.pop();

        auto incoming_nodes = m_graph.neighbors_directed<Direction::Incoming>(node_index);
        for (const NodeIndex& incoming_node : incoming_nodes) {
            auto weight = m_graph.node_weight(incoming_node);
            weight.value()->sub_ref();
            if (weight.value()->is_culled()) {
                stack.push(incoming_node);
            }
        }
    }
}

core::Option<FrameGraph::TopologicalSortResult> FrameGraph::topological_sort() noexcept
{
    // Kahn's algorithm
    // https://en.wikipedia.org/wiki/Topological_sorting#Kahn's_algorithm

    const auto nodes = m_graph.nodes();
    const usize node_count = nodes.size();

    core::HashMap<NodeIndex, u32> in_degree;
    for (const auto& [node_index, node] : nodes) {
        auto incoming_nodes = m_graph.neighbors_directed<Direction::Incoming>(node_index);

        for ([[maybe_unused]] const auto& _ : incoming_nodes) {
            in_degree[node_index] += 1;
        }
    }

    std::stack<NodeIndex> stack;
    for (const auto& [node_index, node] : nodes) {
        if (in_degree[node_index] == 0) {
            stack.push(node_index);
        }
    }

    core::Array<NodeIndex> topologically_sorted;
    topologically_sorted.reserve(node_count);

    core::HashMap<NodeIndex, u32> node_levels;
    u32 dependency_level_count = 1;

    // Iterate through the stack, removing nodes and updating in-degrees for their neighbors.
    // Continue this process until the stack is empty.
    // If the stack becomes empty and the result contains all nodes, the graph is a DAG.
    while (!stack.empty()) {
        const NodeIndex node_index = stack.top();
        stack.pop();

        // #TODO: Only add nodes that are not culled.
        topologically_sorted.push_back(node_index);

        auto outgoing_nodes = m_graph.neighbors_directed<Direction::Outgoing>(node_index);
        for (const auto& neighbor_index : outgoing_nodes) {

            in_degree[neighbor_index] -= 1;
            if (in_degree[neighbor_index] == 0) {
                stack.push(neighbor_index);

                // The level of the neighbor is one more than the current node
                const u32 level = node_levels[node_index] + 1;
                node_levels[neighbor_index] = level;
                dependency_level_count = math::max(level + 1, dependency_level_count);
            }
        }
    }

    if (topologically_sorted.size() != node_count) {
        // The graph is cyclic
        return std::nullopt;
    }

    core::Array<DependencyLevel> dependency_levels(dependency_level_count);
    for (const NodeIndex node : topologically_sorted) {
        const u32 level = node_levels[node];
        FrameGraph::DependencyLevel& dependency_level = dependency_levels[level];
        dependency_level.level = level;
        dependency_level.node_indices.push_back(node);
    }

    return TopologicalSortResult {
        .topologically_sorted = topologically_sorted,
        .dependency_levels = dependency_levels,
    };
}

void FrameGraph::mark_uncullable(const NodeIndex node_index) noexcept
{
    if (auto weight = m_graph.node_weight(node_index)) {
        weight.value()->mark_uncullable();
    }
}

TextureHandle FrameGraph::create_texture(
    const NodeIndex pass_node, //
    core::String name,
    TextureCreateInfo create_info) noexcept
{
    // auto pass_weight = m_graph.node_weight(pass_node).value();
    auto node = core::make_unique<TextureResource>(
        core::move(name), //
        false,
        pass_node,
        core::nullopt,
        core::nullopt,
        0,
        core::move(create_info));

    auto kind = Node::Kind(core::move(node));
    const NodeIndex node_index = m_graph.add_node(core::move(kind));
    m_resource_nodes.push_back(node_index);

    return TextureHandle { node_index };
}

BufferHandle FrameGraph::create_buffer(
    const NodeIndex pass_node, //
    core::String name,
    BufferCreateInfo create_info) noexcept
{
    auto node = core::make_unique<BufferResource>(
        core::move(name), //
        false,
        pass_node,
        core::nullopt,
        core::nullopt,
        0,
        core::move(create_info));

    auto kind = Node::Kind(core::move(node));
    const NodeIndex node_index = m_graph.add_node(core::move(kind));
    m_resource_nodes.push_back(node_index);
    return BufferHandle { node_index };
}

NodeIndex FrameGraph::read_impl(
    const NodeIndex pass_node,
    const NodeIndex node_index,
    const HandleResourceType resource_type,
    const ResourceUsage resource_usage) noexcept
{
    // #TODO: Check if anyone has written to the resource before.
    [[maybe_unused]] const auto _ //
        = m_graph.add_edge(
            node_index, //
            pass_node,
            Edge(resource_usage));

    return node_index;
}

NodeIndex FrameGraph::write_impl(
    const NodeIndex pass_node,
    const NodeIndex node_index,
    const HandleResourceType resource_type,
    const ResourceUsage resource_usage) noexcept
{
    // #TODO: Check if we have previously written to this resource.
    // Check if we already reading from this resource
    const Node* resource_weight = m_graph.node_weight(node_index).value();
    const auto& resource_node //
        = core::get<core::UniquePtr<ResourceNode>>(resource_weight->kind());

    const NodeIndex resource_ref_index = [&] {
        auto resource_ref = core::make_unique<ResourceNodeRef>(
            node_index,
            resource_node->resource_type(),
            resource_node->name(),
            resource_node->is_imported(),
            pass_node,
            pass_node,
            node_index,
            resource_node->generation() + 1);
        auto kind = Node::Kind(core::move(resource_ref));
        const NodeIndex node_index = m_graph.add_node(Node(core::move(kind)));
        m_resource_nodes.push_back(node_index);
        return node_index;
    }();

    {
        [[maybe_unused]] const auto _ //
            = m_graph.add_edge(node_index, pass_node, Edge(ResourceUsage::NONE));
    }

    {
        [[maybe_unused]] const auto _ //
            = m_graph.add_edge(pass_node, resource_ref_index, Edge(resource_usage));
    }

    return resource_ref_index;
}

core::String FrameGraph::export_graphviz() const noexcept
{
    return m_graph.export_graphviz(
        [&](const Node& node) { //
            auto visitor        //
                = core::make_overload(
                    [&](const core::UniquePtr<PassNode>& pass) { //
                        return fmt::format(
                            R"([label="{}"  style=filled, fillcolor={}])",
                            pass->name(),
                            node.is_culled() ? "darkorange4" : "darkorange");
                    },
                    [&](const core::UniquePtr<ResourceNode>& resource) { //
                        return fmt::format(
                            R"([label="{}"  style=filled, fillcolor={}])",
                            resource->name(),
                            node.is_culled() ? "skyblue4" : "skyblue");
                    });
            return core::visit(visitor, node.kind());
        },
        [&](const NodeIndex from, const NodeIndex to, const Edge& edge) {
            const bool from_is_culled = m_graph.node_weight(from).value()->is_culled();
            const bool to_is_culled = m_graph.node_weight(to).value()->is_culled();

            const char* color = [&] {
                if (edge.is_write()) {
                    return "firebrick";
                } else {
                    return "darkolivegreen";
                }
            }();

            if (from_is_culled && to_is_culled) {
                return fmt::format("[color={}4 style=dashed]", color);
            } else {
                return fmt::format("[color={}2]", color);
            }
        });
}

} // namespace tundra::renderer::frame_graph2
