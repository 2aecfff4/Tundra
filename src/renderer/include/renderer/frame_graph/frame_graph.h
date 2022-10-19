#pragma once
#include "renderer/renderer_export.h"
#include "core/core.h"
#include "core/profiler.h"
#include "core/std/containers/array.h"
#include "core/std/containers/hash_map.h"
#include "core/std/containers/hash_set.h"
#include "core/std/containers/string.h"
#include "core/std/option.h"
#include "core/std/traits/is_callable.h"
#include "core/std/tuple.h"
#include "core/std/unique_ptr.h"
#include "renderer/frame_graph/builder.h"
#include "renderer/frame_graph/enums.h"
#include "renderer/frame_graph/present_pass.h"
#include "renderer/frame_graph/registry.h"
#include "renderer/frame_graph/render_pass.h"
#include "renderer/frame_graph/resources/barrier.h"
#include "renderer/frame_graph/resources/base_resource.h"
#include "renderer/frame_graph/resources/enums.h"
#include "rhi/queue.h"

namespace tundra::rhi {
class IRHIContext;
class CommandEncoder;
} // namespace tundra::rhi

namespace tundra::renderer::frame_graph {

namespace concepts {

///
template <typename Func>
concept pass_setup = core::traits::callable<Func, Builder&>;

///
template <typename Func>
concept render_pass_setup = core::traits::callable<Func, Builder&, RenderPass&>;

} // namespace concepts

///
class RENDERER_API FrameGraph {
private:
    ///
    struct RenderPassResources {
        core::HashMap<ResourceId, ResourceUsage> reads;
        core::HashMap<ResourceId, ResourceUsage> writes;
        core::HashSet<ResourceId> creates;
    };

    ///
    template <typename T>
    struct Barrier {
        /// Barrier executed before a render pass.
        T before;
        /// Barrier executed after a render pass.
        T after;
    };

    ///
    struct RenderPassBarriers {
        Barrier<core::Option<GlobalBarrier>> global_barrier;
        Barrier<core::Array<TextureBarrier>> texture_barriers;
        Barrier<core::Array<BufferBarrier>> buffer_barriers;
    };

    ///
    struct DependencyLevel {
        u32 level;
        core::Array<RenderPassId> passes;
    };

    Registry m_registry;
    core::Array<core::UniquePtr<IBaseResource>> m_resources;
    core::Array<core::UniquePtr<IFrameGraphPass>> m_render_passes;
    core::Array<PresentPass> m_present_passes;
    core::Array<RenderPassResources> m_render_passes_resources;
    core::Array<RenderPassBarriers> m_render_passes_barriers;

    core::Array<core::Array<RenderPassId>> m_adjacency_list;
    core::Array<RenderPassId> m_topologically_sorted_passes;
    core::Array<DependencyLevel> m_dependency_levels;

    rhi::QueueFamilyIndices m_queue_indices;

private:
    friend class Builder;

public:
    FrameGraph(const rhi::IRHIContext* context) noexcept;
    FrameGraph(const FrameGraph&) = delete;
    FrameGraph& operator=(const FrameGraph&) = delete;

public:
    /// # Params
    /// - execute - Execute function must capture all variables by value.
    ///
    /// # Example:
    /// ```
    /// fg.add_pass(
    ///     QueueType::Graphics,
    ///     "test_pass",
    ///     [&](Builder& builder) {
    ///         return ExampleData {};
    ///     },
    ///     [=](rhi::IRHIContext*,
    ///         const Registry&,
    ///         rhi::CommandEncoder&,
    ///         const Data&) {
    ///
    ///     });
    /// ```
    template <
        concepts::pass_setup Setup,
        typename Data = std::invoke_result_t<Setup, Builder&>,
        concepts::pass_execute<Data> Execute>
    [[nodiscard]] Data add_pass(
        const QueueType queue_type, const core::String& name, Setup setup, Execute execute)
    {
        TNDR_PROFILER_TRACE("FrameGraph::add_pass");

        m_render_passes_resources.push_back({});
        m_render_passes_barriers.push_back({});
        Builder builder(*this, RenderPassId { static_cast<u32>(m_render_passes.size()) });
        Data data = setup(builder);
        m_render_passes.push_back(core::make_unique<FrameGraphPass<Data, Execute>>(
            core::move(execute), data, name, queue_type));
        return data;
    }

    /// # Example:
    /// struct VisibilityBufferPassData {
    ///     TextureHandle visibility_texture;
    ///     TextureHandle depth_texture;
    /// };
    ///
    /// this->add_render_pass(
    ///     QueueType::Graphics,
    ///     "visibility_buffer",
    ///     [&](Builder& builder, RenderPass& render_pass) {
    ///         // Create textures.
    ///         const auto visibility_texture = builder.create_texture(
    ///             "visibility_pass_visibility_texture",
    ///             TextureCreateInfo {
    ///                 .kind =
    ///                     TextureKind::Texture2D {
    ///                         .width = 1920,
    ///                         .height = 1080,
    ///                         .num_layers = 1,
    ///                         .num_mips = 1,
    ///                         .sample_count = SampleCount::Count1,
    ///                     },
    ///                 .memory_type = MemoryType::GPU,
    ///                 .format = TextureFormat::R32_UINT,
    ///                 .usage = TextureUsageFlags::COLOR_ATTACHMENT | TextureUsageFlags::SRV,
    ///                 .tiling = TextureTiling::Optimal,
    ///             });
    ///
    ///         const auto depth_texture = builder.create_texture(
    ///             "visibility_pass_depth_texture",
    ///             TextureCreateInfo {
    ///                 .kind =
    ///                     TextureKind::Texture2D {
    ///                         .width = 1920,
    ///                         .height = 1080,
    ///                         .num_layers = 1,
    ///                         .num_mips = 1,
    ///                         .sample_count = SampleCount::Count1,
    ///                     },
    ///                 .memory_type = MemoryType::GPU,
    ///                 .format = TextureFormat::D32_FLOAT,
    ///                 .usage = TextureUsageFlags::DEPTH_ATTACHMENT | TextureUsageFlags::SRV,
    ///                 .tiling = TextureTiling::Optimal,
    ///             });
    ///
    ///         // We are gonna write to textures in a render pass.
    ///         builder.write(visibility_texture, ResourceUsage::COLOR_ATTACHMENT);
    ///         builder.write(depth_texture, ResourceUsage::DEPTH_STENCIL_ATTACHMENT);
    ///
    ///         // Prepare a render pass.
    ///         render_pass = RenderPass {
    ///             .color_attachments = {
    ///                 ColorAttachment {
    ///                     .ops = AttachmentOps::INIT,
    ///                     .texture = visibility_texture,
    ///                     .clear_value =
    ///                         math::UVec4 {
    ///                             0xffff'ffff,
    ///                             0,
    ///                             0,
    ///                             0,
    ///                         },
    ///                 },
    ///             },
    ///             .depth_stencil_attachment = DepthStencilAttachment {
    ///                 .ops = AttachmentOps::INIT,
    ///                 .stencil_ops = AttachmentOps::DONT_CARE,
    ///                 .texture = depth_texture,
    ///                 .clear_value =
    ///                     ClearDepthStencil {
    ///                         .depth = 0.0,
    ///                         .stencil = 0,
    ///                     },
    ///             },
    ///         };
    ///
    ///         // Return our struct.
    ///         return VisibilityBufferPassData {
    ///             .visibility_texture = visibility_texture,
    ///             .depth_texture = depth_texture,
    ///         };
    ///     },
    ///     [=](const Registry& registry,
    ///         rhi::CommandEncoder& encoder,
    ///         const VisibilityBufferPassData& data,
    ///         const rhi::RenderPass& render_pass) {
    ///         encoder.begin_render_pass(
    ///             rhi::Rect { .extent = math::UVec2 { 1920, 1080 } }, render_pass);
    ///
    ///         encoder.end_render_pass();
    ///     });
    ///     });
    template <
        concepts::render_pass_setup Setup,
        typename Data = std::invoke_result_t<Setup, Builder&, RenderPass&>,
        concepts::render_pass_execute<Data> Execute>
    [[nodiscard]] Data add_render_pass(
        const QueueType queue_type, const char* name, Setup setup, Execute execute)
    {
        TNDR_PROFILER_TRACE("FrameGraph::add_render_pass");

        m_render_passes_resources.push_back({});
        m_render_passes_barriers.push_back({});
        Builder builder(*this, RenderPassId { static_cast<u32>(m_render_passes.size()) });
        RenderPass render_pass {};
        Data data = setup(builder, render_pass);
        tndr_assert(
            !this->validate_render_pass(render_pass), "Render pass contains duplicates!");
        m_render_passes.push_back(core::make_unique<FrameGraphRenderPass<Data, Execute>>(
            core::move(render_pass), core::move(execute), data, name, queue_type));
        return data;
    }

    /// Texture used in a present pass can't be used after [`FrameGraph::execute`]!
    /// Texture format must be compatible with a swapchain.
    void add_present_pass(
        const rhi::SwapchainHandle swapchain, const TextureHandle texture) noexcept;

public:
    void compile() noexcept;
    void execute(rhi::IRHIContext* context) noexcept;
    void reset() noexcept;

private:
    void build_adjacency_list() noexcept;
    void topological_sort() noexcept;
    void build_dependency_levels() noexcept;
    void build_barriers() noexcept;

private:
    void insert_barrier(
        const ResourceId resource_id,
        const core::Tuple<ResourceUsage, bool>& previous_usage,
        const RenderPassId pass_id,
        const core::Tuple<ResourceUsage, bool>& next_usage,
        const bool discard_contents) noexcept;

    void queue_ownership_transfer(
        const ResourceId resource_id,
        const RenderPassId previous_pass_id,
        const core::Tuple<ResourceUsage, bool>& previous_usage,
        const RenderPassId pass_id,
        const core::Tuple<ResourceUsage, bool>& next_usage,
        const QueueType source_queue,
        const QueueType destination_queue,
        const bool discard_contents) noexcept;

    /// Returns true if a render pass contains duplicates.
    [[nodiscard]] bool validate_render_pass(const RenderPass& render_pass) const noexcept;

private:
    [[nodiscard]] RenderPassResources& get_render_pass_resources(
        const RenderPassId render_pass) noexcept;

    [[nodiscard]] TextureHandle create_texture(
        const RenderPassId creator,
        const core::String& name,
        const TextureCreateInfo& create_info) noexcept;

    [[nodiscard]] BufferHandle create_buffer(
        const RenderPassId creator,
        const core::String& name,
        const BufferCreateInfo& create_info) noexcept;
};

} // namespace tundra::renderer::frame_graph
