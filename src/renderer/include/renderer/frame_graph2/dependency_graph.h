#pragma once
#include "renderer/renderer_export.h"
#include "core/core.h"
#include "core/std/containers/array.h"
#include "core/std/span.h"
#include "core/typedefs.h"
#include <limits>
#include <span>
#include <stack>

namespace tundra::renderer::frame_graph2 {

///
class DependencyGraph {
public:
    ///
    enum class NodeId : u16 {};

    ///
    static constexpr NodeId INVALID_NODE_ID = NodeId { std::numeric_limits<u16>::max() };

    ///
    class Node {
    private:
        NodeId m_node_id = INVALID_NODE_ID;
        Node* m_parent = nullptr;
        u16 m_ref_count = 0;
        bool m_is_uncullable = false;

    public:
        [[nodiscard]]
        auto ref_count() const noexcept -> u16
        {
            return m_ref_count;
        }

        [[nodiscard]]
        auto node_id() const noexcept -> NodeId
        {
            return m_node_id;
        }

        [[nodiscard]]
        bool is_uncullable() const noexcept
        {
            return m_is_uncullable;
        }

        void mark_as_uncullable() noexcept
        {
            m_is_uncullable = true;
        }

        void set_parent(Node* const parent) noexcept
        {
            m_parent = parent;
        }

    private:
        void add_ref(const u16 count = 1) noexcept
        {
            m_ref_count += count;
        }

        void sub_ref(const u16 count = 1) noexcept
        {
            m_ref_count -= count;
        }

        void set_node_id(const NodeId node_id) noexcept
        {
            m_node_id = node_id;
        }

    private:
        friend class DependencyGraph;
    };

private:
    core::Array<Node*> m_nodes;
    core::Array<core::Array<NodeId>> m_outgoing_adjacency_list;
    core::Array<core::Array<NodeId>> m_incoming_adjacency_list;

public:
    [[nodiscard]]
    auto add_node(Node* const node) noexcept -> NodeId
    {
        const usize id = m_nodes.size();
        const NodeId node_id = NodeId { static_cast<u16>(id) };
        node->set_node_id(node_id);

        m_nodes.push_back(node);

        m_outgoing_adjacency_list.emplace_back();
        m_incoming_adjacency_list.emplace_back();

        return node_id;
    }

    void add_edge(const NodeId from, const NodeId to) noexcept
    {
        // #TODO: Validate node ids

        m_outgoing_adjacency_list[static_cast<usize>(from)].push_back(to);
        m_incoming_adjacency_list[static_cast<usize>(to)].push_back(from);
    }

    void cull_nodes() noexcept
    {
        // If something referenced the node, add references
        for (usize i = 0; i < m_nodes.size(); ++i) {
            const usize neighbors_count = m_outgoing_adjacency_list[i].size();
            m_nodes[i]->add_ref(static_cast<u16>(neighbors_count));
        }

        // Find all unreferenced nodes, and push them on a stack.
        std::stack<usize> stack;
        for (usize i = 0; i < m_nodes.size(); ++i) {
            if ((m_nodes[i]->ref_count() == 0) && !m_nodes[i]->is_uncullable()) {
                stack.push(i);
            }
        }

        // Iterate through nodes that have not been referenced,
        // and identify all edges pointing towards the unreferenced node.
        // If there are any nodes pointing towards the node currently being processed,
        // decrement the reference count.
        while (!stack.empty()) {
            const usize node_idx = stack.top();
            stack.pop();

            for (const NodeId neighbor : m_incoming_adjacency_list[node_idx]) {
                const usize from = static_cast<usize>(neighbor);

                Node* const node = m_nodes[from];
                node->sub_ref();
                if ((node->ref_count() == 0) && !node->is_uncullable()) {
                    stack.push(from);
                }
            }
        }
    }

    [[nodiscard]]
    auto get_incoming_edges(const NodeId node_id) const noexcept
        -> std::span<const NodeId>
    {
        return m_incoming_adjacency_list[static_cast<usize>(node_id)];
    }

    [[nodiscard]]
    auto get_outgoing_edges(const NodeId node_id) const noexcept
        -> std::span<const NodeId>
    {
        return m_outgoing_adjacency_list[static_cast<usize>(node_id)];
    }

    [[nodiscard]]
    auto get_node(const NodeId node_id) -> Node*
    {
        return m_nodes[static_cast<usize>(node_id)];
    }
};

} // namespace tundra::renderer::frame_graph2