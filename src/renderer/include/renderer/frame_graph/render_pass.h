#pragma once
#include "renderer/renderer_export.h"
#include "core/core.h"
#include "core/profiler.h"
#include "core/std/containers/array.h"
#include "core/std/containers/string.h"
#include "core/std/panic.h"
#include "core/std/traits/is_callable.h"
#include "core/std/unique_ptr.h"
#include "renderer/frame_graph/enums.h"
#include "renderer/frame_graph/registry.h"
#include "renderer/frame_graph/resources/handle.h"
#include "rhi/resources/render_pass.h"

namespace tundra::rhi {
class CommandEncoder;
class IRHIContext;
} // namespace tundra::rhi

namespace tundra::renderer::frame_graph {

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

template <typename Func, typename Data>
concept render_pass_execute = core::traits::callable_with<
    Func,
    void,
    rhi::IRHIContext*,
    const Registry&,
    rhi::CommandEncoder&,
    const Data&,
    const rhi::RenderPass&>;

// clang-format on

} // namespace concepts

///
enum class RenderPassId : u32 {};

///
inline constexpr RenderPassId NULL_RENDER_PASS_ID = RenderPassId { 0xffffffff };

///
enum class PassType : u8 {
    GraphPass,
    RenderPass,
};

/////////////////////////////////////////////////////////////////////////////////////////
// IFrameGraphPass

///
class RENDERER_API IFrameGraphPass {
public:
    IFrameGraphPass() noexcept = default;
    virtual ~IFrameGraphPass() noexcept = default;
    IFrameGraphPass(IFrameGraphPass&&) noexcept = delete;
    IFrameGraphPass& operator=(IFrameGraphPass&&) noexcept = delete;
    IFrameGraphPass(const IFrameGraphPass&) noexcept = delete;
    IFrameGraphPass& operator=(const IFrameGraphPass&) noexcept = delete;

public:
    virtual void execute(
        rhi::IRHIContext* context,
        const Registry& registry,
        rhi::CommandEncoder& encoder) noexcept = 0;
    [[nodiscard]] virtual const core::String& get_name() const noexcept = 0;
    [[nodiscard]] virtual QueueType get_queue_type() const noexcept = 0;
    [[nodiscard]] virtual PassType get_pass_type() const noexcept = 0;
};

/////////////////////////////////////////////////////////////////////////////////////////
// FrameGraphPass

///
template <typename Data, concepts::pass_execute<Data> Execute>
class FrameGraphPass final : public IFrameGraphPass {
private:
    Execute m_execute_lambda;
    Data m_data;
    core::String m_name;
    QueueType m_queue_type;

public:
    FrameGraphPass(
        Execute&& execute,
        Data data,
        const core::String& name,
        const QueueType queue_type) noexcept
        : m_execute_lambda(core::move(execute))
        , m_data(core::move(data))
        , m_name(name)
        , m_queue_type(queue_type)
    {
    }

public:
    [[nodiscard]] Data& get_data() noexcept
    {
        return m_data;
    }

    [[nodiscard]] const Data& get_data() const noexcept
    {
        return m_data;
    }

public:
    virtual void execute(
        rhi::IRHIContext* context,
        const Registry& registry,
        rhi::CommandEncoder& encoder) noexcept final
    {
        TNDR_PROFILER_TRACE("FrameGraphPass::execute");

        m_execute_lambda(context, registry, encoder, m_data);
    }

    [[nodiscard]] virtual const core::String& get_name() const noexcept final
    {
        return m_name;
    }

    [[nodiscard]] virtual QueueType get_queue_type() const noexcept final
    {
        return m_queue_type;
    }

    [[nodiscard]] virtual PassType get_pass_type() const noexcept final
    {
        return PassType::GraphPass;
    }
};

/////////////////////////////////////////////////////////////////////////////////////////
// Frame graph render pass stuff

using AttachmentTexture = core::Variant<TextureHandle, TextureViewHandle>;
using AttachmentOps = rhi::AttachmentOps;
using ClearValue = rhi::ClearValue;
using ClearDepthStencil = rhi::ClearDepthStencil;

///
struct RENDERER_API ColorAttachment {
    /// Load and store operations of the attachment.
    AttachmentOps ops;
    /// The texture that will be used for rendering.
    AttachmentTexture texture;
    /// The texture used to write resolved multisample data at the end of rendering.
    core::Option<AttachmentTexture> resolve_texture;
    /// Defines values used to clear texture when `AttachmentOps::load` is equal to `AttachmentLoadOp::Clear`.
    ClearValue clear_value;

    bool operator==(const ColorAttachment&) const noexcept = default;
    bool operator!=(const ColorAttachment&) const noexcept = default;
};

///
struct RENDERER_API DepthStencilAttachment {
    /// Load and store operations of the attachment.
    AttachmentOps ops;
    /// Load and store operations of the stencil.
    AttachmentOps stencil_ops;
    /// The texture that will be used for rendering.
    AttachmentTexture texture;
    /// The texture used to write resolved multisample data at the end of rendering.
    core::Option<AttachmentTexture> resolve_texture;
    /// Defines values used to clear texture when `AttachmentOps::load` is equal to `AttachmentLoadOp::Clear`.
    ClearDepthStencil clear_value;

    bool operator==(const DepthStencilAttachment&) const noexcept = default;
    bool operator!=(const DepthStencilAttachment&) const noexcept = default;
};

///
struct RENDERER_API RenderPass {
    /// Number of attachments must be less or equal to `MAX_NUM_COLOR_ATTACHMENTS`.
    core::Array<ColorAttachment> color_attachments;
    core::Option<DepthStencilAttachment> depth_stencil_attachment;

    bool operator==(const RenderPass&) const noexcept = default;
    bool operator!=(const RenderPass&) const noexcept = default;
};

/////////////////////////////////////////////////////////////////////////////////////////
// IFrameGraphRenderPass

///
class RENDERER_API IFrameGraphRenderPass : public IFrameGraphPass {
public:
    [[nodiscard]] virtual const RenderPass& get_fg_render_pass() const noexcept = 0;
    [[nodiscard]] virtual rhi::RenderPass& get_render_pass() noexcept = 0;
};

/////////////////////////////////////////////////////////////////////////////////////////
// FrameGraphRenderPass

///
template <typename Data, concepts::render_pass_execute<Data> Execute>
class FrameGraphRenderPass final : public IFrameGraphRenderPass {
private:
    RenderPass m_fg_render_pass;
    mutable rhi::RenderPass m_render_pass;
    Execute m_execute_lambda;
    Data m_data;
    core::String m_name;
    QueueType m_queue_type;

public:
    FrameGraphRenderPass(
        RenderPass&& render_pass,
        Execute&& execute,
        Data data,
        const core::String& name,
        const QueueType queue_type) noexcept
        : m_fg_render_pass(core::move(render_pass))
        , m_execute_lambda(core::move(execute))
        , m_data(core::move(data))
        , m_name(name)
        , m_queue_type(queue_type)
    {
    }

public:
    [[nodiscard]] Data& get_data() noexcept
    {
        return m_data;
    }

    [[nodiscard]] const Data& get_data() const noexcept
    {
        return m_data;
    }

public:
    virtual void execute(
        rhi::IRHIContext* context,
        const Registry& registry,
        rhi::CommandEncoder& encoder) noexcept final
    {
        TNDR_PROFILER_TRACE("FrameGraphRenderPass::execute");

        const auto overload = core::make_overload(
            [&](const TextureHandle& t) -> rhi::AttachmentTexture {
                return rhi::AttachmentTexture { registry.get_texture(t).get_handle() };
            },
            [](const TextureViewHandle&) -> rhi::AttachmentTexture {
                core::unreachable();
            });

        for (usize i = 0; i < m_fg_render_pass.color_attachments.size(); ++i) {
            const ColorAttachment& fg_color_attachment = m_fg_render_pass
                                                             .color_attachments[i];
            rhi::ColorAttachment& rhi_color_attachment = m_render_pass
                                                             .color_attachments[i];

            rhi_color_attachment.texture = core::visit(
                overload, fg_color_attachment.texture);

            tndr_assert(
                fg_color_attachment.resolve_texture.has_value() ==
                    rhi_color_attachment.resolve_texture.has_value(),
                "Check `FrameGraph::build_barriers`");

            if (fg_color_attachment.resolve_texture.has_value()) {
                rhi_color_attachment.resolve_texture->resolve_texture = core::visit(
                    overload, *fg_color_attachment.resolve_texture);
            }
        }

        tndr_assert(
            m_fg_render_pass.depth_stencil_attachment.has_value() ==
                m_render_pass.depth_stencil_attachment.has_value(),
            "Check `FrameGraph::build_barriers`");
        if (m_render_pass.depth_stencil_attachment.has_value()) {
            rhi::DepthStencilAttachment& depth_stencil_attachment =
                *m_render_pass.depth_stencil_attachment;
            depth_stencil_attachment.texture = core::visit(
                overload, m_fg_render_pass.depth_stencil_attachment->texture);

            tndr_assert(
                depth_stencil_attachment.resolve_texture.has_value() ==
                    m_fg_render_pass.depth_stencil_attachment->resolve_texture.has_value(),
                "Check `FrameGraph::build_barriers`");
            if (depth_stencil_attachment.resolve_texture.has_value()) {
                depth_stencil_attachment.resolve_texture->resolve_texture = core::visit(
                    overload,
                    *m_fg_render_pass.depth_stencil_attachment->resolve_texture);
            }
        }
        m_execute_lambda(context, registry, encoder, m_data, m_render_pass);
    }

    [[nodiscard]] virtual const core::String& get_name() const noexcept final
    {
        return m_name;
    }

    [[nodiscard]] virtual QueueType get_queue_type() const noexcept final
    {
        return m_queue_type;
    }

    [[nodiscard]] virtual const RenderPass& get_fg_render_pass() const noexcept override
    {
        return m_fg_render_pass;
    }

    [[nodiscard]] virtual rhi::RenderPass& get_render_pass() noexcept override
    {
        return m_render_pass;
    }

    [[nodiscard]] virtual PassType get_pass_type() const noexcept final
    {
        return PassType::RenderPass;
    }
};

} // namespace tundra::renderer::frame_graph
