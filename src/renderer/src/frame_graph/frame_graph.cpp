#include "renderer/frame_graph/frame_graph.h"
#include "core/profiler.h"
#include "core/std/assert.h"
#include "core/std/containers/hash_set.h"
#include "core/std/tuple.h"
#include "renderer/frame_graph/resources/buffer.h"
#include "renderer/frame_graph/resources/texture.h"
#include "rhi/commands/command_encoder.h"
#include "rhi/rhi_context.h"
#include <algorithm>
#include <stack>

namespace tundra::renderer::frame_graph {

[[nodiscard]] static u32 map_queue_to_family_index(
    const rhi::QueueFamilyIndices& queues, const QueueType queue) noexcept
{
    switch (queue) {
        case QueueType::Graphics:
            return queues.graphics_queue;
        case QueueType::AsyncCompute:
            return queues.compute_queue;
        case QueueType::Transfer:
            return queues.transfer_queue;
        case QueueType::Present:
            return queues.present_queue;
        default:
            core::unreachable();
    }
}

[[nodiscard]] static ResourceId attachment_texture_to_resource_id(
    const AttachmentTexture& texture)
{
    return core::visit([](const auto& t) { return t.handle; }, texture);
}

[[nodiscard]] static core::Option<rhi::QueueType> map_fg_queue_to_rhi_queue(
    const core::Option<QueueType> queue) noexcept
{
    if (queue.has_value()) {
        switch (*queue) {
            case QueueType::Graphics:
                return rhi::QueueType::Graphics;
            case QueueType::AsyncCompute:
                return rhi::QueueType::Compute;
            case QueueType::Transfer:
                return rhi::QueueType::Transfer;
            case QueueType::Present:
                return rhi::QueueType::Present;
            default:
                break;
        }
    }

    return std::nullopt;
}

[[nodiscard]] static rhi::SynchronizationStage map_queue_to_synchronization_stage(
    const QueueType queue) noexcept
{
    switch (queue) {
        case QueueType::Graphics:
            return rhi::SynchronizationStage::ALL_GRAPHICS |
                   rhi::SynchronizationStage::COMPUTE_SHADER;
        case QueueType::AsyncCompute:
            return rhi::SynchronizationStage::COMPUTE_SHADER;
        case QueueType::Transfer:
            return rhi::SynchronizationStage::TRANSFER;
        case QueueType::Present:
            return rhi::SynchronizationStage::BOTTOM_OF_PIPE;
        default:
            core::unreachable();
    }
}

void translate_barriers(
    const Registry& registry,
    rhi::CommandEncoder& encoder,
    const core::Option<GlobalBarrier>& global_barrier,
    const core::Array<TextureBarrier>& texture_barriers,
    const core::Array<BufferBarrier>& buffer_barriers) noexcept
{
    TNDR_PROFILER_TRACE("translate_barriers");

    // Global barrier.
    if (global_barrier.has_value()) {
        encoder.global_barrier(rhi::GlobalBarrier {
            .previous_access = global_barrier->previous_access,
            .next_access = global_barrier->next_access,
        });
    }

    // Texture barriers.
    if (!texture_barriers.empty()) {
        core::Array<rhi::TextureBarrier> rhi_barriers;
        rhi_barriers.reserve(texture_barriers.size());

        for (const TextureBarrier& barrier : texture_barriers) {
            rhi_barriers.push_back(rhi::TextureBarrier {
                .texture = registry.get_texture(TextureHandle { barrier.texture }),
                .previous_access = barrier.previous_access,
                .next_access = barrier.next_access,
                .source_queue = map_fg_queue_to_rhi_queue(barrier.source_queue),
                .destination_queue = map_fg_queue_to_rhi_queue(barrier.destination_queue),
                .discard_contents = barrier.discard_contents,
            });
        }

        encoder.texture_barrier(core::move(rhi_barriers));
    }

    // Buffer barriers.
    if (!buffer_barriers.empty()) {
        core::Array<rhi::BufferBarrier> rhi_barriers;
        rhi_barriers.reserve(buffer_barriers.size());

        for (const BufferBarrier& barrier : buffer_barriers) {
            rhi_barriers.push_back(rhi::BufferBarrier {
                .buffer = registry.get_buffer(BufferHandle { barrier.buffer }),
                .previous_access = barrier.previous_access,
                .next_access = barrier.next_access,
                .source_queue = map_fg_queue_to_rhi_queue(barrier.source_queue),
                .destination_queue = map_fg_queue_to_rhi_queue(barrier.destination_queue),
            });
        }

        encoder.buffer_barrier(core::move(rhi_barriers));
    }
}

/////////////////////////////////////////////////////////////////////////////////////////
// FrameGraph

FrameGraph::FrameGraph(const rhi::IRHIContext* context) noexcept
    : m_queue_indices(context->get_queue_family_indices())
{
}

void FrameGraph::add_present_pass(
    const rhi::SwapchainHandle swapchain, const TextureHandle texture) noexcept
{
    TNDR_PROFILER_TRACE("FrameGraph::add_present_pass");
    tndr_assert(texture.is_valid(), "`texture` must be a valid handle");

    const TextureResource* texture_resource = static_cast<TextureResource*>(
        m_resources[static_cast<usize>(texture.handle)].get());

    const rhi::TextureUsageFlags texture_usage_flags = texture_resource->get_usage_flags();
    tndr_assert(
        contains(texture_usage_flags, rhi::TextureUsageFlags::PRESENT),
        "Only texture with `TextureUsageFlags::PRESENT` bit set can be used in a present "
        "pass.");

    const rhi::TextureFormat texture_format = texture_resource->get_format();
    tndr_assert(
        rhi::is_valid_src_present(texture_format),
        "Texture with this format cannot be used in a present pass.");

    const rhi::TextureTiling texture_tiling = texture_resource->get_tiling();
    tndr_assert(
        texture_tiling == rhi::TextureTiling::Optimal,
        "Only texture with tiling set to `TextureTiling::Optimal` can be used in a "
        "present pass.");

    tndr_assert(
        core::holds_alternative<rhi::TextureKind::Texture2D>(
            texture_resource->get_kind()),
        "Only 2D texture can be used in a present pass.");

    tndr_assert(
        rhi::TextureKind::get_sample_count(texture_resource->get_kind()) ==
            rhi::SampleCount::Count1,
        "Only texture with `sample_count` set to `SampleCount::Count1` can be used in a "
        "present pass.");

    m_present_passes.push_back(PresentPass {
        .swapchain = swapchain,
        .texture = texture,
    });
}

void FrameGraph::compile() noexcept
{
    TNDR_PROFILER_TRACE("FrameGraph::compile");

    this->build_adjacency_list();
    this->topological_sort();
    this->build_dependency_levels();
    this->build_barriers();
}

void FrameGraph::execute(rhi::IRHIContext* context) noexcept
{
    TNDR_PROFILER_TRACE("FrameGraph::execute");

    // #TODO: Not optimal...
    if (!m_dependency_levels.empty()) {
        core::Array<rhi::SubmitInfo> submit_infos;

        for (const FrameGraph::DependencyLevel& dependency_level : m_dependency_levels) {
            for (const RenderPassId pass_id : dependency_level.passes) {
                const core::UniquePtr<IFrameGraphPass>& pass =
                    m_render_passes[static_cast<usize>(pass_id)];
                const FrameGraph::RenderPassResources& pass_resources =
                    m_render_passes_resources[static_cast<usize>(pass_id)];
                const FrameGraph::RenderPassBarriers& pass_barriers =
                    m_render_passes_barriers[static_cast<usize>(pass_id)];

                for (const ResourceId resource_id : pass_resources.creates) {
                    core::UniquePtr<IBaseResource>& resource =
                        m_resources[static_cast<usize>(resource_id)];
                    resource->create(context, m_registry);
                }

                rhi::CommandEncoder encoder;
                encoder.begin_command_buffer();
                encoder.begin_region(pass->get_name(), math::Vec4 { 1.f, 1.f, 1.f, 1.f });

                translate_barriers(
                    m_registry,
                    encoder,
                    pass_barriers.global_barrier.before,
                    pass_barriers.texture_barriers.before,
                    pass_barriers.buffer_barriers.before);

                pass->execute(context, m_registry, encoder);

                translate_barriers(
                    m_registry,
                    encoder,
                    pass_barriers.global_barrier.after,
                    pass_barriers.texture_barriers.after,
                    pass_barriers.buffer_barriers.after);

                encoder.end_region();
                encoder.end_command_buffer();

                if (!submit_infos.empty()) {
                    rhi::SubmitInfo& submit_info = submit_infos.back();

                    if (submit_info.queue_type ==
                        map_fg_queue_to_rhi_queue(pass->get_queue_type())) {
                        submit_info.encoders.push_back(core::move(encoder));
                        continue;
                    }
                }

                rhi::SubmitInfo submit_info;
                submit_info.encoders.push_back(core::move(encoder));
                submit_info.synchronization_stage = map_queue_to_synchronization_stage(
                    pass->get_queue_type());
                submit_info.queue_type = *map_fg_queue_to_rhi_queue(
                    pass->get_queue_type());

                submit_infos.push_back(core::move(submit_info));
            }
        }

        core::Array<rhi::PresentInfo> present_infos;
        present_infos.reserve(m_present_passes.size());
        if (!m_present_passes.empty()) {
            rhi::CommandEncoder encoder;
            encoder.begin_command_buffer();
            encoder.begin_region(
                "Prepare textures to present", math::Vec4 { 1.f, 0.5f, 1.f, 1.f });

            for (const PresentPass& present_pass : m_present_passes) {
                if (present_pass.barrier.has_value()) {
                    translate_barriers(
                        m_registry, encoder, std::nullopt, { *present_pass.barrier }, {});
                }

                present_infos.push_back(rhi::PresentInfo {
                    .swapchain = present_pass.swapchain,
                    .texture = m_registry.get_texture(present_pass.texture),
                    .texture_previous_access = PresentPass::ACCESS_FLAGS,
                });
            }

            encoder.end_region();
            encoder.end_command_buffer();

            rhi::SubmitInfo submit_info;
            submit_info.encoders.push_back(core::move(encoder));
            submit_info.synchronization_stage = map_queue_to_synchronization_stage(
                QueueType::Present);
            submit_info.queue_type = *map_fg_queue_to_rhi_queue(QueueType::Present);

            submit_infos.push_back(core::move(submit_info));
        }

        context->submit(core::move(submit_infos), core::move(present_infos));

        for (core::UniquePtr<IBaseResource>& resource : m_resources) {
            resource->destroy(context);
        }
    }
}

void FrameGraph::reset() noexcept
{
    TNDR_PROFILER_TRACE("FrameGraph::reset");

    m_registry.clear();
    m_resources.clear();
    m_render_passes.clear();
    m_present_passes.clear();
    m_render_passes_resources.clear();
    m_render_passes_barriers.clear();
    m_adjacency_list.clear();
    m_topologically_sorted_passes.clear();
    m_dependency_levels.clear();
}

void FrameGraph::build_adjacency_list() noexcept
{
    m_adjacency_list.resize(m_render_passes.size());

    for (usize pass_index = 0; pass_index < m_render_passes.size(); ++pass_index) {
        core::Array<RenderPassId>& adjacent_node_indices = m_adjacency_list[pass_index];

        for (usize other_pass_index = 0; other_pass_index < m_render_passes.size();
             ++other_pass_index) {
            // We don't want to check dependencies to itself.
            if (pass_index == other_pass_index) {
                continue;
            }

            for (const auto& [read, _] :
                 m_render_passes_resources[other_pass_index].reads) {
                const core::HashMap<ResourceId, ResourceUsage>& pass_write_resources =
                    m_render_passes_resources[pass_index].writes;
                const bool depends = pass_write_resources.contains(read);

                if (depends) {
                    adjacent_node_indices.push_back(
                        RenderPassId { static_cast<u32>(other_pass_index) });
                }
            }
        }
    }
}

void FrameGraph::topological_sort() noexcept
{
    const usize num_render_passes = m_render_passes.size();
    std::stack<u32> stack;
    core::Array<bool> visited(num_render_passes, false);
    core::Array<bool> on_stack(num_render_passes, false);

    for (usize i = 0; i < num_render_passes; ++i) {
        if (visited[i]) {
            continue;
        }

        stack.push(static_cast<u32>(i));

        while (!stack.empty()) {
            const u32 s = stack.top();

            if (!visited[s]) {
                visited[s] = true;
                on_stack[s] = true;
                m_topologically_sorted_passes.push_back(RenderPassId { s });
            } else {
                on_stack[s] = false;
                stack.pop();
            }

            for (const RenderPassId v : m_adjacency_list[s]) {
                const u32 v_idx = static_cast<u32>(v);
                if (!visited[v_idx]) {
                    stack.push(v_idx);
                } else if (on_stack[v_idx]) {
                    core::panic("Graph is cyclic!");
                }
            }
        }
    }
}

void FrameGraph::build_dependency_levels() noexcept
{
    const usize num_render_passes = m_render_passes.size();
    core::Array<u32> distances(num_render_passes, 0);
    u32 dependency_level_count = 1;

    for (usize i = 0; i < num_render_passes; ++i) {
        for (const RenderPassId adjacent_index : m_adjacency_list[i]) {
            if (distances[static_cast<u32>(adjacent_index)] < (distances[i] + 1)) {
                const u32 distance = distances[i] + 1;

                distances[static_cast<u32>(adjacent_index)] = distance;
                dependency_level_count = math::max(distance + 1, dependency_level_count);
            }
        }
    }

    m_dependency_levels.resize(dependency_level_count);

    for (const RenderPassId pass : m_topologically_sorted_passes) {
        const u32 level = distances[static_cast<u32>(pass)];
        FrameGraph::DependencyLevel& dependency_level = m_dependency_levels[level];
        dependency_level.level = level;
        dependency_level.passes.push_back(pass);
    }
}

void FrameGraph::build_barriers() noexcept
{
    struct LastResourceUsage {
        RenderPassId render_pass;
        QueueType queue;
        ResourceUsage usage;
        bool is_written;
    };

    core::HashMap<ResourceId, LastResourceUsage> last_resources_usage;

    const auto get_access_flags = [&](const ResourceId resource) {
        const auto it = last_resources_usage.find(resource);
        tndr_assert(it != last_resources_usage.end(), "");

        const LastResourceUsage& last_resource_use = it->second;

        return map_resource_usage(last_resource_use.usage, last_resource_use.is_written);
    };

    for (const FrameGraph::DependencyLevel& dependency_level : m_dependency_levels) {
        for (const RenderPassId pass_id : dependency_level.passes) {
            const usize pass_id_index = static_cast<usize>(pass_id);

            core::UniquePtr<IFrameGraphPass>& pass = m_render_passes[pass_id_index];
            const QueueType pass_queue = pass->get_queue_type();
            const PassType pass_type = pass->get_pass_type();
            const auto& pass_resource_writes = m_render_passes_resources[pass_id_index]
                                                   .writes;
            const auto& pass_resource_reads = m_render_passes_resources[pass_id_index]
                                                  .reads;

            const core::HashSet<ResourceId> resources_used = [&] {
                core::HashSet<ResourceId> resources_used;

                for (const auto& [key, _] : pass_resource_writes) {
                    resources_used.insert(key);
                }

                for (const auto& [key, _] : pass_resource_reads) {
                    resources_used.insert(key);
                }

                return resources_used;
            }();

            for (const ResourceId resource_id : resources_used) {
                const bool is_written = pass_resource_writes.contains(resource_id);
                const bool is_read = pass_resource_reads.contains(resource_id);

                const ResourceUsage all_resource_usage = [&] {
                    ResourceUsage all_resource_usage {};

                    if (const auto it = pass_resource_reads.find(resource_id);
                        it != pass_resource_reads.end()) {
                        all_resource_usage |= it->second;
                    }

                    if (const auto it = pass_resource_writes.find(resource_id);
                        it != pass_resource_writes.end()) {
                        all_resource_usage |= it->second;
                    }

                    return all_resource_usage;
                }();

                if (contains(all_resource_usage, ResourceUsage::COLOR_ATTACHMENT)) {
                    tndr_assert(
                        !intersects(
                            all_resource_usage,
                            ResourceUsage::ALL & ~(ResourceUsage::COLOR_ATTACHMENT |
                                                   ResourceUsage::SHADER_GRAPHICS |
                                                   ResourceUsage::SHADER_COMPUTE |
                                                   ResourceUsage::TRANSFER)),
                        "`COLOR_ATTACHMENT` can be only used with `COLOR_ATTACHMENT`, "
                        "`SHADER_GRAPHICS`, `SHADER_COMPUTE` and `TRANSFER`.");
                }

                if (contains(
                        all_resource_usage, ResourceUsage::DEPTH_STENCIL_ATTACHMENT)) {
                    tndr_assert(
                        !intersects(
                            all_resource_usage,
                            ResourceUsage::ALL &
                                ~(ResourceUsage::DEPTH_STENCIL_ATTACHMENT |
                                  ResourceUsage::SHADER_GRAPHICS |
                                  ResourceUsage::SHADER_COMPUTE |
                                  ResourceUsage::TRANSFER)),
                        "`DEPTH_STENCIL_ATTACHMENT` can be only used with "
                        "`DEPTH_STENCIL_ATTACHMENT`, `SHADER_GRAPHICS`, `SHADER_COMPUTE` "
                        "and `TRANSFER`.");
                }

                // For buffers (at least for now, maybe #TODO?) we can insert a global barrier,
                // but only when queues are the same.
                // When queues are different, then we have to use a buffer barrier to transition queue ownership.
                // For textures we have to use a texture barrier.
                //
                // 1. We are reading, and previous access was read:
                //    1.1. If queues are the same:
                //          - We don't need to insert a barrier.
                //    1.2. If queues are **not** the same:
                //          - We have to transition queue ownership.
                //
                // 2. We are reading, and previous access was write:
                //    2.1. If queues are the same:
                //         - We have to wait for source pipeline stage to finish writes.
                //    2.2. If queues are **not** the same:
                //          - We have to wait for source pipeline stage to finish writes,
                //              and transition queue ownership.
                //
                // 3. We are writing, and previous access was read.
                //    3.1. If queues are the same:
                //          - We have to wait for source pipeline stage to finish reads before we start writing.
                //              In case of textures we can discard previous contents.
                //    3.2. If queues are **not** the same:
                //          - We have to wait for source pipeline stage to finish reads before we start writing,
                //              and transition queue ownership.
                //              In case of textures we can discard previous contents.
                //
                // 4. We are writing, and previous access was write.
                //    4.1. If queues are the same:
                //          - We have to wait for source pipeline stage to finish writes before we start writing.
                //              In case of textures we can discard previous contents.
                //    4.2. If queues are **not** the same:
                //          - We have transition queue ownership and wait for source pipeline stage
                //              to finish writes before we start writing.
                //              In case of textures we can discard previous contents.
                //
                // 5. We are writing and reading, and previous access was read.
                //    5.1. If queues are the same:
                //          - We have to wait for source pipeline stage to finish reads
                //              before we start writing and reading.
                //    5.2. If queues are **not** the same:
                //          - We have to transition queue ownership and
                //              wait for source pipeline stage to finish reads
                //              before we start writing.
                //
                // 6. We are writing and reading, and previous access was write.
                //    6.1. If queues are the same:
                //          - We have to wait for source pipeline stage to finish writes
                //              before we start writing and reading.
                //    6.2. If queues are **not** the same:
                //          - We have transition queue ownership and
                //              wait for source pipeline stage to finish writes
                //              before we start writing.
                if (const auto it = last_resources_usage.find(resource_id);
                    it != last_resources_usage.end()) {
                    const LastResourceUsage& last_resource_usage = it->second;

                    const bool is_same_queue = map_queue_to_family_index(
                                                   m_queue_indices,
                                                   last_resource_usage.queue) ==
                                               map_queue_to_family_index(
                                                   m_queue_indices, pass_queue);

                    const bool discard_previous_contents = [&] {
                        const auto tuple = core::tie(
                            is_written, is_read, last_resource_usage.is_written);

                        if ((tuple == core::make_tuple(false, true, false)) ||
                            (tuple == core::make_tuple(false, true, true)) ||
                            (tuple == core::make_tuple(true, true, false)) ||
                            (tuple == core::make_tuple(true, true, true))) {
                            return false;
                        } else if (
                            (tuple == core::make_tuple(true, false, false)) ||
                            (tuple == core::make_tuple(true, false, true))) {
                            return true;
                        } else {
                            core::panic("?");
                        }
                    }();

                    if (is_same_queue) {
                        this->insert_barrier(
                            resource_id,
                            core::make_tuple(
                                last_resource_usage.usage,
                                last_resource_usage.is_written),
                            pass_id,
                            core::make_tuple(all_resource_usage, is_written),
                            discard_previous_contents);
                    } else {
                        this->queue_ownership_transfer(
                            resource_id,
                            last_resource_usage.render_pass,
                            core::make_tuple(
                                last_resource_usage.usage,
                                last_resource_usage.is_written),
                            pass_id,
                            core::make_tuple(all_resource_usage, is_written),
                            last_resource_usage.queue,
                            pass_queue,
                            discard_previous_contents);
                    }
                } else {
                    tndr_assert(!is_read, "No one used resource before us.");
                    // We are inserting a barrier, but only for textures. For buffers we don't bother.

                    const ResourceType resource_type =
                        m_resources[static_cast<usize>(resource_id)]->get_resource_type();
                    switch (resource_type) {
                        case ResourceType::Texture: {
                            const rhi::AccessFlags previous_access = map_resource_usage(
                                ResourceUsage::NONE, false);
                            const rhi::AccessFlags next_access = map_resource_usage(
                                all_resource_usage, is_written);

                            core::Array<TextureBarrier>& texture_barriers =
                                m_render_passes_barriers[static_cast<usize>(pass_id)]
                                    .texture_barriers.before;

                            texture_barriers.push_back(TextureBarrier {
                                .texture = resource_id,
                                .previous_access = previous_access,
                                .next_access = next_access,
                                .discard_contents = true,
                            });
                            break;
                        }
                        default:
                            break;
                    }
                }

                last_resources_usage.insert({
                    resource_id,
                    LastResourceUsage {
                        .render_pass = pass_id,
                        .queue = pass_queue,
                        .usage = all_resource_usage,
                        .is_written = is_written,
                    },
                });
            }

            // Prepare rhi::RenderPass
            if (pass_type == PassType::RenderPass) {
                IFrameGraphRenderPass* const render_pass =
                    static_cast<IFrameGraphRenderPass*>(pass.get());
                const RenderPass& fg_render_pass = render_pass->get_fg_render_pass();

                rhi::RenderPass& rhi_render_pass = render_pass->get_render_pass();
                rhi_render_pass.color_attachments.reserve(
                    fg_render_pass.color_attachments.size());

                for (const ColorAttachment& fg_color_attachment :
                     fg_render_pass.color_attachments) {
                    const ResourceId color_attachment_resource_id =
                        attachment_texture_to_resource_id(fg_color_attachment.texture);

                    tndr_assert(
                        resources_used.contains(color_attachment_resource_id), "");

                    rhi_render_pass.color_attachments.push_back(rhi::ColorAttachment {
                        .ops = fg_color_attachment.ops,
                        .texture_access = get_access_flags(color_attachment_resource_id),
                        .clear_value = fg_color_attachment.clear_value,
                    });

                    if (fg_color_attachment.resolve_texture.has_value()) {
                        const ResourceId color_attachment_resolve_resource_id =
                            attachment_texture_to_resource_id(
                                *fg_color_attachment.resolve_texture);

                        tndr_assert(
                            resources_used.contains(color_attachment_resolve_resource_id),
                            "");

                        rhi_render_pass.color_attachments.back()
                            .resolve_texture = rhi::ResolveTexture {
                            .texture_access = get_access_flags(
                                color_attachment_resolve_resource_id),
                        };
                    }
                }

                if (fg_render_pass.depth_stencil_attachment.has_value()) {
                    const DepthStencilAttachment& fg_depth_stencil_attachment =
                        *fg_render_pass.depth_stencil_attachment;

                    const ResourceId depth_stencil_attachment_resource_id =
                        attachment_texture_to_resource_id(
                            fg_depth_stencil_attachment.texture);

                    tndr_assert(
                        resources_used.contains(depth_stencil_attachment_resource_id),
                        "");

                    rhi_render_pass
                        .depth_stencil_attachment = rhi::DepthStencilAttachment {
                        .ops = fg_depth_stencil_attachment.ops,
                        .stencil_ops = fg_depth_stencil_attachment.stencil_ops,
                        .texture_access = get_access_flags(
                            depth_stencil_attachment_resource_id),
                        .clear_value = fg_depth_stencil_attachment.clear_value,
                    };

                    if (fg_depth_stencil_attachment.resolve_texture.has_value()) {
                        const ResourceId depth_stencil_attachment_resolve_resource_id =
                            attachment_texture_to_resource_id(
                                *fg_depth_stencil_attachment.resolve_texture);

                        tndr_assert(
                            resources_used.contains(
                                depth_stencil_attachment_resolve_resource_id),
                            "");

                        rhi_render_pass.depth_stencil_attachment
                            ->resolve_texture = rhi::ResolveTexture {
                            .texture_access = get_access_flags(
                                depth_stencil_attachment_resolve_resource_id),
                        };
                    }
                }
            }
        }
    }

    for (PresentPass& present_pass : m_present_passes) {
        const ResourceId resource_id = present_pass.texture.handle;

        if (auto it = last_resources_usage.find(resource_id);
            it != last_resources_usage.end()) {
            const LastResourceUsage& last_resource_usage = it->second;

            const rhi::AccessFlags previous_access = map_resource_usage(
                last_resource_usage.usage, last_resource_usage.is_written);

            const bool is_same_queue = map_queue_to_family_index(
                                           m_queue_indices, last_resource_usage.queue) ==
                                       map_queue_to_family_index(
                                           m_queue_indices, QueueType::Present);

            if (is_same_queue) {
                present_pass.barrier = TextureBarrier {
                    .texture = resource_id,
                    .previous_access = get_access_flags(resource_id),
                    .next_access = PresentPass::ACCESS_FLAGS,
                    .discard_contents = false,
                };
            } else {
                // Release barrier
                {
                    core::Array<TextureBarrier>& texture_barriers =
                        m_render_passes_barriers[static_cast<usize>(
                                                     last_resource_usage.render_pass)]
                            .texture_barriers.after;

                    texture_barriers.push_back(TextureBarrier {
                        .texture = resource_id,
                        .previous_access = previous_access,
                        .next_access = PresentPass::ACCESS_FLAGS,
                        .source_queue = last_resource_usage.queue,
                        .destination_queue = QueueType::Present,
                        .discard_contents = false,
                    });
                }

                // Acquire barrier
                present_pass.barrier = TextureBarrier {
                    .texture = resource_id,
                    .previous_access = get_access_flags(resource_id),
                    .next_access = PresentPass::ACCESS_FLAGS,
                    .source_queue = last_resource_usage.queue,
                    .destination_queue = QueueType::Present,
                    .discard_contents = false,
                };
            }
        } else {
            core::panic("No one used resource before us!");
        }
    }
}

void FrameGraph::insert_barrier(
    const ResourceId resource_id,
    const core::Tuple<ResourceUsage, bool>& previous_usage,
    const RenderPassId pass_id,
    const core::Tuple<ResourceUsage, bool>& next_usage,
    const bool discard_contents) noexcept
{
    const ResourceType resource_type = m_resources[static_cast<usize>(resource_id)]
                                           ->get_resource_type();
    const rhi::AccessFlags previous_access = map_resource_usage(
        core::get<ResourceUsage>(previous_usage), core::get<bool>(previous_usage));
    const rhi::AccessFlags next_access = map_resource_usage(
        core::get<ResourceUsage>(next_usage), core::get<bool>(next_usage));

    switch (resource_type) {
        case ResourceType::Buffer: {
            // #TODO: For buffers always use global barrier, at least for now.
            // If we are only reading, then barrier is unnecessary.
            if (core::get<bool>(previous_usage) || core::get<bool>(next_usage)) {
                core::Option<GlobalBarrier>& global_barrier =
                    m_render_passes_barriers[static_cast<usize>(pass_id)]
                        .global_barrier.before;
                if (global_barrier.has_value()) {
                    global_barrier->previous_access |= previous_access;
                    global_barrier->next_access |= next_access;
                } else {
                    global_barrier = GlobalBarrier {
                        .previous_access = previous_access,
                        .next_access = next_access,
                    };
                }
            }
            break;
        }
        case ResourceType::Texture: {
            if (core::get<ResourceUsage>(previous_usage) ==
                core::get<ResourceUsage>(next_usage)) {
                if (core::get<bool>(previous_usage) || core::get<bool>(next_usage)) {
                    core::Option<GlobalBarrier>& global_barrier =
                        m_render_passes_barriers[static_cast<usize>(pass_id)]
                            .global_barrier.before;
                    if (global_barrier.has_value()) {
                        global_barrier->previous_access |= previous_access;
                        global_barrier->next_access |= next_access;
                    } else {
                        global_barrier = GlobalBarrier {
                            .previous_access = previous_access,
                            .next_access = next_access,
                        };
                    }
                }
            } else {
                // normal barrier
                core::Array<TextureBarrier>& texture_barriers =
                    m_render_passes_barriers[static_cast<usize>(pass_id)]
                        .texture_barriers.before;

                texture_barriers.push_back(TextureBarrier {
                    .texture = resource_id,
                    .previous_access = previous_access,
                    .next_access = next_access,
                    .discard_contents = discard_contents,
                });
            }
            break;
        }
        default:
            core::unreachable();
    }
}

void FrameGraph::queue_ownership_transfer(
    const ResourceId resource_id,
    const RenderPassId previous_pass_id,
    const core::Tuple<ResourceUsage, bool>& previous_usage,
    const RenderPassId pass_id,
    const core::Tuple<ResourceUsage, bool>& next_usage,
    const QueueType source_queue,
    const QueueType destination_queue,
    const bool discard_contents) noexcept
{
    const ResourceType resource_type = m_resources[static_cast<usize>(resource_id)]
                                           ->get_resource_type();
    const rhi::AccessFlags previous_access = map_resource_usage(
        core::get<ResourceUsage>(previous_usage), core::get<bool>(previous_usage));
    const rhi::AccessFlags next_access = map_resource_usage(
        core::get<ResourceUsage>(next_usage), core::get<bool>(next_usage));

    switch (resource_type) {
        case ResourceType::Buffer: {
            // Release barrier
            {
                core::Array<BufferBarrier>& buffer_barriers =
                    m_render_passes_barriers[static_cast<usize>(previous_pass_id)]
                        .buffer_barriers.after;

                buffer_barriers.push_back(BufferBarrier {
                    .buffer = resource_id,
                    .previous_access = previous_access,
                    .next_access = next_access,
                    .source_queue = source_queue,
                    .destination_queue = destination_queue,
                });
            }

            // Acquire barrier
            {
                core::Array<BufferBarrier>& buffer_barriers =
                    m_render_passes_barriers[static_cast<usize>(pass_id)]
                        .buffer_barriers.before;

                buffer_barriers.push_back(BufferBarrier {
                    .buffer = resource_id,
                    .previous_access = previous_access,
                    .next_access = next_access,
                    .source_queue = source_queue,
                    .destination_queue = destination_queue,
                });
            }
            break;
        }
        case ResourceType::Texture: {
            // Release barrier
            {
                core::Array<TextureBarrier>& texture_barriers =
                    m_render_passes_barriers[static_cast<usize>(previous_pass_id)]
                        .texture_barriers.after;

                texture_barriers.push_back(TextureBarrier {
                    .texture = resource_id,
                    .previous_access = previous_access,
                    .next_access = next_access,
                    .source_queue = source_queue,
                    .destination_queue = destination_queue,
                    .discard_contents = discard_contents,
                });
            }

            // Acquire barrier
            {
                core::Array<TextureBarrier>& texture_barriers =
                    m_render_passes_barriers[static_cast<usize>(pass_id)]
                        .texture_barriers.before;

                texture_barriers.push_back(TextureBarrier {
                    .texture = resource_id,
                    .previous_access = previous_access,
                    .next_access = next_access,
                    .source_queue = source_queue,
                    .destination_queue = destination_queue,
                    .discard_contents = discard_contents,
                });
            }
            break;
        }
        default:
            core::unreachable();
    }
}

bool FrameGraph::validate_render_pass(const RenderPass& render_pass) const noexcept
{
    core::HashSet<ResourceId> set;

    bool contains_duplicates = !std::any_of(
        render_pass.color_attachments.begin(),
        render_pass.color_attachments.end(),
        [&](const ColorAttachment& attachment) {
            const bool a =
                set.insert(attachment_texture_to_resource_id(attachment.texture)).second;

            if (attachment.resolve_texture.has_value()) {
                return a && set.insert(attachment_texture_to_resource_id(
                                           *attachment.resolve_texture))
                                .second;
            }

            return a;
        });

    if (render_pass.depth_stencil_attachment.has_value()) {
        contains_duplicates |= !set
                                    .insert(attachment_texture_to_resource_id(
                                        render_pass.depth_stencil_attachment->texture))
                                    .second;

        if (render_pass.depth_stencil_attachment->resolve_texture.has_value()) {
            contains_duplicates |= !set.insert(attachment_texture_to_resource_id(
                                                   *render_pass.depth_stencil_attachment
                                                        ->resolve_texture))
                                        .second;
        }
    }

    return contains_duplicates;
}

FrameGraph::RenderPassResources& FrameGraph::get_render_pass_resources(
    const RenderPassId render_pass) noexcept
{
    return m_render_passes_resources[static_cast<usize>(render_pass)];
}

TextureHandle FrameGraph::create_texture(
    const RenderPassId creator,
    const core::String& name,
    const TextureCreateInfo& create_info) noexcept
{
    const ResourceId handle = static_cast<ResourceId>(m_resources.size());
    const TextureHandle fg_handle { handle };
    m_resources.emplace_back(
        core::make_unique<TextureResource>(creator, fg_handle, name, create_info));

    return fg_handle;
}

BufferHandle FrameGraph::create_buffer(
    const RenderPassId creator,
    const core::String& name,
    const BufferCreateInfo& create_info) noexcept
{
    const ResourceId handle = static_cast<ResourceId>(m_resources.size());
    const BufferHandle fg_handle { handle };
    m_resources.emplace_back(
        core::make_unique<BufferResource>(creator, fg_handle, name, create_info));

    return fg_handle;
}

} // namespace tundra::renderer::frame_graph
