#pragma once
#include "renderer/renderer_export.h"
#include "core/core.h"
#include "core/std/traits/is_callable.h"
#include "renderer/frame_graph2/enums.h"
#include "rhi/rhi_context.h"

namespace tundra::rhi {
class IRHIContext;
}

namespace tundra::renderer::frame_graph2 {

class Registry;

///
class RENDERER_API PassNode {
private:
    core::String m_name;
    QueueType m_queue_type;

public:
    PassNode(core::String name, const QueueType queue_type) noexcept;

    PassNode(const PassNode&) = delete;
    PassNode(PassNode&&) = delete;
    PassNode& operator=(const PassNode&) = delete;
    PassNode& operator=(PassNode&&) = delete;
    virtual ~PassNode() = default;

public:
    ///
    virtual void execute(
        rhi::IRHIContext* context,
        const Registry& registry,
        rhi::CommandEncoder& encoder) noexcept = 0;

public:
    [[nodiscard]] const core::String& name() const noexcept { return m_name; }
    [[nodiscard]] QueueType queue_type() const noexcept { return m_queue_type; }
};

namespace concepts {

// clang-format off
template <typename Func, typename Data>
concept pass_execute = core::traits::callable_with<
    Func,
    void,
    rhi::IRHIContext*,
    const Registry&,
    rhi::CommandEncoder&,
    const Data&>;

// clang-format on

} // namespace concepts

///
template <typename Data, concepts::pass_execute<Data> Execute>
class PassNodeLambda final : public PassNode {
private:
    Execute m_execute_lambda;
    core::Option<Data> m_data;

public:
    PassNodeLambda(
        Execute&& execute, //
        core::String name,
        const QueueType queue_type) noexcept
        : PassNode(core::move(name), queue_type)
        , m_execute_lambda(core::move(execute))
    {
    }

public:
    ///
    void execute(
        rhi::IRHIContext* context,
        const Registry& registry,
        rhi::CommandEncoder& encoder) noexcept final
    {
        m_execute_lambda(context, registry, encoder, *m_data);
    }

    void set_data(Data data) noexcept { m_data = data; }
};

} // namespace tundra::renderer::frame_graph2
