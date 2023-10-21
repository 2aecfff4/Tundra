#pragma once
#include "renderer/renderer_export.h"
#include "core/core.h"
#include "core/std/assert.h"
#include "core/std/containers/array.h"
#include "core/std/containers/hash_map.h"
#include "core/std/option.h"
#include "core/std/unique_ptr.h"
#include "math/math_utils.h"
#include "renderer/frame_graph2/dependency_graph.h"
#include <stack>

namespace tundra::renderer::frame_graph2 {

///
class FrameGraph {
private:
    ///
    struct DependencyLevel {
        u32 level = 0;
        core::Array<usize> node_indices;
    };

private:
    core::Array<core::UniquePtr<DependencyGraph::Node>> m_pass_nodes;
    core::Array<core::UniquePtr<DependencyGraph::Node>> m_resource_nodes;

    // #TODO: For now, map the node pointer to its index.
    core::HashMap<const DependencyGraph::Node*, usize> m_pass_node_to_index;
    core::HashMap<const DependencyGraph::Node*, usize> m_resource_node_to_index;

    DependencyGraph m_graph;

public:
    ///
    void compile() noexcept
    {
        // Brain dump:
        // This works, but we need custom edges so we can store information about resource reads/writes.
        // Adjacency lists are nice because we don't have to recompute outgoing/incoming connections,
        // unlike edges where we have to filter them.
        // Maybe store them in addition to adjacency lists?
        //
        // Maybe use `std::variant<PassNode, ResourceNode>`, instead of pointers?
        // Especially because of the memory safety, and it would make it a lot easier to port to rust.
        // `std::variant<Write, Read>`
        // ```
        // const NodeHandle<PassNode>     a = graph.add_node<PassNode>();
        // const NodeHandle<ResourceNode> b = graph.add_node<ResourceNode>();
        // const NodeHandle<PassNode>     c = graph.add_node<PassNode>();
        //
        // m_graph.add_edge(a, b, Write {});
        // m_graph.add_edge(b, c, Read {});
        // auto node = graph.get_node(a);
        // const auto& metadata = graph.get_metadata(a);
        // ```
        //
        // `add_edge` could be implemented something like that:
        // ```
        // template <typename From, typename To, typename Access>
        // void add_edge(
        //     const NodeHandle<From> from,
        //     const NodeHandle<To> to,
        //     const Access access
        // ) {
        // }
        // ```

        // ┌─┐
        // │a│
        // └┬┘
        // ┌▽───┐
        // │ b  │
        // └┬──┬┘
        // ┌▽┐┌▽┐
        // │d││c│
        // └┬┘└┬┘
        // ┌▽──▽┐
        // │ e  │
        // └────┘
        const auto a = m_graph.add_node(this->create_pass_node());
        const auto b = m_graph.add_node(this->create_pass_node());
        const auto c = m_graph.add_node(this->create_pass_node());
        const auto d = m_graph.add_node(this->create_pass_node());
        const auto e = m_graph.add_node(this->create_pass_node());

        m_graph.add_edge(a, b);
        m_graph.add_edge(b, c);
        m_graph.add_edge(b, d);
        m_graph.add_edge(d, e);
        m_graph.add_edge(c, e);

        const auto sort_result = this->topological_sort();
        tndr_assert(sort_result.has_value(), "The graph is cyclic!");

        const auto& [topologically_sorted, dependency_levels] = *sort_result;
    }

private:
    ///
    struct TopologicalSortResult {
        core::Array<usize> topologically_sorted;
        core::Array<DependencyLevel> dependency_levels;
    };

    [[nodiscard]]
    auto topological_sort() noexcept -> core::Option<TopologicalSortResult>
    {
        // Kahn's algorithm
        // https://en.wikipedia.org/wiki/Topological_sorting#Kahn's_algorithm

        core::Array<usize> in_degree(m_pass_nodes.size(), 0);
        for (usize i = 0; i < m_pass_nodes.size(); ++i) {
            DependencyGraph::Node* const node = m_pass_nodes[i].get();
            in_degree[i] = m_graph.get_incoming_edges(node->node_id()).size();
        }

        std::stack<usize> stack;
        for (usize i = 0; i < m_pass_nodes.size(); ++i) {
            if (in_degree[i] == 0) {
                stack.push(i);
            }
        }

        core::Array<usize> topologically_sorted;
        core::Array<u32> node_levels(m_pass_nodes.size(), 0);
        u32 dependency_level_count = 1;

        // Iterate through the stack, removing nodes and updating in-degrees for their neighbors.
        // Continue this process until the stack is empty.
        // If the stack becomes empty and the result contains all nodes, the graph is a DAG.
        while (!stack.empty()) {
            const usize node_idx = stack.top();
            stack.pop();

            topologically_sorted.push_back(node_idx);

            const auto node_id = m_pass_nodes[node_idx]->node_id();
            for (const auto neighbor : m_graph.get_outgoing_edges(node_id)) {
                const auto* const node = m_graph.get_node(neighbor);

                const usize neighbor_index = m_pass_node_to_index.at(node);

                in_degree[neighbor_index] -= 1;
                if (in_degree[neighbor_index] == 0) {
                    stack.push(neighbor_index);

                    // The level of the neighbor is one more than the current node
                    const u32 level = node_levels[node_idx] + 1;
                    node_levels[neighbor_index] = level;
                    dependency_level_count = math::max(level + 1, dependency_level_count);
                }
            }
        }

        if (topologically_sorted.size() != m_pass_nodes.size()) {
            // The graph is cyclic
            return std::nullopt;
        }

        core::Array<DependencyLevel> dependency_levels(dependency_level_count);
        for (const usize node : topologically_sorted) {
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

private:
    [[nodiscard]]
    auto create_pass_node() -> DependencyGraph::Node*
    {
        const usize index = m_pass_nodes.size();
        auto node = core::make_unique<DependencyGraph::Node>();
        auto* node_ptr = node.get();
        m_pass_node_to_index.insert({
            node.get(),
            index,
        });

        m_pass_nodes.push_back(core::move(node));

        return node_ptr;
    }
};

} // namespace tundra::renderer::frame_graph2
