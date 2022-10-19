#include "commands/vulkan_command_decoder.h"
#include "core/profiler.h"
#include "core/std/option.h"
#include "managers/vulkan_descriptor_bindless_manager.h"
#include "managers/vulkan_pipeline_layout_manager.h"
#include "resources/vulkan_buffer.h"
#include "resources/vulkan_compute_pipeline.h"
#include "resources/vulkan_graphics_pipeline.h"
#include "resources/vulkan_texture.h"
#include "resources/vulkan_texture_view.h"
#include "rhi/commands/command_encoder.h"
#include "vulkan_device.h"
#include "vulkan_helpers.h"
#include "vulkan_instance.h"

static constexpr bool PROFILE_DECODER = true;

namespace tundra::vulkan_rhi {

VulkanCommandDecoder::VulkanCommandDecoder(
    const core::SharedPtr<VulkanRawDevice>& raw_device,
    Managers& managers,
    VulkanCommandBufferManager::CommandBundle bundle) noexcept
    : m_raw_device(raw_device)
    , m_loader_device(m_raw_device->get_device())
    , m_bundle(core::move(bundle))
    , m_resources(m_bundle.get_resources())
    , m_managers(managers)
    , m_barrier(m_raw_device)
    , m_device_limits(m_raw_device->get_device_limits())
{
    m_cache.pipeline_layout = m_managers.pipeline_layout_manager->get_pipeline_layout()
                                  .pipeline_layout;
}

[[nodiscard]] VkCommandBuffer VulkanCommandDecoder::decode(
    const rhi::CommandEncoder& encoder) noexcept
{
    TNDR_PROFILER_TRACE("VulkanCommandDecoder::decode");

    encoder.execute(core::make_overload(
        [&](const rhi::commands::BeginCommandBufferCommand& cmd) {
            this->begin_command_buffer(cmd);
        },
        [&](const rhi::commands::EndCommandBufferCommand& cmd) {
            this->end_command_buffer(cmd);
        },
        [&](const rhi::commands::BeginRegionCommand& cmd) {
            helpers::begin_region(
                m_raw_device, m_bundle.command_buffer, cmd.name, cmd.color);
        },
        [&](const rhi::commands::EndRegionCommand&) {
            helpers::end_region(m_raw_device, m_bundle.command_buffer);
        },
        [&](const rhi::commands::BeginRenderPassCommand& cmd) {
            this->begin_render_pass(cmd);
        },
        [&](const rhi::commands::EndRenderPassCommand& cmd) {
            this->end_render_pass(cmd);
        },
        [&](const rhi::commands::PushConstantsCommand& cmd) {
            this->push_constants(cmd);
        },
        [&](const rhi::commands::BindGraphicsPipelineCommand& cmd) {
            this->bind_graphics_pipeline(cmd);
        },
        [&](const rhi::commands::SetViewportCommand& cmd) { //
            this->set_viewport(cmd);
        },
        [&](const rhi::commands::SetScissorCommand& cmd) { //
            this->set_scissor(cmd);
        },
        [&](const rhi::commands::SetCullingModeCommand& cmd) {
            this->set_culling_mode(cmd);
        },
        [&](const rhi::commands::BindIndexBufferCommand& cmd) {
            this->bind_index_buffer(cmd);
        },
        [&](const rhi::commands::DrawCommand& cmd) { //
            this->draw(cmd);
        },
        [&](const rhi::commands::DrawIndexedCommand& cmd) { //
            this->draw_indexed(cmd);
        },
        [&](const rhi::commands::DrawIndexedInstancedCommand& cmd) {
            this->draw_indexed_instanced(cmd);
        },
        [&](const rhi::commands::DrawIndexedIndirectCommand& cmd) {
            this->draw_indexed_indirect(cmd);
        },
        [&](const rhi::commands::DrawIndexedIndirectCountCommand& cmd) {
            this->draw_indexed_indirect_count(cmd);
        },
        [&](const rhi::commands::DispatchCommand& cmd) { //
            this->dispatch(cmd);
        },
        [&](const rhi::commands::DispatchIndirectCommand& cmd) { //
            this->dispatch_indirect(cmd);
        },
        [&](const rhi::commands::BufferCopyCommand& cmd) { //
            this->buffer_copy(cmd);
        },
        [&](const rhi::commands::TextureCopyCommand& cmd) { //
            this->texture_copy(cmd);
        },
        [&](const rhi::commands::BufferTextureCopyCommand& cmd) { //
            this->buffer_texture_copy(cmd);
        },
        [&](const rhi::commands::TextureBufferCopyCommand& cmd) { //
            this->texture_buffer_copy(cmd);
        },
        [&](const rhi::commands::GlobalBarrierCommand& cmd) {
            this->global_barrier(cmd);
        },
        [&](const rhi::commands::TextureBarrierCommand& cmd) {
            this->texture_barrier(cmd);
        },
        [&](const rhi::commands::BufferBarrierCommand& cmd) {
            this->buffer_barrier(cmd);
        }));

    return m_bundle.command_buffer;
}

void VulkanCommandDecoder::begin_command_buffer(
    const rhi::commands::BeginCommandBufferCommand&) noexcept
{
    TNDR_PROFILER_TRACE_IF(PROFILE_DECODER, "VulkanCommandDecoder::begin_command_buffer");

    const VkCommandBufferBeginInfo command_buffer_begin_info {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };

    vulkan_map_result(
        m_loader_device.begin_command_buffer(
            m_bundle.command_buffer, command_buffer_begin_info),
        "`begin_command_buffer` failed");

    auto& descriptor_bindless_manager = m_managers.descriptor_bindless_manager;
    descriptor_bindless_manager->bind_descriptors(
        m_bundle.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS);
    descriptor_bindless_manager->bind_descriptors(
        m_bundle.command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE);
}

void VulkanCommandDecoder::end_command_buffer(
    const rhi::commands::EndCommandBufferCommand&) noexcept
{
    TNDR_PROFILER_TRACE_IF(PROFILE_DECODER, "VulkanCommandDecoder::end_command_buffer");

    vulkan_map_result(
        m_loader_device.end_command_buffer(m_bundle.command_buffer),
        "`end_command_buffer` failed");
}

void VulkanCommandDecoder::begin_render_pass(
    const rhi::commands::BeginRenderPassCommand& cmd) noexcept
{
    TNDR_PROFILER_TRACE_IF(PROFILE_DECODER, "VulkanCommandDecoder::begin_render_pass");

    // #TODO: Is this really necessary?
    // For color attachments `access_flags` can only be set to:
    // - `AccessFlags::GENERAL`
    // - `AccessFlags::COLOR_ATTACHMENT_WRITE`
    // - `AccessFlags::COLOR_ATTACHMENT`
    //
    // `texture_usage` must have `TextureUsageFlags::COLOR_ATTACHMENT` bit set.
    // Translating `access_flags` to `VkImageLayout` will yield two layouts:
    // - `VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL`
    // - `VK_IMAGE_LAYOUT_GENERAL`
    // and both layouts are compatible with `VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT`.
    // The same thing happen with depth stencil attachments.
    const auto check_is_layout_allowed = [](const rhi::AccessFlags access_flags,
                                            const rhi::TextureUsageFlags texture_usage) {
        const VkImageLayout current_layout = helpers::map_access_flags_to_image_layout(
            access_flags);

        if (!helpers::is_layout_allowed(current_layout, texture_usage)) {
            core::panic("{} is not allowed with: {}.", current_layout, texture_usage);
        }
    };

    u32 color_attachments_count = 0;
    VkRenderingAttachmentInfo color_attachments[rhi::config::MAX_NUM_ATTACHMENTS];
    bool has_depth_stencil = false;
    VkRenderingAttachmentInfo depth_stencil_attachment;

    for (const rhi::ColorAttachment& color_attachment :
         cmd.render_pass.color_attachments) {
        const VkImageView image_view =
            core::visit(
                core::make_overload(
                    [&](rhi::TextureHandleType handle) {
                        m_resources->add_reference(
                            *m_managers.resource_tracker, handle.get_id());

                        return m_managers.texture_manager->with(
                            handle, [&](const VulkanTexture& texture) {
                                check_is_layout_allowed(
                                    color_attachment.texture_access, texture.get_usage());
                                return texture.get_image_view();
                            });
                    },
                    [&](rhi::TextureViewHandleType handle)
                        -> core::Expected<VkImageView, rhi::HandleManagerError> {
                        m_resources->add_reference(
                            *m_managers.resource_tracker, handle.get_id());

                        return m_managers.texture_view_manager->with(
                            handle, [&](const VulkanTextureView& texture_view) {
                                check_is_layout_allowed(
                                    color_attachment.texture_access,
                                    texture_view.get_usage());
                                return texture_view.get_image_view();
                            });
                    }),
                color_attachment.texture)
                .value_or_else([](const auto&) -> VkImageView {
                    core::panic("`ColorAttachment::texture` is not alive.");
                });

        VkImageView resolve_image_view = VK_NULL_HANDLE;
        VkImageLayout resolve_image_layout = VK_IMAGE_LAYOUT_UNDEFINED;
        VkResolveModeFlagBits resolve_mode = VK_RESOLVE_MODE_NONE;
        if (color_attachment.resolve_texture.has_value()) {
            const rhi::ResolveTexture& resolve_texture = *color_attachment.resolve_texture;

            resolve_image_view =
                core::visit(
                    core::make_overload(
                        [&](rhi::TextureHandleType handle) {
                            m_resources->add_reference(
                                *m_managers.resource_tracker, handle.get_id());

                            return m_managers.texture_manager->with(
                                handle, [&](const VulkanTexture& texture) {
                                    check_is_layout_allowed(
                                        resolve_texture.texture_access,
                                        texture.get_usage());
                                    return texture.get_image_view();
                                });
                        },
                        [&](rhi::TextureViewHandleType handle)
                            -> core::Expected<VkImageView, rhi::HandleManagerError> {
                            m_resources->add_reference(
                                *m_managers.resource_tracker, handle.get_id());

                            return m_managers.texture_view_manager->with(
                                handle, [&](const VulkanTextureView& texture_view) {
                                    check_is_layout_allowed(
                                        color_attachment.texture_access,
                                        texture_view.get_usage());
                                    return texture_view.get_image_view();
                                });
                        }),
                    resolve_texture.resolve_texture)
                    .value_or_else([](const auto&) -> VkImageView {
                        core::panic("`ColorAttachment::resolve_texture` is not alive.");
                    });

            resolve_mode = VK_RESOLVE_MODE_AVERAGE_BIT;
            resolve_image_layout = helpers::map_access_flags_to_image_layout(
                resolve_texture.texture_access);
        }

        color_attachments[color_attachments_count] = VkRenderingAttachmentInfo {
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .pNext = nullptr,
            .imageView = image_view,
            .imageLayout = helpers::map_access_flags_to_image_layout(
                color_attachment.texture_access),
            .resolveMode = resolve_mode,
            .resolveImageView = resolve_image_view,
            .resolveImageLayout = resolve_image_layout,
            .loadOp = helpers::map_attachment_load_op(color_attachment.ops.load),
            .storeOp = helpers::map_attachment_store_op(color_attachment.ops.store),
            .clearValue = helpers::map_clear_value(color_attachment.clear_value),
        };

        color_attachments_count += 1;
    }

    // Depth stencil
    if (cmd.render_pass.depth_stencil_attachment.has_value()) {
        const rhi::DepthStencilAttachment& depth_stencil_attachment_ref =
            *cmd.render_pass.depth_stencil_attachment;

        const VkImageView image_view =
            core::visit(
                core::make_overload(
                    [&](rhi::TextureHandleType handle) {
                        m_resources->add_reference(
                            *m_managers.resource_tracker, handle.get_id());

                        return m_managers.texture_manager->with(
                            handle, [&](const VulkanTexture& texture) {
                                check_is_layout_allowed(
                                    depth_stencil_attachment_ref.texture_access,
                                    texture.get_usage());
                                return texture.get_image_view();
                            });
                    },
                    [&](rhi::TextureViewHandleType handle)
                        -> core::Expected<VkImageView, rhi::HandleManagerError> {
                        m_resources->add_reference(
                            *m_managers.resource_tracker, handle.get_id());

                        return m_managers.texture_view_manager->with(
                            handle, [&](const VulkanTextureView& texture_view) {
                                check_is_layout_allowed(
                                    depth_stencil_attachment_ref.texture_access,
                                    texture_view.get_usage());
                                return texture_view.get_image_view();
                            });
                    }),
                depth_stencil_attachment_ref.texture)
                .value_or_else([](const auto&) -> VkImageView {
                    core::panic("`DepthStencilAttachment::texture` is not alive.");
                });

        VkImageView resolve_image_view = VK_NULL_HANDLE;
        VkImageLayout resolve_image_layout = VK_IMAGE_LAYOUT_UNDEFINED;
        VkResolveModeFlagBits resolve_mode = VK_RESOLVE_MODE_NONE;
        if (depth_stencil_attachment_ref.resolve_texture.has_value()) {
            const rhi::ResolveTexture& resolve_texture = *depth_stencil_attachment_ref
                                                              .resolve_texture;
            resolve_image_view =
                core::visit(
                    core::make_overload(
                        [&](rhi::TextureHandleType handle) {
                            m_resources->add_reference(
                                *m_managers.resource_tracker, handle.get_id());

                            return m_managers.texture_manager->with(
                                handle, [&](const VulkanTexture& texture) {
                                    check_is_layout_allowed(
                                        resolve_texture.texture_access,
                                        texture.get_usage());
                                    return texture.get_image_view();
                                });
                        },
                        [&](rhi::TextureViewHandleType handle)
                            -> core::Expected<VkImageView, rhi::HandleManagerError> {
                            m_resources->add_reference(
                                *m_managers.resource_tracker, handle.get_id());

                            return m_managers.texture_view_manager->with(
                                handle, [&](const VulkanTextureView& texture_view) {
                                    check_is_layout_allowed(
                                        depth_stencil_attachment_ref.texture_access,
                                        texture_view.get_usage());
                                    return texture_view.get_image_view();
                                });
                        }),
                    resolve_texture.resolve_texture)
                    .value_or_else([](const auto&) -> VkImageView {
                        core::panic(
                            "`DepthStencilAttachment::resolve_texture` is not alive.");
                    });

            resolve_mode = VK_RESOLVE_MODE_AVERAGE_BIT;
            resolve_image_layout = helpers::map_access_flags_to_image_layout(
                resolve_texture.texture_access);
        }

        depth_stencil_attachment = VkRenderingAttachmentInfo {
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .pNext = nullptr,
            .imageView = image_view,
            .imageLayout = helpers::map_access_flags_to_image_layout(
                depth_stencil_attachment_ref.texture_access),
            .resolveMode = resolve_mode,
            .resolveImageView = resolve_image_view,
            .resolveImageLayout = resolve_image_layout,
            .loadOp = helpers::map_attachment_load_op(
                depth_stencil_attachment_ref.ops.load),
            .storeOp = helpers::map_attachment_store_op(
                depth_stencil_attachment_ref.ops.store),
            .clearValue =
                VkClearValue {
                    .depthStencil =
                        VkClearDepthStencilValue {
                            .depth = depth_stencil_attachment_ref.clear_value.depth,
                            .stencil = depth_stencil_attachment_ref.clear_value.stencil,
                        },
                },
        };

        has_depth_stencil = true;
    }

    const VkRenderingInfo rendering_info {
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea =
            VkRect2D {
                .offset =
                    VkOffset2D {
                        .x = cmd.render_area.offset.x,
                        .y = cmd.render_area.offset.y,
                    },
                .extent =
                    VkExtent2D {
                        .width = cmd.render_area.extent.x,
                        .height = cmd.render_area.extent.y,
                    },
            },
        .layerCount = 1,
        .viewMask = 0,
        .colorAttachmentCount = color_attachments_count,
        .pColorAttachments = color_attachments_count > 0 ? color_attachments : nullptr,
        .pDepthAttachment = has_depth_stencil ? &depth_stencil_attachment : nullptr,
        .pStencilAttachment = has_depth_stencil ? &depth_stencil_attachment : nullptr,
    };

    m_loader_device.cmd_begin_rendering(m_bundle.command_buffer, rendering_info);
}

void VulkanCommandDecoder::end_render_pass(
    const rhi::commands::EndRenderPassCommand&) noexcept
{
    TNDR_PROFILER_TRACE_IF(PROFILE_DECODER, "VulkanCommandDecoder::end_render_pass");

    m_loader_device.cmd_end_rendering(m_bundle.command_buffer);
}

void VulkanCommandDecoder::push_constants(
    const rhi::commands::PushConstantsCommand& cmd) noexcept
{
    TNDR_PROFILER_TRACE_IF(PROFILE_DECODER, "VulkanCommandDecoder::push_constants");

    m_resources->add_reference(
        *m_managers.resource_tracker, cmd.ubo_buffer.get_handle().get_id());

    const rhi::PushConstants push_constants {
        .buffer_index = cmd.ubo_buffer.get_srv(),
        .buffer_offset = cmd.offset,
    };

    m_loader_device.cmd_push_constants(
        m_bundle.command_buffer,
        m_cache.pipeline_layout,
        VK_SHADER_STAGE_ALL,
        0,
        core::as_byte_span(push_constants));
}

void VulkanCommandDecoder::bind_graphics_pipeline(
    const rhi::commands::BindGraphicsPipelineCommand& cmd) noexcept
{
    TNDR_PROFILER_TRACE_IF(
        PROFILE_DECODER, "VulkanCommandDecoder::bind_graphics_pipeline");

    if (m_cache.current_graphics_pipeline != cmd.pipeline) {
        m_resources->add_reference(
            *m_managers.resource_tracker, cmd.pipeline.get_handle().get_id());
        m_cache.current_graphics_pipeline = cmd.pipeline;

        const VkPipeline pipeline =
            m_managers.graphics_pipeline_manager
                ->with(
                    cmd.pipeline.get_handle(),
                    [&](const VulkanGraphicsPipeline& p) { return p.get_pipeline(); })
                .value_or_else([](const auto&) -> VkPipeline {
                    core::panic("`BindGraphicsPipelineCommand::pipeline` is not alive.");
                });

        m_loader_device.cmd_bind_pipeline(
            m_bundle.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    }
}

void VulkanCommandDecoder::set_viewport(
    const rhi::commands::SetViewportCommand& cmd) noexcept
{
    TNDR_PROFILER_TRACE_IF(PROFILE_DECODER, "VulkanCommandDecoder::set_viewport");

    const VkViewport viewport {
        .x = static_cast<f32>(cmd.viewport.rect.offset.x),
        .y = static_cast<f32>(cmd.viewport.rect.offset.y),
        .width = static_cast<f32>(cmd.viewport.rect.extent.x),
        .height = static_cast<f32>(cmd.viewport.rect.extent.y),
        .minDepth = cmd.viewport.depth_min,
        .maxDepth = cmd.viewport.depth_max,
    };

    m_loader_device.cmd_set_viewport(m_bundle.command_buffer, 0, core::as_span(viewport));
}

void VulkanCommandDecoder::set_scissor(
    const rhi::commands::SetScissorCommand& cmd) noexcept
{
    TNDR_PROFILER_TRACE_IF(PROFILE_DECODER, "VulkanCommandDecoder::set_scissor");

    const VkRect2D scissor {
        .offset =
            VkOffset2D {
                .x = cmd.scissor.offset.x,
                .y = cmd.scissor.offset.y,
            },
        .extent =
            VkExtent2D {
                .width = cmd.scissor.extent.x,
                .height = cmd.scissor.extent.y,
            },
    };

    m_loader_device.cmd_set_scissor(m_bundle.command_buffer, 0, core::as_span(scissor));
}

void VulkanCommandDecoder::set_culling_mode(
    const rhi::commands::SetCullingModeCommand& cmd) noexcept
{
    TNDR_PROFILER_TRACE_IF(PROFILE_DECODER, "VulkanCommandDecoder::set_culling_mode");

    m_loader_device.cmd_set_cull_mode(
        m_bundle.command_buffer, helpers::map_culling_mode(cmd.culling_mode));
}

void VulkanCommandDecoder::bind_index_buffer(
    const rhi::commands::BindIndexBufferCommand& cmd) noexcept
{
    TNDR_PROFILER_TRACE_IF(PROFILE_DECODER, "VulkanCommandDecoder::bind_index_buffer");

    const auto index_buffer_tie = core::tie(cmd.buffer, cmd.index_type, cmd.offset);
    if (m_cache.current_index_buffer != index_buffer_tie) {
        m_resources->add_reference(
            *m_managers.resource_tracker, cmd.buffer.get_handle().get_id());
        m_cache.current_index_buffer = index_buffer_tie;

        const VkBuffer buffer =
            m_managers.buffer_manager
                ->with(
                    cmd.buffer.get_handle(),
                    [](const VulkanBuffer& buffer) { return buffer.get_buffer(); })
                .value_or_else([](const auto&) -> VkBuffer {
                    core::panic("`BindIndexBufferCommand::buffer` is not alive.");
                });

        const VkIndexType index_type = [&] {
            switch (cmd.index_type) {
                case rhi::IndexType::U16:
                    return VK_INDEX_TYPE_UINT16;
                case rhi::IndexType::U32:
                    return VK_INDEX_TYPE_UINT32;
                default:
                    core::panic("Invalid enum!");
            }
        }();

        m_loader_device.cmd_bind_index_buffer(
            m_bundle.command_buffer, buffer, cmd.offset, index_type);
    }
}

void VulkanCommandDecoder::draw(const rhi::commands::DrawCommand& cmd) noexcept
{
    TNDR_PROFILER_TRACE_IF(PROFILE_DECODER, "VulkanCommandDecoder::draw");

    m_loader_device.cmd_draw(
        m_bundle.command_buffer, cmd.vertex_count, 1, cmd.first_vertex, 0);
}

void VulkanCommandDecoder::draw_indexed(
    const rhi::commands::DrawIndexedCommand& cmd) noexcept
{
    TNDR_PROFILER_TRACE_IF(PROFILE_DECODER, "VulkanCommandDecoder::draw_indexed");

    m_loader_device.cmd_draw_indexed(
        m_bundle.command_buffer, cmd.indices_count, 1, cmd.first_index, 0, 0);
}

void VulkanCommandDecoder::draw_indexed_instanced(
    const rhi::commands::DrawIndexedInstancedCommand& cmd) noexcept
{
    TNDR_PROFILER_TRACE_IF(
        PROFILE_DECODER, "VulkanCommandDecoder::draw_indexed_instanced");

    m_loader_device.cmd_draw_indexed(
        m_bundle.command_buffer,
        cmd.indices_count,
        cmd.num_instances,
        cmd.first_index,
        cmd.vertex_offset,
        cmd.first_instance);
}

void VulkanCommandDecoder::draw_indexed_indirect(
    const rhi::commands::DrawIndexedIndirectCommand& cmd) noexcept
{
    TNDR_PROFILER_TRACE_IF(
        PROFILE_DECODER, "VulkanCommandDecoder::draw_indexed_indirect");

    m_resources->add_reference(
        *m_managers.resource_tracker, cmd.buffer.get_handle().get_id());

    const VkBuffer indirect_buffer =
        m_managers.buffer_manager
            ->with(
                cmd.buffer.get_handle(),
                [](const VulkanBuffer& buffer) { return buffer.get_buffer(); })
            .value_or_else([](const auto&) -> VkBuffer {
                core::panic("`DrawIndexedIndirectCommand::buffer` is not alive.");
            });

    m_loader_device.cmd_draw_indexed_indirect(
        m_bundle.command_buffer, indirect_buffer, cmd.offset, cmd.draw_count, cmd.stride);
}

void VulkanCommandDecoder::draw_indexed_indirect_count(
    const rhi::commands::DrawIndexedIndirectCountCommand& cmd) noexcept
{
    TNDR_PROFILER_TRACE_IF(
        PROFILE_DECODER, "VulkanCommandDecoder::draw_indexed_indirect_count");

    m_resources->add_reference(
        *m_managers.resource_tracker, cmd.buffer.get_handle().get_id());
    m_resources->add_reference(
        *m_managers.resource_tracker, cmd.count_buffer.get_handle().get_id());

    const VkBuffer indirect_buffer =
        m_managers.buffer_manager
            ->with(
                cmd.buffer.get_handle(),
                [](const VulkanBuffer& buffer) { return buffer.get_buffer(); })
            .value_or_else([](const auto&) -> VkBuffer {
                core::panic("`DrawIndexedIndirectCountCommand::buffer` is not alive.");
            });

    const VkBuffer count_buffer =
        m_managers.buffer_manager
            ->with(
                cmd.count_buffer.get_handle(),
                [](const VulkanBuffer& buffer) { return buffer.get_buffer(); })
            .value_or_else([](const auto&) -> VkBuffer {
                core::panic(
                    "`DrawIndexedIndirectCountCommand::count_buffer` is not alive.");
            });

    m_loader_device.cmd_draw_indexed_indirect_count(
        m_bundle.command_buffer,
        indirect_buffer,
        cmd.offset,
        count_buffer,
        cmd.count_buffer_offset,
        cmd.max_draw_count,
        cmd.stride);
}

void VulkanCommandDecoder::dispatch(const rhi::commands::DispatchCommand& cmd) noexcept
{
    TNDR_PROFILER_TRACE_IF(PROFILE_DECODER, "VulkanCommandDecoder::dispatch");

    this->bind_compute_pipeline(cmd.pipeline);

    m_loader_device.cmd_dispatch(
        m_bundle.command_buffer, cmd.group_count_x, cmd.group_count_y, cmd.group_count_z);
}

void VulkanCommandDecoder::dispatch_indirect(
    const rhi::commands::DispatchIndirectCommand& cmd) noexcept
{
    TNDR_PROFILER_TRACE_IF(PROFILE_DECODER, "VulkanCommandDecoder::dispatch_indirect");

    m_resources->add_reference(
        *m_managers.resource_tracker, cmd.buffer.get_handle().get_id());

    this->bind_compute_pipeline(cmd.pipeline);

    const VkBuffer indirect_buffer =
        m_managers.buffer_manager
            ->with(
                cmd.buffer.get_handle(),
                [](const VulkanBuffer& buffer) { return buffer.get_buffer(); })
            .value_or_else([](const auto&) -> VkBuffer {
                core::panic("`DispatchIndirectCommand::buffer` is not alive.");
            });

    m_loader_device.cmd_dispatch_indirect(
        m_bundle.command_buffer, indirect_buffer, cmd.offset);
}

void VulkanCommandDecoder::buffer_copy(
    const rhi::commands::BufferCopyCommand& cmd) noexcept
{
    TNDR_PROFILER_TRACE_IF(PROFILE_DECODER, "VulkanCommandDecoder::buffer_copy");

    m_resources->add_reference(
        *m_managers.resource_tracker, cmd.src.get_handle().get_id());
    m_resources->add_reference(
        *m_managers.resource_tracker, cmd.dst.get_handle().get_id());

    const VkBuffer src = m_managers.buffer_manager
                             ->with(
                                 cmd.src.get_handle(),
                                 [](const VulkanBuffer& buffer) {
                                     return buffer.get_buffer();
                                 })
                             .value_or_else([](const auto&) -> VkBuffer {
                                 core::panic("`BufferCopyCommand::src` is not alive.");
                             });

    const VkBuffer dst = m_managers.buffer_manager
                             ->with(
                                 cmd.dst.get_handle(),
                                 [](const VulkanBuffer& buffer) {
                                     return buffer.get_buffer();
                                 })
                             .value_or_else([](const auto&) -> VkBuffer {
                                 core::panic("`BufferCopyCommand::dst` is not alive.");
                             });

    core::Array<VkBufferCopy> regions;
    regions.reserve(cmd.regions.size());

    for (const rhi::BufferCopyRegion& region : cmd.regions) {
        regions.push_back(VkBufferCopy {
            .srcOffset = region.src_offset,
            .dstOffset = region.dst_offset,
            .size = region.size,
        });
    }

    m_loader_device.cmd_copy_buffer(
        m_bundle.command_buffer, src, dst, core::as_span(regions));
}

void VulkanCommandDecoder::texture_copy(
    const rhi::commands::TextureCopyCommand& cmd) noexcept
{
    TNDR_PROFILER_TRACE_IF(PROFILE_DECODER, "VulkanCommandDecoder::texture_copy");

    m_resources->add_reference(
        *m_managers.resource_tracker, cmd.src.get_handle().get_id());
    m_resources->add_reference(
        *m_managers.resource_tracker, cmd.dst.get_handle().get_id());

    const auto [src_image, src_image_aspect_flags] =
        m_managers.texture_manager
            ->with(
                cmd.src.get_handle(),
                [](const VulkanTexture& texture) {
                    return core::make_tuple(
                        texture.get_image(),
                        helpers::map_texture_aspect(
                            rhi::get_texture_format_desc(texture.get_format()).aspect));
                })
            .value_or_else([](const auto&) -> core::Tuple<VkImage, VkImageAspectFlags> {
                core::panic("`TextureCopyCommand::src` is not alive.");
            });

    const auto [dst_image, dst_image_aspect_flags] =
        m_managers.texture_manager
            ->with(
                cmd.dst.get_handle(),
                [](const VulkanTexture& texture) {
                    return core::make_tuple(
                        texture.get_image(),
                        helpers::map_texture_aspect(
                            rhi::get_texture_format_desc(texture.get_format()).aspect));
                })
            .value_or_else([](const auto&) -> core::Tuple<VkImage, VkImageAspectFlags> {
                core::panic("`TextureCopyCommand::dst` is not alive.");
            });

    const VkImageLayout src_image_layout = helpers::map_access_flags_to_image_layout(
        cmd.src_texture_access);
    const VkImageLayout dst_image_layout = helpers::map_access_flags_to_image_layout(
        cmd.dst_texture_access);

    core::Array<VkImageCopy> regions;
    regions.reserve(cmd.regions.size());

    for (const rhi::TextureCopyRegion& region : cmd.regions) {
        regions.push_back(VkImageCopy {
            .srcSubresource =
                VkImageSubresourceLayers {
                    .aspectMask = src_image_aspect_flags,
                    .mipLevel = region.src_subresource.mip_level,
                    .baseArrayLayer = region.src_subresource.first_layer,
                    .layerCount = region.src_subresource.layer_count,
                },
            .srcOffset =
                VkOffset3D {
                    .x = region.src_offset.x,
                    .y = region.src_offset.y,
                    .z = region.src_offset.z,
                },
            .dstSubresource =
                VkImageSubresourceLayers {
                    .aspectMask = dst_image_aspect_flags,
                    .mipLevel = region.dst_subresource.mip_level,
                    .baseArrayLayer = region.dst_subresource.first_layer,
                    .layerCount = region.dst_subresource.layer_count,
                },
            .dstOffset =
                VkOffset3D {
                    .x = region.dst_offset.x,
                    .y = region.dst_offset.y,
                    .z = region.dst_offset.z,
                },
            .extent = helpers::map_extent(region.extent),
        });
    }

    m_loader_device.cmd_copy_image(
        m_bundle.command_buffer,
        src_image,
        src_image_layout,
        dst_image,
        dst_image_layout,
        core::as_span(regions));
}

void VulkanCommandDecoder::global_barrier(
    const rhi::commands::GlobalBarrierCommand& cmd) noexcept
{
    TNDR_PROFILER_TRACE_IF(PROFILE_DECODER, "VulkanCommandDecoder::global_barrier");

    m_barrier.global_barrier(cmd.barrier);
    m_barrier.execute(m_bundle.command_buffer);
    m_barrier.reset();
}

void VulkanCommandDecoder::texture_barrier(
    const rhi::commands::TextureBarrierCommand& cmd) noexcept
{
    TNDR_PROFILER_TRACE_IF(PROFILE_DECODER, "VulkanCommandDecoder::texture_barrier");

    for (const rhi::TextureBarrier& barrier : cmd.barriers) {
        m_resources->add_reference(
            *m_managers.resource_tracker, barrier.texture.get_handle().get_id());

        tndr_assert(
            m_managers.texture_manager
                ->with(
                    barrier.texture.get_handle(),
                    [&](const VulkanTexture& texture) {
                        m_barrier.texture_barrier(texture, barrier);
                    })
                .has_value(),
            "`TextureBarrier::texture` is not alive.");
    }

    m_barrier.execute(m_bundle.command_buffer);
    m_barrier.reset();
}

void VulkanCommandDecoder::buffer_texture_copy(
    const rhi::commands::BufferTextureCopyCommand& cmd) noexcept
{
    TNDR_PROFILER_TRACE_IF(PROFILE_DECODER, "VulkanCommandDecoder::buffer_texture_copy");

    m_resources->add_reference(
        *m_managers.resource_tracker, cmd.src.get_handle().get_id());
    m_resources->add_reference(
        *m_managers.resource_tracker, cmd.dst.get_handle().get_id());

    const VkBuffer src_buffer =
        m_managers.buffer_manager
            ->with(
                cmd.src.get_handle(),
                [](const VulkanBuffer& buffer) { return buffer.get_buffer(); })
            .value_or_else([](const auto&) -> VkBuffer {
                core::panic("`BufferTextureCopyCommand::src` is not alive.");
            });

    const auto [dst_image, dst_image_aspect_flags] =
        m_managers.texture_manager
            ->with(
                cmd.dst.get_handle(),
                [](const VulkanTexture& texture) {
                    return core::make_tuple(
                        texture.get_image(),
                        helpers::map_texture_aspect(
                            rhi::get_texture_format_desc(texture.get_format()).aspect));
                })
            .value_or_else([](const auto&) -> core::Tuple<VkImage, VkImageAspectFlags> {
                core::panic("`BufferTextureCopyCommand::dst` is not alive.");
            });

    const VkImageLayout dst_image_layout = helpers::map_access_flags_to_image_layout(
        cmd.texture_access);

    core::Array<VkBufferImageCopy> regions;
    regions.reserve(cmd.regions.size());

    for (const rhi::BufferTextureCopyRegion& region : cmd.regions) {
        regions.push_back(VkBufferImageCopy {
            .bufferOffset = region.buffer_offset,
            .bufferRowLength = region.buffer_width,
            .bufferImageHeight = region.buffer_height,
            .imageSubresource =
                VkImageSubresourceLayers {
                    .aspectMask = dst_image_aspect_flags,
                    .mipLevel = region.texture_subresource.mip_level,
                    .baseArrayLayer = region.texture_subresource.first_layer,
                    .layerCount = region.texture_subresource.layer_count,
                },
            .imageOffset = VkOffset3D {},
            .imageExtent = helpers::map_extent(region.texture_extent),
        });
    }

    m_loader_device.cmd_copy_buffer_to_image(
        m_bundle.command_buffer,
        src_buffer,
        dst_image,
        dst_image_layout,
        core::as_span(regions));
}

void VulkanCommandDecoder::texture_buffer_copy(
    const rhi::commands::TextureBufferCopyCommand& cmd) noexcept
{
    TNDR_PROFILER_TRACE_IF(PROFILE_DECODER, "VulkanCommandDecoder::texture_buffer_copy");

    m_resources->add_reference(
        *m_managers.resource_tracker, cmd.src.get_handle().get_id());
    m_resources->add_reference(
        *m_managers.resource_tracker, cmd.dst.get_handle().get_id());

    const auto [src_image, src_image_aspect_flags] =
        m_managers.texture_manager
            ->with(
                cmd.src.get_handle(),
                [](const VulkanTexture& texture) {
                    return core::make_tuple(
                        texture.get_image(),
                        helpers::map_texture_aspect(
                            rhi::get_texture_format_desc(texture.get_format()).aspect));
                })
            .value_or_else([](const auto&) -> core::Tuple<VkImage, VkImageAspectFlags> {
                core::panic("`BufferTextureCopyCommand::dst` is not alive.");
            });

    const VkImageLayout src_image_layout = helpers::map_access_flags_to_image_layout(
        cmd.texture_access);

    const VkBuffer dst_buffer =
        m_managers.buffer_manager
            ->with(
                cmd.dst.get_handle(),
                [](const VulkanBuffer& buffer) { return buffer.get_buffer(); })
            .value_or_else([](const auto&) -> VkBuffer {
                core::panic("`TextureBufferCopyCommand::dst` is not alive.");
            });

    core::Array<VkBufferImageCopy> regions;
    regions.reserve(cmd.regions.size());

    for (const rhi::BufferTextureCopyRegion& region : cmd.regions) {
        regions.push_back(VkBufferImageCopy {
            .bufferOffset = region.buffer_offset,
            .bufferRowLength = region.buffer_width,
            .bufferImageHeight = region.buffer_height,
            .imageSubresource =
                VkImageSubresourceLayers {
                    .aspectMask = src_image_aspect_flags,
                    .mipLevel = region.texture_subresource.mip_level,
                    .baseArrayLayer = region.texture_subresource.first_layer,
                    .layerCount = region.texture_subresource.layer_count,
                },
            .imageOffset = VkOffset3D {},
            .imageExtent = helpers::map_extent(region.texture_extent),
        });
    }

    m_loader_device.cmd_copy_image_to_buffer(
        m_bundle.command_buffer,
        src_image,
        src_image_layout,
        dst_buffer,
        core::as_span(regions));
}

void VulkanCommandDecoder::buffer_barrier(
    const rhi::commands::BufferBarrierCommand& cmd) noexcept
{
    TNDR_PROFILER_TRACE_IF(PROFILE_DECODER, "VulkanCommandDecoder::buffer_barrier");

    for (const rhi::BufferBarrier& barrier : cmd.barriers) {
        m_resources->add_reference(
            *m_managers.resource_tracker, barrier.buffer.get_handle().get_id());

        tndr_assert(
            m_managers.buffer_manager
                ->with(
                    barrier.buffer.get_handle(),
                    [&](const VulkanBuffer& buffer) {
                        m_barrier.buffer_barrier(buffer, barrier);
                    })
                .has_value(),
            "`TextureBarrier::texture` is not alive.");
    }

    m_barrier.execute(m_bundle.command_buffer);
    m_barrier.reset();
}

void VulkanCommandDecoder::bind_compute_pipeline(
    const rhi::ComputePipelineHandle pipeline) noexcept
{
    TNDR_PROFILER_TRACE_IF(
        PROFILE_DECODER, "VulkanCommandDecoder::bind_compute_pipeline");

    if (m_cache.current_compute_pipeline != pipeline) {
        m_resources->add_reference(
            *m_managers.resource_tracker, pipeline.get_handle().get_id());
        m_cache.current_compute_pipeline = pipeline;

        const VkPipeline vk_pipeline =
            m_managers.compute_pipeline_manager
                ->with(
                    pipeline.get_handle(),
                    [](const VulkanComputePipeline& pipeline) {
                        return pipeline.get_pipeline();
                    })
                .value_or_else([](const auto&) -> VkPipeline {
                    core::panic("`ComputePipelineHandle` is not alive.");
                });

        m_loader_device.cmd_bind_pipeline(
            m_bundle.command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, vk_pipeline);
    }
}

} // namespace tundra::vulkan_rhi
