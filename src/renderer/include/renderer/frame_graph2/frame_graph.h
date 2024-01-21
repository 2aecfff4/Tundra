#pragma once
#include "renderer/renderer_export.h"
#include "core/core.h"
#include "core/std/assert.h"
#include "core/std/containers/array.h"
#include "core/std/option.h"
#include "core/std/traits/is_callable.h"
#include "core/std/unique_ptr.h"
#include "core/typedefs.h"
#include "renderer/frame_graph2/builder.h"
#include "renderer/frame_graph2/graph.h"
#include "renderer/frame_graph2/resources/edge.h"
#include "renderer/frame_graph2/resources/node.h"
#include "renderer/frame_graph2/resources/pass_node.h"
#include "renderer/frame_graph2/resources/resource_node.h"

namespace tundra::renderer::frame_graph2 {

class Builder;

namespace concepts {

///
template <typename Func>
concept pass_setup = core::traits::callable<Func, Builder&>;

} // namespace concepts

///
class RENDERER_API FrameGraph {
private:
    ///
    struct DependencyLevel {
        u32 level = 0;
        core::Array<NodeIndex> node_indices;
    };

private:
    Graph<Node, Edge> m_graph;
    core::Array<NodeIndex> m_pass_nodes;
    core::Array<NodeIndex> m_resource_nodes;

private:
    friend class Builder;

public:
    explicit FrameGraph(const rhi::IRHIContext* context) noexcept;
    FrameGraph(FrameGraph&&) = default;
    FrameGraph& operator=(FrameGraph&&) = default;
    FrameGraph(const FrameGraph&) = delete;
    FrameGraph& operator=(const FrameGraph&) = delete;
    ~FrameGraph() noexcept = default;

public:
    ///
    template <
        concepts::pass_setup Setup,
        typename Data = std::invoke_result_t<Setup, Builder&>,
        concepts::pass_execute<Data> Execute>
    [[nodiscard]] Data add_pass(
        const QueueType queue_type, //
        core::String name,
        Setup setup,
        Execute&& execute)
    {
        auto pass_node = core::make_unique<PassNodeLambda<Data, Execute>>(
            core::forward<Execute>(execute), //
            core::move(name),
            queue_type);
        auto* pass_node_ptr = pass_node.get();

        auto kind = Node::Kind(core::move(pass_node));
        const NodeIndex node_index = m_graph.add_node(Node(core::move(kind)));

        Builder builder(*this, node_index);
        Data data = setup(builder);

        pass_node_ptr->set_data(data);

        m_pass_nodes.push_back(node_index);

        return data;
    }

public:
    ///
    void compile() noexcept;

private:
    void cull_nodes() noexcept;

    ///
    struct TopologicalSortResult {
        core::Array<NodeIndex> topologically_sorted;
        core::Array<DependencyLevel> dependency_levels;
    };

    [[nodiscard]] core::Option<TopologicalSortResult> topological_sort() noexcept;

private:
    ///
    void mark_uncullable(const NodeIndex node_index) noexcept;

    ///
    [[nodiscard]]
    TextureHandle create_texture(
        const NodeIndex pass_node,
        core::String name,
        TextureCreateInfo create_info) noexcept;

    ///
    [[nodiscard]]
    BufferHandle create_buffer(
        const NodeIndex pass_node,
        core::String name,
        BufferCreateInfo create_info) noexcept;

    ///
    [[nodiscard]]
    NodeIndex read_impl(
        const NodeIndex pass_node,
        const NodeIndex node_index,
        const HandleResourceType resource_type,
        const ResourceUsage resource_usage) noexcept;

    ///
    [[nodiscard]]
    NodeIndex write_impl(
        const NodeIndex pass_node,
        const NodeIndex node_index,
        const HandleResourceType resource_type,
        const ResourceUsage resource_usage) noexcept;

public:
    /// https://dreampuf.github.io/GraphvizOnline
    [[nodiscard]]
    core::String export_graphviz() const noexcept;
};

} // namespace tundra::renderer::frame_graph2
