#pragma once
#include "renderer/renderer_export.h"
#include "core/core.h"
#include "core/std/containers/array.h"
#include "core/std/containers/string.h"
#include "core/std/option.h"
#include "core/std/panic.h"
#include "core/std/traits/is_callable.h"
#include "core/std/utils.h"
#include <concepts>
#include <ranges>
#include <sstream>

namespace tundra::renderer::frame_graph2 {

///
struct NodeIndex {
    u16 index;

    explicit operator usize() const noexcept { return index; }
    constexpr auto operator<=>(const NodeIndex&) const noexcept = default;
};

///
struct EdgeIndex {
    u16 index;
    operator usize() const noexcept { return index; }
    constexpr auto operator<=>(const EdgeIndex&) const noexcept = default;
};

/// Edge direction.
enum class Direction : u8 {
    Outgoing,
    Incoming,
};

///
template <typename T, typename E>
class Graph {
private:
    struct Node {
        T weight;
    };

    struct Edge {
        E weight;
        NodeIndex from;
        NodeIndex to;
    };

private:
    core::Array<Node> m_nodes;
    core::Array<Edge> m_edges;

public:
    ///
    template <typename W>
    [[nodiscard]]
    NodeIndex add_node(W&& weight) noexcept
    requires(std::constructible_from<T, W>)
    {
        const usize index = m_nodes.size();
        m_nodes.emplace_back(core::forward<W>(weight));
        return NodeIndex { .index = static_cast<u16>(index) };
    }

    ///
    [[nodiscard]]
    core::Option<T*> node_weight(const NodeIndex node) noexcept
    {
        if (node.index < m_nodes.size()) {
            return &m_nodes[node.index].weight;
        }

        return core::nullopt;
    }

    ///
    [[nodiscard]]
    core::Option<const T*> node_weight(const NodeIndex node) const noexcept
    {
        if (node.index < m_nodes.size()) {
            return &m_nodes[node.index].weight;
        }

        return core::nullopt;
    }

    ///
    [[nodiscard]]
    auto nodes() noexcept
    {
        return m_nodes                 //
               | std::views::enumerate //
               | std::views::transform([](const auto& tuple) {
                     const auto& [index, node] = tuple;
                     struct Voldemort {
                         NodeIndex index;
                         T& weight;
                     };

                     return Voldemort {
                         .index = NodeIndex { .index = static_cast<u16>(index) },
                         .weight = node.weight,
                     };
                 });
    }

public:
    ///
    template <typename W>
    [[nodiscard]]
    EdgeIndex add_edge(
        const NodeIndex from, //
        const NodeIndex to,
        W&& weight) noexcept
    requires(std::constructible_from<E, W>)
    {
        const usize index = m_edges.size();
        m_edges.emplace_back(core::forward<W>(weight), from, to);
        return EdgeIndex { .index = static_cast<u16>(index) };
    }

    ///
    [[nodiscard]]
    core::Option<E*> edge_weight(const EdgeIndex edge) noexcept
    {
        if (edge.index < m_edges.size()) {
            return &m_edges[edge.index].weight;
        }

        return core::nullopt;
    }

    ///
    [[nodiscard]]
    core::Option<const E*> edge_weight(const EdgeIndex edge) const noexcept
    {
        if (edge.index < m_edges.size()) {
            return &m_edges[edge.index].weight;
        }

        return core::nullopt;
    }

public:
    ///
    template <Direction direction>
    [[nodiscard]]
    auto neighbors_directed(const NodeIndex node) const noexcept
    {
        const auto filter = [=](const Edge& edge) {
            if constexpr (direction == Direction::Outgoing) {
                return edge.from == node;

            } else if constexpr (direction == Direction::Incoming) {
                return edge.to == node;
            } else {
                core::unreachable();
            }
        };

        const auto transform = [=](const Edge& edge) {
            if constexpr (direction == Direction::Outgoing) {
                return edge.to;
            } else if constexpr (direction == Direction::Incoming) {
                return edge.from;
            } else {
                core::unreachable();
            }
        };

        return m_edges | std::views::filter(filter) | std::views::transform(transform);
    }

private: ///
    template <Direction direction>
    [[nodiscard]]
    auto neighbors_directed_raw(const NodeIndex node) const noexcept
    {
        const auto filter = [=](const Edge& edge) {
            if constexpr (direction == Direction::Outgoing) {
                return edge.from == node;

            } else if constexpr (direction == Direction::Incoming) {
                return edge.to == node;
            } else {
                core::unreachable();
            }
        };

        return m_edges | std::views::filter(filter);
    }

public:
    template <
        core::traits::callable_with<core::String, const T&> NodeAttributesFn,
        core::traits::callable_with<core::String, NodeIndex, NodeIndex, const E&>
            EdgeAttributesFn>
    [[nodiscard]] core::String export_graphviz(
        NodeAttributesFn node_attributes, //
        EdgeAttributesFn edge_attributes) const noexcept
    {
        std::stringstream out;
        const char* graph_name = "graph";
        out << "digraph \"" << graph_name << "\" {\n";
        out << "rankdir = LR\n";
        out << "bgcolor = black\n";
        out << "graph [fontname = \"helvetica\"];\n";
        out << "edge [fontname = \"helvetica\", fontcolor=white, fontsize=8];\n";
        out << "node [shape=rectangle, fontname=\"helvetica\", fontsize=10];\n\n";

        for (const auto& [index, node] : m_nodes | std::views::enumerate) {
            const auto node_data = node_attributes(node.weight);
            out << fmt::format("\"N{}\" {} \n", index, node_data);
        }

        out << '\n';

        for (const auto& [index, node] : m_nodes | std::views::enumerate) {
            const auto node_index = NodeIndex { .index = static_cast<u16>(index) };
            auto outgoing_edges //
                = this->neighbors_directed_raw<Direction::Outgoing>(node_index);

            for (const Edge& edge : outgoing_edges) {
                const auto edge_data = edge_attributes(edge.from, edge.to, edge.weight);

                out << fmt::format(
                    "N{} -> N{} {};\n", //
                    edge.from.index,
                    edge.to.index,
                    edge_data);
            }
        }

        out << "}";

        return out.str();
    }
};

} // namespace tundra::renderer::frame_graph2

namespace tundra::core {

template <>
struct Hash<renderer::frame_graph2::NodeIndex> {
    [[nodiscard]] usize operator()(
        const renderer::frame_graph2::NodeIndex& key) const noexcept
    {
        return Hash<usize> {}(key.index);
    }
};

} // namespace tundra::core