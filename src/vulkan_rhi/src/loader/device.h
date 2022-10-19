#pragma once
#include "core/core.h"
#include "core/std/containers/array.h"
#include "core/std/expected.h"
#include "core/std/span.h"
#include "loader/device.h"
#include "vulkan_utils.h"

namespace tundra::vulkan_rhi::loader {

class Device {
private:
    VkDevice m_device;

    ///
    struct Table {
        // clang-format off
        PFN_vkGetDeviceProcAddr get_device_proc_addr;
        PFN_vkAllocateCommandBuffers allocate_command_buffers;
        PFN_vkAllocateDescriptorSets allocate_descriptor_sets;
        PFN_vkAllocateMemory allocate_memory;
        PFN_vkBeginCommandBuffer begin_command_buffer;
        PFN_vkBindBufferMemory bind_buffer_memory;
        PFN_vkBindImageMemory bind_image_memory;
        PFN_vkCmdBeginQuery cmd_begin_query;
        PFN_vkCmdBeginRenderPass cmd_begin_render_pass;
        PFN_vkCmdBeginRendering cmd_begin_rendering;
        PFN_vkCmdBindDescriptorSets cmd_bind_descriptor_sets;
        PFN_vkCmdBindIndexBuffer cmd_bind_index_buffer;
        PFN_vkCmdBindPipeline cmd_bind_pipeline;
        PFN_vkCmdBindVertexBuffers cmd_bind_vertex_buffers;
        PFN_vkCmdBlitImage cmd_blit_image;
        PFN_vkCmdClearAttachments cmd_clear_attachments;
        PFN_vkCmdClearColorImage cmd_clear_color_image;
        PFN_vkCmdClearDepthStencilImage cmd_clear_depth_stencil_image;
        PFN_vkCmdCopyBuffer cmd_copy_buffer;
        PFN_vkCmdCopyBufferToImage cmd_copy_buffer_to_image;
        PFN_vkCmdCopyImage cmd_copy_image;
        PFN_vkCmdCopyImageToBuffer cmd_copy_image_to_buffer;
        PFN_vkCmdCopyQueryPoolResults cmd_copy_query_pool_results;
        PFN_vkCmdDispatch cmd_dispatch;
        PFN_vkCmdDispatchIndirect cmd_dispatch_indirect;
        PFN_vkCmdDraw cmd_draw;
        PFN_vkCmdDrawIndexed cmd_draw_indexed;
        PFN_vkCmdDrawIndexedIndirect cmd_draw_indexed_indirect;
        PFN_vkCmdDrawIndirect cmd_draw_indirect;
        PFN_vkCmdEndQuery cmd_end_query;
        PFN_vkCmdEndRenderPass cmd_end_render_pass;
        PFN_vkCmdEndRendering cmd_end_rendering;
        PFN_vkCmdExecuteCommands cmd_execute_commands;
        PFN_vkCmdFillBuffer cmd_fill_buffer;
        PFN_vkCmdNextSubpass cmd_next_subpass;
        PFN_vkCmdPipelineBarrier cmd_pipeline_barrier;
        PFN_vkCmdPushConstants cmd_push_constants;
        PFN_vkCmdResetEvent cmd_reset_event;
        PFN_vkCmdResetQueryPool cmd_reset_query_pool;
        PFN_vkCmdResolveImage cmd_resolve_image;
        PFN_vkCmdSetBlendConstants cmd_set_blend_constants;
        PFN_vkCmdSetDepthBias cmd_set_depth_bias;
        PFN_vkCmdSetDepthBounds cmd_set_depth_bounds;
        PFN_vkCmdSetEvent cmd_set_event;
        PFN_vkCmdSetLineWidth cmd_set_line_width;
        PFN_vkCmdSetScissor cmd_set_scissor;
        PFN_vkCmdSetCullMode cmd_set_cull_mode;
        PFN_vkCmdSetStencilCompareMask cmd_set_stencil_compare_mask;
        PFN_vkCmdSetStencilReference cmd_set_stencil_reference;
        PFN_vkCmdSetStencilWriteMask cmd_set_stencil_write_mask;
        PFN_vkCmdSetViewport cmd_set_viewport;
        PFN_vkCmdUpdateBuffer cmd_update_buffer;
        PFN_vkCmdWaitEvents cmd_wait_events;
        PFN_vkCmdWriteTimestamp cmd_write_timestamp;
        PFN_vkCreateBuffer create_buffer;
        PFN_vkCreateBufferView create_buffer_view;
        PFN_vkCreateCommandPool create_command_pool;
        PFN_vkCreateComputePipelines create_compute_pipelines;
        PFN_vkCreateDescriptorPool create_descriptor_pool;
        PFN_vkCreateDescriptorSetLayout create_descriptor_set_layout;
        PFN_vkCreateEvent create_event;
        PFN_vkCreateFence create_fence;
        PFN_vkCreateFramebuffer create_framebuffer;
        PFN_vkCreateGraphicsPipelines create_graphics_pipelines;
        PFN_vkCreateImage create_image;
        PFN_vkCreateImageView create_image_view;
        PFN_vkCreatePipelineCache create_pipeline_cache;
        PFN_vkCreatePipelineLayout create_pipeline_layout;
        PFN_vkCreateQueryPool create_query_pool;
        PFN_vkCreateRenderPass create_render_pass;
        PFN_vkCreateSampler create_sampler;
        PFN_vkCreateSemaphore create_semaphore;
        PFN_vkCreateShaderModule create_shader_module;
        PFN_vkDestroyBuffer destroy_buffer;
        PFN_vkDestroyBufferView destroy_buffer_view;
        PFN_vkDestroyCommandPool destroy_command_pool;
        PFN_vkDestroyDescriptorPool destroy_descriptor_pool;
        PFN_vkDestroyDescriptorSetLayout destroy_descriptor_set_layout;
        PFN_vkDestroyDevice destroy_device;
        PFN_vkDestroyEvent destroy_event;
        PFN_vkDestroyFence destroy_fence;
        PFN_vkDestroyFramebuffer destroy_framebuffer;
        PFN_vkDestroyImage destroy_image;
        PFN_vkDestroyImageView destroy_image_view;
        PFN_vkDestroyPipeline destroy_pipeline;
        PFN_vkDestroyPipelineCache destroy_pipeline_cache;
        PFN_vkDestroyPipelineLayout destroy_pipeline_layout;
        PFN_vkDestroyQueryPool destroy_query_pool;
        PFN_vkDestroyRenderPass destroy_render_pass;
        PFN_vkDestroySampler destroy_sampler;
        PFN_vkDestroySemaphore destroy_semaphore;
        PFN_vkDestroyShaderModule destroy_shader_module;
        PFN_vkDeviceWaitIdle device_wait_idle;
        PFN_vkEndCommandBuffer end_command_buffer;
        PFN_vkFlushMappedMemoryRanges flush_mapped_memory_ranges;
        PFN_vkFreeCommandBuffers free_command_buffers;
        PFN_vkFreeDescriptorSets free_descriptor_sets;
        PFN_vkFreeMemory free_memory;
        PFN_vkGetBufferMemoryRequirements get_buffer_memory_requirements;
        PFN_vkGetDeviceMemoryCommitment get_device_memory_commitment;
        PFN_vkGetDeviceQueue get_device_queue;
        PFN_vkGetEventStatus get_event_status;
        PFN_vkGetFenceStatus get_fence_status;
        PFN_vkGetImageMemoryRequirements get_image_memory_requirements;
        PFN_vkGetImageSparseMemoryRequirements get_image_sparse_memory_requirements;
        PFN_vkGetImageSubresourceLayout get_image_subresource_layout;
        PFN_vkGetPipelineCacheData get_pipeline_cache_data;
        PFN_vkGetQueryPoolResults get_query_pool_results;
        PFN_vkGetRenderAreaGranularity get_render_area_granularity;
        PFN_vkInvalidateMappedMemoryRanges invalidate_mapped_memory_ranges;
        PFN_vkMapMemory map_memory;
        PFN_vkMergePipelineCaches merge_pipeline_caches;
        PFN_vkQueueBindSparse queue_bind_sparse;
        PFN_vkQueueSubmit queue_submit;
        PFN_vkQueueWaitIdle queue_wait_idle;
        PFN_vkResetCommandBuffer reset_command_buffer;
        PFN_vkResetCommandPool reset_command_pool;
        PFN_vkResetDescriptorPool reset_descriptor_pool;
        PFN_vkResetEvent reset_event;
        PFN_vkResetFences reset_fences;
        PFN_vkSetEvent set_event;
        PFN_vkUnmapMemory unmap_memory;
        PFN_vkUpdateDescriptorSets update_descriptor_sets;
        PFN_vkWaitForFences wait_for_fences;
        PFN_vkBindBufferMemory2 bind_buffer_memory2;
        PFN_vkBindImageMemory2 bind_image_memory2;
        PFN_vkCmdDispatchBase cmd_dispatch_base;
        PFN_vkCmdSetDeviceMask cmd_set_device_mask;
        PFN_vkCreateDescriptorUpdateTemplate create_descriptor_update_template;
        PFN_vkCreateSamplerYcbcrConversion create_sampler_ycbcr_conversion;
        PFN_vkDestroyDescriptorUpdateTemplate destroy_descriptor_update_template;
        PFN_vkDestroySamplerYcbcrConversion destroy_sampler_ycbcr_conversion;
        PFN_vkGetBufferMemoryRequirements2 get_buffer_memory_requirements2;
        PFN_vkGetDescriptorSetLayoutSupport get_descriptor_set_layout_support;
        PFN_vkGetDeviceGroupPeerMemoryFeatures get_device_group_peer_memory_features;
        PFN_vkGetDeviceQueue2 get_device_queue2;
        PFN_vkGetImageMemoryRequirements2 get_image_memory_requirements2;
        PFN_vkGetImageSparseMemoryRequirements2 get_image_sparse_memory_requirements2;
        PFN_vkTrimCommandPool trim_command_pool;
        PFN_vkUpdateDescriptorSetWithTemplate update_descriptor_set_with_template;
        PFN_vkCmdBeginRenderPass2 cmd_begin_render_pass2;
        PFN_vkCmdDrawIndexedIndirectCount cmd_draw_indexed_indirect_count;
        PFN_vkCmdDrawIndirectCount cmd_draw_indirect_count;
        PFN_vkCmdEndRenderPass2 cmd_end_render_pass2;
        PFN_vkCmdNextSubpass2 cmd_next_subpass2;
        PFN_vkCreateRenderPass2 create_render_pass2;
        PFN_vkGetBufferDeviceAddress get_buffer_device_address;
        PFN_vkGetBufferOpaqueCaptureAddress get_buffer_opaque_capture_address;
        PFN_vkGetDeviceMemoryOpaqueCaptureAddress get_device_memory_opaque_capture_address;
        PFN_vkGetSemaphoreCounterValue get_semaphore_counter_value;
        PFN_vkResetQueryPool reset_query_pool;
        PFN_vkSignalSemaphore signal_semaphore;
        PFN_vkWaitSemaphores wait_semaphores;
        PFN_vkGetDeviceBufferMemoryRequirements get_device_buffer_memory_requirements;
        PFN_vkGetDeviceImageMemoryRequirements get_device_image_memory_requirements;
        // clang-format on
    } m_table;

private:
    Device(const VkDevice device, PFN_vkGetDeviceProcAddr get_device_proc_addr) noexcept;

public:
    void destroy_device(const VkAllocationCallbacks* allocator) noexcept;

public:
    [[nodiscard]] core::Expected<core::Array<VkCommandBuffer>, VkResult>
        allocate_command_buffers(
            const VkCommandBufferAllocateInfo& allocate_info) const noexcept;

public:
    /// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkBeginCommandBuffer.html
    [[nodiscard]] core::Expected<void, VkResult> begin_command_buffer(
        const VkCommandBuffer command_buffer,
        const VkCommandBufferBeginInfo& begin_info) const noexcept;

    /// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkCmdBeginRenderPass.html
    void cmd_begin_render_pass(
        const VkCommandBuffer command_buffer,
        const VkRenderPassBeginInfo& begin_info,
        const VkSubpassContents contents) const noexcept;

    /// https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/vkCmdBeginRendering.html
    void cmd_begin_rendering(
        const VkCommandBuffer command_buffer,
        const VkRenderingInfo& rendering_info) const noexcept;

    /// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkCmdBindDescriptorSets.html
    void cmd_bind_descriptor_sets(
        const VkCommandBuffer command_buffer,
        const VkPipelineBindPoint pipeline_bind_point,
        const VkPipelineLayout layout,
        const u32 first_set,
        const core::Span<const VkDescriptorSet>& descriptor_sets,
        const core::Span<const u32>& dynamic_offsets) const noexcept;

    /// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkCmdBindIndexBuffer.html
    void cmd_bind_index_buffer(
        const VkCommandBuffer command_buffer,
        const VkBuffer buffer,
        const u64 offset,
        const VkIndexType index_type) const noexcept;

    /// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkCmdBindPipeline.html
    void cmd_bind_pipeline(
        const VkCommandBuffer command_buffer,
        const VkPipelineBindPoint pipeline_bind_point,
        const VkPipeline pipeline) const noexcept;

    /// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkCmdBlitImage.html
    void cmd_blit_image(
        const VkCommandBuffer command_buffer,
        const VkImage src_image,
        const VkImageLayout src_image_layout,
        const VkImage dst_image,
        const VkImageLayout dst_image_layout,
        const core::Span<const VkImageBlit>& regions,
        const VkFilter filter) const noexcept;

    /// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkCmdCopyBuffer.html
    void cmd_copy_buffer(
        const VkCommandBuffer command_buffer,
        const VkBuffer src_buffer,
        const VkBuffer dst_buffer,
        const core::Span<const VkBufferCopy>& regions) const noexcept;

    /// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkCmdCopyImageToBuffer.html
    void cmd_copy_image_to_buffer(
        const VkCommandBuffer command_buffer,
        const VkImage src_image,
        const VkImageLayout src_image_layout,
        const VkBuffer dst_buffer,
        const core::Span<const VkBufferImageCopy>& regions) const noexcept;

    /// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkCmdCopyBufferToImage.html
    void cmd_copy_buffer_to_image(
        const VkCommandBuffer command_buffer,
        const VkBuffer src_buffer,
        const VkImage dst_image,
        const VkImageLayout dst_image_layout,
        const core::Span<const VkBufferImageCopy>& regions) const noexcept;

    /// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkCmdCopyImage.html
    void cmd_copy_image(
        const VkCommandBuffer command_buffer,
        const VkImage src_image,
        const VkImageLayout src_image_layout,
        const VkImage dst_image,
        const VkImageLayout dst_image_layout,
        const core::Span<const VkImageCopy>& regions) const noexcept;

    /// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkCmdDispatch.html
    void cmd_dispatch(
        const VkCommandBuffer command_buffer,
        const u32 group_count_x,
        const u32 group_count_y,
        const u32 group_count_z) const noexcept;

    /// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkCmdDispatchIndirect.html
    void cmd_dispatch_indirect(
        const VkCommandBuffer command_buffer,
        const VkBuffer buffer,
        const u64 offset) const noexcept;

    /// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkCmdDraw.html
    void cmd_draw(
        const VkCommandBuffer command_buffer,
        const u32 vertex_count,
        const u32 instance_count,
        const u32 first_vertex,
        const u32 first_instance) const noexcept;

    /// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkCmdDrawIndexed.html
    void cmd_draw_indexed(
        const VkCommandBuffer command_buffer,
        const u32 index_count,
        const u32 instance_count,
        const u32 first_index,
        const i32 vertex_offset,
        const u32 first_instance) const noexcept;

    /// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkCmdDrawIndexedIndirect.html
    void cmd_draw_indexed_indirect(
        const VkCommandBuffer command_buffer,
        const VkBuffer buffer,
        const u64 offset,
        const u32 draw_count,
        const u32 stride) const noexcept;

    /// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkCmdEndRenderPass.html
    void cmd_end_render_pass(const VkCommandBuffer command_buffer) const noexcept;

    /// https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/vkCmdEndRendering.html
    void cmd_end_rendering(const VkCommandBuffer command_buffer) const noexcept;

    /// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkCmdPipelineBarrier.html
    void cmd_pipeline_barrier(
        const VkCommandBuffer command_buffer,
        const VkPipelineStageFlags src_stage_mask,
        const VkPipelineStageFlags dst_stage_mask,
        const VkDependencyFlags dependency_flags,
        const core::Span<const VkMemoryBarrier>& memory_barriers,
        const core::Span<const VkBufferMemoryBarrier>& buffer_memory_barriers,
        const core::Span<const VkImageMemoryBarrier>& image_memory_barriers)
        const noexcept;

    /// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkCmdPushConstants.html
    void cmd_push_constants(
        const VkCommandBuffer command_buffer,
        const VkPipelineLayout pipeline_layout,
        const VkShaderStageFlags shader_stage,
        const u32 offset,
        const core::Span<const char>& constants) const noexcept;

    /// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkCmdSetScissor.html
    void cmd_set_scissor(
        const VkCommandBuffer command_buffer,
        const u32 first_scissor,
        const core::Span<const VkRect2D>& scissors) const noexcept;

    /// https://khronos.org/registry/vulkan/specs/1.3-extensions/man/html/vkCmdSetCullModeEXT.html
    void cmd_set_cull_mode(
        const VkCommandBuffer command_buffer,
        const VkCullModeFlags cull_mode) const noexcept;

    /// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkCmdSetViewport.html
    void cmd_set_viewport(
        const VkCommandBuffer command_buffer,
        const u32 first_viewport,
        const core::Span<const VkViewport>& viewports) const noexcept;

    /// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkCmdDrawIndexedIndirectCount.html
    void cmd_draw_indexed_indirect_count(
        const VkCommandBuffer command_buffer,
        const VkBuffer buffer,
        const u64 offset,
        const VkBuffer count_buffer,
        const u64 count_buffer_offset,
        const u32 max_draw_count,
        const u32 stride) const noexcept;

public:
    /// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkAllocateDescriptorSets.html
    [[nodiscard]] core::Expected<core::Array<VkDescriptorSet>, VkResult>
        allocate_descriptor_sets(
            const VkDescriptorSetAllocateInfo& allocate_info) const noexcept;

    /// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkCreateCommandPool.html
    [[nodiscard]] core::Expected<VkCommandPool, VkResult> create_command_pool(
        const VkCommandPoolCreateInfo& create_info,
        const VkAllocationCallbacks* allocator) const noexcept;

    /// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkCreateFence.html
    [[nodiscard]] core::Expected<VkFence, VkResult> create_fence(
        const VkFenceCreateInfo& create_info,
        const VkAllocationCallbacks* allocator) const noexcept;

    /// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkCreateFramebuffer.html
    [[nodiscard]] core::Expected<VkFramebuffer, VkResult> create_framebuffer(
        const VkFramebufferCreateInfo& create_info,
        const VkAllocationCallbacks* allocator) const noexcept;

    /// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkCreateGraphicsPipelines.html
    [[nodiscard]] core::Expected<core::Array<VkPipeline>, VkResult>
        create_graphics_pipelines(
            const VkPipelineCache pipeline_cache,
            const core::Span<const VkGraphicsPipelineCreateInfo>& create_infos,
            const VkAllocationCallbacks* allocator) const noexcept;

    /// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkCreateImageView.html
    [[nodiscard]] core::Expected<VkImageView, VkResult> create_image_view(
        const VkImageViewCreateInfo& create_info,
        const VkAllocationCallbacks* allocator) const noexcept;

    /// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkCreatePipelineCache.html
    [[nodiscard]] core::Expected<VkPipelineCache, VkResult> create_pipeline_cache(
        const VkPipelineCacheCreateInfo& create_info,
        const VkAllocationCallbacks* allocator) const noexcept;

    /// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkCreatePipelineLayout.html
    [[nodiscard]] core::Expected<VkPipelineLayout, VkResult> create_pipeline_layout(
        const VkPipelineLayoutCreateInfo& create_info,
        const VkAllocationCallbacks* allocator) const noexcept;

    /// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkCreateDescriptorSetLayout.html
    [[nodiscard]] core::Expected<VkDescriptorSetLayout, VkResult>
        create_descriptor_set_layout(
            const VkDescriptorSetLayoutCreateInfo& create_info,
            const VkAllocationCallbacks* allocator) const noexcept;

    /// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkCreateSampler.html
    [[nodiscard]] core::Expected<VkSampler, VkResult> create_sampler(
        const VkSamplerCreateInfo& create_info,
        const VkAllocationCallbacks* allocator) const noexcept;

    /// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkCreateSemaphore.html
    [[nodiscard]] core::Expected<VkSemaphore, VkResult> create_semaphore(
        const VkSemaphoreCreateInfo& create_info,
        const VkAllocationCallbacks* allocator) const noexcept;

    /// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkCreateShaderModule.html
    [[nodiscard]] core::Expected<VkShaderModule, VkResult> create_shader_module(
        const VkShaderModuleCreateInfo& create_info,
        const VkAllocationCallbacks* allocator) const noexcept;

    /// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkCreateComputePipelines.html
    [[nodiscard]] core::Expected<core::Array<VkPipeline>, VkResult>
        create_compute_pipelines(
            const VkPipelineCache pipeline_cache,
            const core::Span<const VkComputePipelineCreateInfo>& create_infos,
            const VkAllocationCallbacks* allocator) const noexcept;

    /// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkCreateDescriptorPool.html
    [[nodiscard]] core::Expected<VkDescriptorPool, VkResult> create_descriptor_pool(
        const VkDescriptorPoolCreateInfo& create_info,
        const VkAllocationCallbacks* allocator) const noexcept;

    /// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkDestroyComandPool.html
    void destroy_command_pool(
        const VkCommandPool pool, const VkAllocationCallbacks* allocator) const noexcept;

    /// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkDestroyDescriptorPool.html
    void destroy_descriptor_pool(
        const VkDescriptorPool pool,
        const VkAllocationCallbacks* allocator) const noexcept;

    /// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkDestroyDescriptorSetLayout.html
    void destroy_descriptor_set_layout(
        const VkDescriptorSetLayout layout,
        const VkAllocationCallbacks* allocator) const noexcept;

    /// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkDestroyFence.html
    void destroy_fence(
        const VkFence fence, const VkAllocationCallbacks* allocator) const noexcept;

    /// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkDestroyFramebuffer.html
    void destroy_framebuffer(
        const VkFramebuffer framebuffer,
        const VkAllocationCallbacks* allocator) const noexcept;

    /// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkDestroyImageView.html
    void destroy_image_view(
        const VkImageView image_view,
        const VkAllocationCallbacks* allocator) const noexcept;

    /// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkDestroyPipeline.html
    void destroy_pipeline(
        const VkPipeline pipeline, const VkAllocationCallbacks* allocator) const noexcept;

    /// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkDestroyPipelineCache.html
    void destroy_pipeline_cache(
        const VkPipelineCache pipeline_cache,
        const VkAllocationCallbacks* allocator) const noexcept;

    /// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkDestroySampler.html
    void destroy_sampler(
        const VkSampler sampler, const VkAllocationCallbacks* allocator) const noexcept;

    /// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkDestroySemaphore.html
    void destroy_semaphore(
        const VkSemaphore semaphore,
        const VkAllocationCallbacks* allocator) const noexcept;

    /// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkDestroyShaderModule.html
    void destroy_shader_module(
        const VkShaderModule shader_module,
        const VkAllocationCallbacks* allocator) const noexcept;

    [[nodiscard]] core::Expected<void, VkResult> end_command_buffer(
        const VkCommandBuffer command_buffer) const noexcept;

    /// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkDestroyPipelineLayout.html
    void destroy_pipeline_layout(
        const VkPipelineLayout layout,
        const VkAllocationCallbacks* allocator) const noexcept;

    /// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkDestroyRenderPass.html
    void destroy_render_pass(
        const VkRenderPass render_pass,
        const VkAllocationCallbacks* allocator) const noexcept;

    /// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkDeviceWaitIdle.html
    [[nodiscard]] core::Expected<void, VkResult> device_wait_idle() const noexcept;

    /// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkGetDeviceQueue.html
    [[nodiscard]] VkQueue get_device_queue(
        const u32 queue_family_index, const u32 queue_index) const noexcept;

    /// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkUpdateDescriptorSets.html
    void update_descriptor_sets(
        const core::Span<const VkWriteDescriptorSet>& descriptor_writes,
        const core::Span<const VkCopyDescriptorSet>& descriptor_copies) const noexcept;

    /// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkGetPipelineCacheData.html
    [[nodiscard]] core::Expected<core::Array<char>, VkResult> get_pipeline_cache_data(
        const VkPipelineCache pipeline_cache) const noexcept;

    /// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkWaitForFences.html
    [[nodiscard]] core::Expected<void, VkResult> wait_for_fences(
        const core::Span<const VkFence>& fences,
        const bool wait_all,
        const u64 timeout) const noexcept;

    /// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkQueueSubmit.html
    [[nodiscard]] core::Expected<void, VkResult> queue_submit(
        const VkQueue queue,
        const core::Span<const VkSubmitInfo>& submit_infos,
        const VkFence fence) const noexcept;

    /// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkResetCommandBuffer.html
    [[nodiscard]] core::Expected<void, VkResult> reset_command_pool(
        const VkCommandPool command_pool,
        const VkCommandBufferResetFlags flags) const noexcept;

    /// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkResetFences.html
    [[nodiscard]] core::Expected<void, VkResult> reset_fences(
        const core::Span<const VkFence>& fences) const noexcept;

    /// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkCreateRenderPass2.html
    [[nodiscard]] core::Expected<VkRenderPass, VkResult> create_render_pass2(
        const VkRenderPassCreateInfo2& create_info,
        const VkAllocationCallbacks* allocator) const noexcept;

public:
    [[nodiscard]] VkDevice get_handle() const noexcept;
    [[nodiscard]] const Table& get_table() const noexcept;

private:
    friend class Instance;
};

} // namespace tundra::vulkan_rhi::loader
