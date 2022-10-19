#include "loader/device.h"

namespace tundra::vulkan_rhi::loader {

Device::Device(
    const VkDevice device, PFN_vkGetDeviceProcAddr get_device_proc_addr) noexcept
    : m_device(device)
{
    const auto load = [device, get_device_proc_addr](const char* name) {
        return get_device_proc_addr(device, name);
    };

    // clang-format off
    m_table.get_device_proc_addr = get_device_proc_addr;
    m_table.allocate_command_buffers = reinterpret_cast<PFN_vkAllocateCommandBuffers>(load("vkAllocateCommandBuffers"));
    m_table.allocate_descriptor_sets = reinterpret_cast<PFN_vkAllocateDescriptorSets>(load("vkAllocateDescriptorSets"));
    m_table.allocate_memory = reinterpret_cast<PFN_vkAllocateMemory>(load("vkAllocateMemory"));
    m_table.begin_command_buffer = reinterpret_cast<PFN_vkBeginCommandBuffer>(load("vkBeginCommandBuffer"));
    m_table.bind_buffer_memory = reinterpret_cast<PFN_vkBindBufferMemory>(load("vkBindBufferMemory"));
    m_table.bind_image_memory = reinterpret_cast<PFN_vkBindImageMemory>(load("vkBindImageMemory"));
    m_table.cmd_begin_query = reinterpret_cast<PFN_vkCmdBeginQuery>(load("vkCmdBeginQuery"));
    m_table.cmd_begin_render_pass = reinterpret_cast<PFN_vkCmdBeginRenderPass>(load("vkCmdBeginRenderPass"));
    m_table.cmd_begin_rendering = reinterpret_cast<PFN_vkCmdBeginRendering>(load("vkCmdBeginRendering"));
    m_table.cmd_bind_descriptor_sets = reinterpret_cast<PFN_vkCmdBindDescriptorSets>(load("vkCmdBindDescriptorSets"));
    m_table.cmd_bind_index_buffer = reinterpret_cast<PFN_vkCmdBindIndexBuffer>(load("vkCmdBindIndexBuffer"));
    m_table.cmd_bind_pipeline = reinterpret_cast<PFN_vkCmdBindPipeline>(load("vkCmdBindPipeline"));
    m_table.cmd_bind_vertex_buffers = reinterpret_cast<PFN_vkCmdBindVertexBuffers>(load("vkCmdBindVertexBuffers"));
    m_table.cmd_blit_image = reinterpret_cast<PFN_vkCmdBlitImage>(load("vkCmdBlitImage"));
    m_table.cmd_clear_attachments = reinterpret_cast<PFN_vkCmdClearAttachments>(load("vkCmdClearAttachments"));
    m_table.cmd_clear_color_image = reinterpret_cast<PFN_vkCmdClearColorImage>(load("vkCmdClearColorImage"));
    m_table.cmd_clear_depth_stencil_image = reinterpret_cast<PFN_vkCmdClearDepthStencilImage>(load("vkCmdClearDepthStencilImage"));
    m_table.cmd_copy_buffer = reinterpret_cast<PFN_vkCmdCopyBuffer>(load("vkCmdCopyBuffer"));
    m_table.cmd_copy_buffer_to_image = reinterpret_cast<PFN_vkCmdCopyBufferToImage>(load("vkCmdCopyBufferToImage"));
    m_table.cmd_copy_image = reinterpret_cast<PFN_vkCmdCopyImage>(load("vkCmdCopyImage"));
    m_table.cmd_copy_image_to_buffer = reinterpret_cast<PFN_vkCmdCopyImageToBuffer>(load("vkCmdCopyImageToBuffer"));
    m_table.cmd_copy_query_pool_results = reinterpret_cast<PFN_vkCmdCopyQueryPoolResults>(load("vkCmdCopyQueryPoolResults"));
    m_table.cmd_dispatch = reinterpret_cast<PFN_vkCmdDispatch>(load("vkCmdDispatch"));
    m_table.cmd_dispatch_indirect = reinterpret_cast<PFN_vkCmdDispatchIndirect>(load("vkCmdDispatchIndirect"));
    m_table.cmd_draw = reinterpret_cast<PFN_vkCmdDraw>(load("vkCmdDraw"));
    m_table.cmd_draw_indexed = reinterpret_cast<PFN_vkCmdDrawIndexed>(load("vkCmdDrawIndexed"));
    m_table.cmd_draw_indexed_indirect = reinterpret_cast<PFN_vkCmdDrawIndexedIndirect>(load("vkCmdDrawIndexedIndirect"));
    m_table.cmd_draw_indirect = reinterpret_cast<PFN_vkCmdDrawIndirect>(load("vkCmdDrawIndirect"));
    m_table.cmd_end_query = reinterpret_cast<PFN_vkCmdEndQuery>(load("vkCmdEndQuery"));
    m_table.cmd_end_render_pass = reinterpret_cast<PFN_vkCmdEndRenderPass>(load("vkCmdEndRenderPass"));
    m_table.cmd_end_rendering = reinterpret_cast<PFN_vkCmdEndRendering>(load("vkCmdEndRendering"));
    m_table.cmd_execute_commands = reinterpret_cast<PFN_vkCmdExecuteCommands>(load("vkCmdExecuteCommands"));
    m_table.cmd_fill_buffer = reinterpret_cast<PFN_vkCmdFillBuffer>(load("vkCmdFillBuffer"));
    m_table.cmd_next_subpass = reinterpret_cast<PFN_vkCmdNextSubpass>(load("vkCmdNextSubpass"));
    m_table.cmd_pipeline_barrier = reinterpret_cast<PFN_vkCmdPipelineBarrier>(load("vkCmdPipelineBarrier"));
    m_table.cmd_push_constants = reinterpret_cast<PFN_vkCmdPushConstants>(load("vkCmdPushConstants"));
    m_table.cmd_reset_event = reinterpret_cast<PFN_vkCmdResetEvent>(load("vkCmdResetEvent"));
    m_table.cmd_reset_query_pool = reinterpret_cast<PFN_vkCmdResetQueryPool>(load("vkCmdResetQueryPool"));
    m_table.cmd_resolve_image = reinterpret_cast<PFN_vkCmdResolveImage>(load("vkCmdResolveImage"));
    m_table.cmd_set_blend_constants = reinterpret_cast<PFN_vkCmdSetBlendConstants>(load("vkCmdSetBlendConstants"));
    m_table.cmd_set_depth_bias = reinterpret_cast<PFN_vkCmdSetDepthBias>(load("vkCmdSetDepthBias"));
    m_table.cmd_set_depth_bounds = reinterpret_cast<PFN_vkCmdSetDepthBounds>(load("vkCmdSetDepthBounds"));
    m_table.cmd_set_event = reinterpret_cast<PFN_vkCmdSetEvent>(load("vkCmdSetEvent"));
    m_table.cmd_set_line_width = reinterpret_cast<PFN_vkCmdSetLineWidth>(load("vkCmdSetLineWidth"));
    m_table.cmd_set_scissor = reinterpret_cast<PFN_vkCmdSetScissor>(load("vkCmdSetScissor"));
    m_table.cmd_set_cull_mode = reinterpret_cast<PFN_vkCmdSetCullMode>(load("vkCmdSetCullMode"));
    m_table.cmd_set_stencil_compare_mask = reinterpret_cast<PFN_vkCmdSetStencilCompareMask>(load("vkCmdSetStencilCompareMask"));
    m_table.cmd_set_stencil_reference = reinterpret_cast<PFN_vkCmdSetStencilReference>(load("vkCmdSetStencilReference"));
    m_table.cmd_set_stencil_write_mask = reinterpret_cast<PFN_vkCmdSetStencilWriteMask>(load("vkCmdSetStencilWriteMask"));
    m_table.cmd_set_viewport = reinterpret_cast<PFN_vkCmdSetViewport>(load("vkCmdSetViewport"));
    m_table.cmd_update_buffer = reinterpret_cast<PFN_vkCmdUpdateBuffer>(load("vkCmdUpdateBuffer"));
    m_table.cmd_wait_events = reinterpret_cast<PFN_vkCmdWaitEvents>(load("vkCmdWaitEvents"));
    m_table.cmd_write_timestamp = reinterpret_cast<PFN_vkCmdWriteTimestamp>(load("vkCmdWriteTimestamp"));
    m_table.create_buffer = reinterpret_cast<PFN_vkCreateBuffer>(load("vkCreateBuffer"));
    m_table.create_buffer_view = reinterpret_cast<PFN_vkCreateBufferView>(load("vkCreateBufferView"));
    m_table.create_command_pool = reinterpret_cast<PFN_vkCreateCommandPool>(load("vkCreateCommandPool"));
    m_table.create_compute_pipelines = reinterpret_cast<PFN_vkCreateComputePipelines>(load("vkCreateComputePipelines"));
    m_table.create_descriptor_pool = reinterpret_cast<PFN_vkCreateDescriptorPool>(load("vkCreateDescriptorPool"));
    m_table.create_descriptor_set_layout = reinterpret_cast<PFN_vkCreateDescriptorSetLayout>(load("vkCreateDescriptorSetLayout"));
    m_table.create_event = reinterpret_cast<PFN_vkCreateEvent>(load("vkCreateEvent"));
    m_table.create_fence = reinterpret_cast<PFN_vkCreateFence>(load("vkCreateFence"));
    m_table.create_framebuffer = reinterpret_cast<PFN_vkCreateFramebuffer>(load("vkCreateFramebuffer"));
    m_table.create_graphics_pipelines = reinterpret_cast<PFN_vkCreateGraphicsPipelines>(load("vkCreateGraphicsPipelines"));
    m_table.create_image = reinterpret_cast<PFN_vkCreateImage>(load("vkCreateImage"));
    m_table.create_image_view = reinterpret_cast<PFN_vkCreateImageView>(load("vkCreateImageView"));
    m_table.create_pipeline_cache = reinterpret_cast<PFN_vkCreatePipelineCache>(load("vkCreatePipelineCache"));
    m_table.create_pipeline_layout = reinterpret_cast<PFN_vkCreatePipelineLayout>(load("vkCreatePipelineLayout"));
    m_table.create_query_pool = reinterpret_cast<PFN_vkCreateQueryPool>(load("vkCreateQueryPool"));
    m_table.create_render_pass = reinterpret_cast<PFN_vkCreateRenderPass>(load("vkCreateRenderPass"));
    m_table.create_sampler = reinterpret_cast<PFN_vkCreateSampler>(load("vkCreateSampler"));
    m_table.create_semaphore = reinterpret_cast<PFN_vkCreateSemaphore>(load("vkCreateSemaphore"));
    m_table.create_shader_module = reinterpret_cast<PFN_vkCreateShaderModule>(load("vkCreateShaderModule"));
    m_table.destroy_buffer = reinterpret_cast<PFN_vkDestroyBuffer>(load("vkDestroyBuffer"));
    m_table.destroy_buffer_view = reinterpret_cast<PFN_vkDestroyBufferView>(load("vkDestroyBufferView"));
    m_table.destroy_command_pool = reinterpret_cast<PFN_vkDestroyCommandPool>(load("vkDestroyCommandPool"));
    m_table.destroy_descriptor_pool = reinterpret_cast<PFN_vkDestroyDescriptorPool>(load("vkDestroyDescriptorPool"));
    m_table.destroy_descriptor_set_layout = reinterpret_cast<PFN_vkDestroyDescriptorSetLayout>(load("vkDestroyDescriptorSetLayout"));
    m_table.destroy_device = reinterpret_cast<PFN_vkDestroyDevice>(load("vkDestroyDevice"));
    m_table.destroy_event = reinterpret_cast<PFN_vkDestroyEvent>(load("vkDestroyEvent"));
    m_table.destroy_fence = reinterpret_cast<PFN_vkDestroyFence>(load("vkDestroyFence"));
    m_table.destroy_framebuffer = reinterpret_cast<PFN_vkDestroyFramebuffer>(load("vkDestroyFramebuffer"));
    m_table.destroy_image = reinterpret_cast<PFN_vkDestroyImage>(load("vkDestroyImage"));
    m_table.destroy_image_view = reinterpret_cast<PFN_vkDestroyImageView>(load("vkDestroyImageView"));
    m_table.destroy_pipeline = reinterpret_cast<PFN_vkDestroyPipeline>(load("vkDestroyPipeline"));
    m_table.destroy_pipeline_cache = reinterpret_cast<PFN_vkDestroyPipelineCache>(load("vkDestroyPipelineCache"));
    m_table.destroy_pipeline_layout = reinterpret_cast<PFN_vkDestroyPipelineLayout>(load("vkDestroyPipelineLayout"));
    m_table.destroy_query_pool = reinterpret_cast<PFN_vkDestroyQueryPool>(load("vkDestroyQueryPool"));
    m_table.destroy_render_pass = reinterpret_cast<PFN_vkDestroyRenderPass>(load("vkDestroyRenderPass"));
    m_table.destroy_sampler = reinterpret_cast<PFN_vkDestroySampler>(load("vkDestroySampler"));
    m_table.destroy_semaphore = reinterpret_cast<PFN_vkDestroySemaphore>(load("vkDestroySemaphore"));
    m_table.destroy_shader_module = reinterpret_cast<PFN_vkDestroyShaderModule>(load("vkDestroyShaderModule"));
    m_table.device_wait_idle = reinterpret_cast<PFN_vkDeviceWaitIdle>(load("vkDeviceWaitIdle"));
    m_table.end_command_buffer = reinterpret_cast<PFN_vkEndCommandBuffer>(load("vkEndCommandBuffer"));
    m_table.flush_mapped_memory_ranges = reinterpret_cast<PFN_vkFlushMappedMemoryRanges>(load("vkFlushMappedMemoryRanges"));
    m_table.free_command_buffers = reinterpret_cast<PFN_vkFreeCommandBuffers>(load("vkFreeCommandBuffers"));
    m_table.free_descriptor_sets = reinterpret_cast<PFN_vkFreeDescriptorSets>(load("vkFreeDescriptorSets"));
    m_table.free_memory = reinterpret_cast<PFN_vkFreeMemory>(load("vkFreeMemory"));
    m_table.get_buffer_memory_requirements = reinterpret_cast<PFN_vkGetBufferMemoryRequirements>(load("vkGetBufferMemoryRequirements"));
    m_table.get_device_memory_commitment = reinterpret_cast<PFN_vkGetDeviceMemoryCommitment>(load("vkGetDeviceMemoryCommitment"));
    m_table.get_device_queue = reinterpret_cast<PFN_vkGetDeviceQueue>(load("vkGetDeviceQueue"));
    m_table.get_event_status = reinterpret_cast<PFN_vkGetEventStatus>(load("vkGetEventStatus"));
    m_table.get_fence_status = reinterpret_cast<PFN_vkGetFenceStatus>(load("vkGetFenceStatus"));
    m_table.get_image_memory_requirements = reinterpret_cast<PFN_vkGetImageMemoryRequirements>(load("vkGetImageMemoryRequirements"));
    m_table.get_image_sparse_memory_requirements = reinterpret_cast<PFN_vkGetImageSparseMemoryRequirements>(load("vkGetImageSparseMemoryRequirements"));
    m_table.get_image_subresource_layout = reinterpret_cast<PFN_vkGetImageSubresourceLayout>(load("vkGetImageSubresourceLayout"));
    m_table.get_pipeline_cache_data = reinterpret_cast<PFN_vkGetPipelineCacheData>(load("vkGetPipelineCacheData"));
    m_table.get_query_pool_results = reinterpret_cast<PFN_vkGetQueryPoolResults>(load("vkGetQueryPoolResults"));
    m_table.get_render_area_granularity = reinterpret_cast<PFN_vkGetRenderAreaGranularity>(load("vkGetRenderAreaGranularity"));
    m_table.invalidate_mapped_memory_ranges = reinterpret_cast<PFN_vkInvalidateMappedMemoryRanges>(load("vkInvalidateMappedMemoryRanges"));
    m_table.map_memory = reinterpret_cast<PFN_vkMapMemory>(load("vkMapMemory"));
    m_table.merge_pipeline_caches = reinterpret_cast<PFN_vkMergePipelineCaches>(load("vkMergePipelineCaches"));
    m_table.queue_bind_sparse = reinterpret_cast<PFN_vkQueueBindSparse>(load("vkQueueBindSparse"));
    m_table.queue_submit = reinterpret_cast<PFN_vkQueueSubmit>(load("vkQueueSubmit"));
    m_table.queue_wait_idle = reinterpret_cast<PFN_vkQueueWaitIdle>(load("vkQueueWaitIdle"));
    m_table.reset_command_buffer = reinterpret_cast<PFN_vkResetCommandBuffer>(load("vkResetCommandBuffer"));
    m_table.reset_command_pool = reinterpret_cast<PFN_vkResetCommandPool>(load("vkResetCommandPool"));
    m_table.reset_descriptor_pool = reinterpret_cast<PFN_vkResetDescriptorPool>(load("vkResetDescriptorPool"));
    m_table.reset_event = reinterpret_cast<PFN_vkResetEvent>(load("vkResetEvent"));
    m_table.reset_fences = reinterpret_cast<PFN_vkResetFences>(load("vkResetFences"));
    m_table.set_event = reinterpret_cast<PFN_vkSetEvent>(load("vkSetEvent"));
    m_table.unmap_memory = reinterpret_cast<PFN_vkUnmapMemory>(load("vkUnmapMemory"));
    m_table.update_descriptor_sets = reinterpret_cast<PFN_vkUpdateDescriptorSets>(load("vkUpdateDescriptorSets"));
    m_table.wait_for_fences = reinterpret_cast<PFN_vkWaitForFences>(load("vkWaitForFences"));
    m_table.bind_buffer_memory2 = reinterpret_cast<PFN_vkBindBufferMemory2>(load("vkBindBufferMemory2"));
    m_table.bind_image_memory2 = reinterpret_cast<PFN_vkBindImageMemory2>(load("vkBindImageMemory2"));
    m_table.cmd_dispatch_base = reinterpret_cast<PFN_vkCmdDispatchBase>(load("vkCmdDispatchBase"));
    m_table.cmd_set_device_mask = reinterpret_cast<PFN_vkCmdSetDeviceMask>(load("vkCmdSetDeviceMask"));
    m_table.create_descriptor_update_template = reinterpret_cast<PFN_vkCreateDescriptorUpdateTemplate>(load("vkCreateDescriptorUpdateTemplate"));
    m_table.create_sampler_ycbcr_conversion = reinterpret_cast<PFN_vkCreateSamplerYcbcrConversion>(load("vkCreateSamplerYcbcrConversion"));
    m_table.destroy_descriptor_update_template = reinterpret_cast<PFN_vkDestroyDescriptorUpdateTemplate>(load("vkDestroyDescriptorUpdateTemplate"));
    m_table.destroy_sampler_ycbcr_conversion = reinterpret_cast<PFN_vkDestroySamplerYcbcrConversion>(load("vkDestroySamplerYcbcrConversion"));
    m_table.get_buffer_memory_requirements2 = reinterpret_cast<PFN_vkGetBufferMemoryRequirements2>(load("vkGetBufferMemoryRequirements2"));
    m_table.get_descriptor_set_layout_support = reinterpret_cast<PFN_vkGetDescriptorSetLayoutSupport>(load("vkGetDescriptorSetLayoutSupport"));
    m_table.get_device_group_peer_memory_features = reinterpret_cast<PFN_vkGetDeviceGroupPeerMemoryFeatures>(load("vkGetDeviceGroupPeerMemoryFeatures"));
    m_table.get_device_queue2 = reinterpret_cast<PFN_vkGetDeviceQueue2>(load("vkGetDeviceQueue2"));
    m_table.get_image_memory_requirements2 = reinterpret_cast<PFN_vkGetImageMemoryRequirements2>(load("vkGetImageMemoryRequirements2"));
    m_table.get_image_sparse_memory_requirements2 = reinterpret_cast<PFN_vkGetImageSparseMemoryRequirements2>(load("vkGetImageSparseMemoryRequirements2"));
    m_table.trim_command_pool = reinterpret_cast<PFN_vkTrimCommandPool>(load("vkTrimCommandPool"));
    m_table.update_descriptor_set_with_template = reinterpret_cast<PFN_vkUpdateDescriptorSetWithTemplate>(load("vkUpdateDescriptorSetWithTemplate"));
    m_table.cmd_begin_render_pass2 = reinterpret_cast<PFN_vkCmdBeginRenderPass2>(load("vkCmdBeginRenderPass2"));
    m_table.cmd_draw_indexed_indirect_count = reinterpret_cast<PFN_vkCmdDrawIndexedIndirectCount>(load("vkCmdDrawIndexedIndirectCount"));
    m_table.cmd_draw_indirect_count = reinterpret_cast<PFN_vkCmdDrawIndirectCount>(load("vkCmdDrawIndirectCount"));
    m_table.cmd_end_render_pass2 = reinterpret_cast<PFN_vkCmdEndRenderPass2>(load("vkCmdEndRenderPass2"));
    m_table.cmd_next_subpass2 = reinterpret_cast<PFN_vkCmdNextSubpass2>(load("vkCmdNextSubpass2"));
    m_table.create_render_pass2 = reinterpret_cast<PFN_vkCreateRenderPass2>(load("vkCreateRenderPass2"));
    m_table.get_buffer_device_address = reinterpret_cast<PFN_vkGetBufferDeviceAddress>(load("vkGetBufferDeviceAddress"));
    m_table.get_buffer_opaque_capture_address = reinterpret_cast<PFN_vkGetBufferOpaqueCaptureAddress>(load("vkGetBufferOpaqueCaptureAddress"));
    m_table.get_device_memory_opaque_capture_address = reinterpret_cast<PFN_vkGetDeviceMemoryOpaqueCaptureAddress>(load("vkGetDeviceMemoryOpaqueCaptureAddress"));
    m_table.get_semaphore_counter_value = reinterpret_cast<PFN_vkGetSemaphoreCounterValue>(load("vkGetSemaphoreCounterValue"));
    m_table.reset_query_pool = reinterpret_cast<PFN_vkResetQueryPool>(load("vkResetQueryPool"));
    m_table.signal_semaphore = reinterpret_cast<PFN_vkSignalSemaphore>(load("vkSignalSemaphore"));
    m_table.wait_semaphores = reinterpret_cast<PFN_vkWaitSemaphores>(load("vkWaitSemaphores"));
    m_table.get_device_buffer_memory_requirements = reinterpret_cast<PFN_vkGetDeviceBufferMemoryRequirements>(load("vkGetDeviceBufferMemoryRequirements"));
    m_table.get_device_image_memory_requirements = reinterpret_cast<PFN_vkGetDeviceImageMemoryRequirements>(load("vkGetDeviceImageMemoryRequirements"));
    // clang-format on
}

void Device::destroy_device(const VkAllocationCallbacks* allocator) noexcept
{
    m_table.destroy_device(m_device, allocator);
}

core::Expected<core::Array<VkCommandBuffer>, VkResult> Device::allocate_command_buffers(
    const VkCommandBufferAllocateInfo& allocate_info) const noexcept
{
    core::Array<VkCommandBuffer> command_buffers(allocate_info.commandBufferCount);
    const VkResult result = m_table.allocate_command_buffers(
        m_device, &allocate_info, command_buffers.data());

    if (result == VK_SUCCESS) {
        return core::move(command_buffers);
    } else {
        return core::make_unexpected(result);
    }
}

core::Expected<void, VkResult> Device::begin_command_buffer(
    const VkCommandBuffer command_buffer,
    const VkCommandBufferBeginInfo& begin_info) const noexcept
{
    const VkResult result = m_table.begin_command_buffer(command_buffer, &begin_info);
    if (result == VK_SUCCESS) {
        return {};
    } else {
        return core::make_unexpected(result);
    }
}

void Device::cmd_begin_render_pass(
    const VkCommandBuffer command_buffer,
    const VkRenderPassBeginInfo& begin_info,
    const VkSubpassContents contents) const noexcept
{
    m_table.cmd_begin_render_pass(command_buffer, &begin_info, contents);
}

void Device::cmd_begin_rendering(
    const VkCommandBuffer command_buffer,
    const VkRenderingInfo& rendering_info) const noexcept
{
    m_table.cmd_begin_rendering(command_buffer, &rendering_info);
}

void Device::cmd_bind_descriptor_sets(
    const VkCommandBuffer command_buffer,
    const VkPipelineBindPoint pipeline_bind_point,
    const VkPipelineLayout layout,
    const u32 first_set,
    const core::Span<const VkDescriptorSet>& descriptor_sets,
    const core::Span<const u32>& dynamic_offsets) const noexcept
{
    m_table.cmd_bind_descriptor_sets(
        command_buffer,
        pipeline_bind_point,
        layout,
        first_set,
        static_cast<u32>(descriptor_sets.size()),
        descriptor_sets.data(),
        static_cast<u32>(dynamic_offsets.size()),
        dynamic_offsets.data());
}

void Device::cmd_bind_index_buffer(
    const VkCommandBuffer command_buffer,
    const VkBuffer buffer,
    const u64 offset,
    const VkIndexType index_type) const noexcept
{
    m_table.cmd_bind_index_buffer(command_buffer, buffer, offset, index_type);
}

void Device::cmd_bind_pipeline(
    const VkCommandBuffer command_buffer,
    const VkPipelineBindPoint pipeline_bind_point,
    const VkPipeline pipeline) const noexcept
{
    m_table.cmd_bind_pipeline(command_buffer, pipeline_bind_point, pipeline);
}

void Device::cmd_blit_image(
    const VkCommandBuffer command_buffer,
    const VkImage src_image,
    const VkImageLayout src_image_layout,
    const VkImage dst_image,
    const VkImageLayout dst_image_layout,
    const core::Span<const VkImageBlit>& regions,
    const VkFilter filter) const noexcept
{
    m_table.cmd_blit_image(
        command_buffer,
        src_image,
        src_image_layout,
        dst_image,
        dst_image_layout,
        static_cast<u32>(regions.size()),
        regions.data(),
        filter);
}

void Device::cmd_copy_buffer(
    const VkCommandBuffer command_buffer,
    const VkBuffer src_buffer,
    const VkBuffer dst_buffer,
    const core::Span<const VkBufferCopy>& regions) const noexcept
{
    m_table.cmd_copy_buffer(
        command_buffer,
        src_buffer,
        dst_buffer,
        static_cast<u32>(regions.size()),
        regions.data());
}

void Device::cmd_copy_image_to_buffer(
    const VkCommandBuffer command_buffer,
    const VkImage src_image,
    const VkImageLayout src_image_layout,
    const VkBuffer dst_buffer,
    const core::Span<const VkBufferImageCopy>& regions) const noexcept
{
    m_table.cmd_copy_image_to_buffer(
        command_buffer,
        src_image,
        src_image_layout,
        dst_buffer,
        static_cast<u32>(regions.size()),
        regions.data());
}

void Device::cmd_copy_buffer_to_image(
    const VkCommandBuffer command_buffer,
    const VkBuffer src_buffer,
    const VkImage dst_image,
    const VkImageLayout dst_image_layout,
    const core::Span<const VkBufferImageCopy>& regions) const noexcept
{
    m_table.cmd_copy_buffer_to_image(
        command_buffer,
        src_buffer,
        dst_image,
        dst_image_layout,
        static_cast<u32>(regions.size()),
        regions.data());
}

void Device::cmd_copy_image(
    const VkCommandBuffer command_buffer,
    const VkImage src_image,
    const VkImageLayout src_image_layout,
    const VkImage dst_image,
    const VkImageLayout dst_image_layout,
    const core::Span<const VkImageCopy>& regions) const noexcept
{
    m_table.cmd_copy_image(
        command_buffer,
        src_image,
        src_image_layout,
        dst_image,
        dst_image_layout,
        static_cast<u32>(regions.size()),
        regions.data());
}

void Device::cmd_dispatch(
    const VkCommandBuffer command_buffer,
    const u32 group_count_x,
    const u32 group_count_y,
    const u32 group_count_z) const noexcept
{
    m_table.cmd_dispatch(command_buffer, group_count_x, group_count_y, group_count_z);
}

void Device::cmd_dispatch_indirect(
    const VkCommandBuffer command_buffer,
    const VkBuffer buffer,
    const u64 offset) const noexcept
{
    m_table.cmd_dispatch_indirect(command_buffer, buffer, offset);
}

void Device::cmd_draw(
    const VkCommandBuffer command_buffer,
    const u32 vertex_count,
    const u32 instance_count,
    const u32 first_vertex,
    const u32 first_instance) const noexcept
{
    m_table.cmd_draw(
        command_buffer, vertex_count, instance_count, first_vertex, first_instance);
}

void Device::cmd_draw_indexed(
    const VkCommandBuffer command_buffer,
    const u32 index_count,
    const u32 instance_count,
    const u32 first_index,
    const i32 vertex_offset,
    const u32 first_instance) const noexcept
{
    m_table.cmd_draw_indexed(
        command_buffer,
        index_count,
        instance_count,
        first_index,
        vertex_offset,
        first_instance);
}

void Device::cmd_draw_indexed_indirect(
    const VkCommandBuffer command_buffer,
    const VkBuffer buffer,
    const u64 offset,
    const u32 draw_count,
    const u32 stride) const noexcept
{
    m_table.cmd_draw_indexed_indirect(command_buffer, buffer, offset, draw_count, stride);
}

void Device::cmd_end_render_pass(const VkCommandBuffer command_buffer) const noexcept
{
    m_table.cmd_end_render_pass(command_buffer);
}

void Device::cmd_end_rendering(const VkCommandBuffer command_buffer) const noexcept
{
    m_table.cmd_end_rendering(command_buffer);
}

void Device::cmd_pipeline_barrier(
    const VkCommandBuffer command_buffer,
    const VkPipelineStageFlags src_stage_mask,
    const VkPipelineStageFlags dst_stage_mask,
    const VkDependencyFlags dependency_flags,
    const core::Span<const VkMemoryBarrier>& memory_barriers,
    const core::Span<const VkBufferMemoryBarrier>& buffer_memory_barriers,
    const core::Span<const VkImageMemoryBarrier>& image_memory_barriers) const noexcept
{
    m_table.cmd_pipeline_barrier(
        command_buffer,
        src_stage_mask,
        dst_stage_mask,
        dependency_flags,
        static_cast<u32>(memory_barriers.size()),
        memory_barriers.data(),
        static_cast<u32>(buffer_memory_barriers.size()),
        buffer_memory_barriers.data(),
        static_cast<u32>(image_memory_barriers.size()),
        image_memory_barriers.data());
}

void Device::cmd_push_constants(
    const VkCommandBuffer command_buffer,
    const VkPipelineLayout pipeline_layout,
    const VkShaderStageFlags shader_stage,
    const u32 offset,
    const core::Span<const char>& constants) const noexcept
{
    m_table.cmd_push_constants(
        command_buffer,
        pipeline_layout,
        shader_stage,
        offset,
        static_cast<u32>(constants.size()),
        constants.data());
}

void Device::cmd_set_scissor(
    const VkCommandBuffer command_buffer,
    const u32 first_scissor,
    const core::Span<const VkRect2D>& scissors) const noexcept
{
    m_table.cmd_set_scissor(
        command_buffer, first_scissor, static_cast<u32>(scissors.size()), scissors.data());
}

void Device::cmd_set_cull_mode(
    const VkCommandBuffer command_buffer, const VkCullModeFlags cull_mode) const noexcept
{
    m_table.cmd_set_cull_mode(command_buffer, cull_mode);
}

void Device::cmd_set_viewport(
    const VkCommandBuffer command_buffer,
    const u32 first_viewport,
    const core::Span<const VkViewport>& viewports) const noexcept
{
    m_table.cmd_set_viewport(
        command_buffer,
        first_viewport,
        static_cast<u32>(viewports.size()),
        viewports.data());
}

void Device::cmd_draw_indexed_indirect_count(
    const VkCommandBuffer command_buffer,
    const VkBuffer buffer,
    const u64 offset,
    const VkBuffer count_buffer,
    const u64 count_buffer_offset,
    const u32 max_draw_count,
    const u32 stride) const noexcept
{
    m_table.cmd_draw_indexed_indirect_count(
        command_buffer,
        buffer,
        offset,
        count_buffer,
        count_buffer_offset,
        max_draw_count,
        stride);
}

core::Expected<core::Array<VkDescriptorSet>, VkResult> Device::allocate_descriptor_sets(
    const VkDescriptorSetAllocateInfo& allocate_info) const noexcept
{
    core::Array<VkDescriptorSet> sets(allocate_info.descriptorSetCount);
    const VkResult result = m_table.allocate_descriptor_sets(
        m_device, &allocate_info, sets.data());
    if (result == VK_SUCCESS) {
        return core::move(sets);
    } else {
        return core::make_unexpected(result);
    }
}

core::Expected<VkCommandPool, VkResult> Device::create_command_pool(
    const VkCommandPoolCreateInfo& create_info,
    const VkAllocationCallbacks* allocator) const noexcept
{
    VkCommandPool pool;
    const VkResult result = m_table.create_command_pool(
        m_device, &create_info, allocator, &pool);
    if (result == VK_SUCCESS) {
        return pool;
    } else {
        return core::make_unexpected(result);
    }
}

core::Expected<VkFence, VkResult> Device::create_fence(
    const VkFenceCreateInfo& create_info,
    const VkAllocationCallbacks* allocator) const noexcept
{
    VkFence fence;
    const VkResult result = m_table.create_fence(
        m_device, &create_info, allocator, &fence);
    if (result == VK_SUCCESS) {
        return fence;
    } else {
        return core::make_unexpected(result);
    }
}

core::Expected<VkFramebuffer, VkResult> Device::create_framebuffer(
    const VkFramebufferCreateInfo& create_info,
    const VkAllocationCallbacks* allocator) const noexcept
{
    VkFramebuffer framebuffer;
    const VkResult result = m_table.create_framebuffer(
        m_device, &create_info, allocator, &framebuffer);
    if (result == VK_SUCCESS) {
        return framebuffer;
    } else {
        return core::make_unexpected(result);
    }
}

core::Expected<core::Array<VkPipeline>, VkResult> Device::create_graphics_pipelines(
    const VkPipelineCache pipeline_cache,
    const core::Span<const VkGraphicsPipelineCreateInfo>& create_infos,
    const VkAllocationCallbacks* allocator) const noexcept
{
    core::Array<VkPipeline> pipelines(create_infos.size());
    const VkResult result = m_table.create_graphics_pipelines(
        m_device,
        pipeline_cache,
        static_cast<u32>(create_infos.size()),
        create_infos.data(),
        allocator,
        pipelines.data());

    if (result == VK_SUCCESS) {
        return core::move(pipelines);
    } else {
        return core::make_unexpected(result);
    }
}

core::Expected<VkImageView, VkResult> Device::create_image_view(
    const VkImageViewCreateInfo& create_info,
    const VkAllocationCallbacks* allocator) const noexcept
{
    VkImageView image_view;
    const VkResult result = m_table.create_image_view(
        m_device, &create_info, allocator, &image_view);
    if (result == VK_SUCCESS) {
        return image_view;
    } else {
        return core::make_unexpected(result);
    }
}

core::Expected<VkPipelineCache, VkResult> Device::create_pipeline_cache(
    const VkPipelineCacheCreateInfo& create_info,
    const VkAllocationCallbacks* allocator) const noexcept
{
    VkPipelineCache pipeline_cache;
    const VkResult result = m_table.create_pipeline_cache(
        m_device, &create_info, allocator, &pipeline_cache);
    if (result == VK_SUCCESS) {
        return pipeline_cache;
    } else {
        return core::make_unexpected(result);
    }
}

core::Expected<VkPipelineLayout, VkResult> Device::create_pipeline_layout(
    const VkPipelineLayoutCreateInfo& create_info,
    const VkAllocationCallbacks* allocator) const noexcept
{
    VkPipelineLayout pipeline_layout;
    const VkResult result = m_table.create_pipeline_layout(
        m_device, &create_info, allocator, &pipeline_layout);
    if (result == VK_SUCCESS) {
        return pipeline_layout;
    } else {
        return core::make_unexpected(result);
    }
}

core::Expected<VkDescriptorSetLayout, VkResult> Device::create_descriptor_set_layout(
    const VkDescriptorSetLayoutCreateInfo& create_info,
    const VkAllocationCallbacks* allocator) const noexcept
{
    VkDescriptorSetLayout layout;
    const VkResult result = m_table.create_descriptor_set_layout(
        m_device, &create_info, allocator, &layout);
    if (result == VK_SUCCESS) {
        return layout;
    } else {
        return core::make_unexpected(result);
    }
}

core::Expected<VkSampler, VkResult> Device::create_sampler(
    const VkSamplerCreateInfo& create_info,
    const VkAllocationCallbacks* allocator) const noexcept
{
    VkSampler sampler;
    const VkResult result = m_table.create_sampler(
        m_device, &create_info, allocator, &sampler);
    if (result == VK_SUCCESS) {
        return sampler;
    } else {
        return core::make_unexpected(result);
    }
}

core::Expected<VkSemaphore, VkResult> Device::create_semaphore(
    const VkSemaphoreCreateInfo& create_info,
    const VkAllocationCallbacks* allocator) const noexcept
{
    VkSemaphore semaphore;
    const VkResult result = m_table.create_semaphore(
        m_device, &create_info, allocator, &semaphore);
    if (result == VK_SUCCESS) {
        return semaphore;
    } else {
        return core::make_unexpected(result);
    }
}

core::Expected<VkShaderModule, VkResult> Device::create_shader_module(
    const VkShaderModuleCreateInfo& create_info,
    const VkAllocationCallbacks* allocator) const noexcept
{
    VkShaderModule shader_module;
    const VkResult result = m_table.create_shader_module(
        m_device, &create_info, allocator, &shader_module);
    if (result == VK_SUCCESS) {
        return shader_module;
    } else {
        return core::make_unexpected(result);
    }
}

core::Expected<core::Array<VkPipeline>, VkResult> Device::create_compute_pipelines(
    const VkPipelineCache pipeline_cache,
    const core::Span<const VkComputePipelineCreateInfo>& create_infos,
    const VkAllocationCallbacks* allocator) const noexcept
{
    core::Array<VkPipeline> pipelines(create_infos.size());
    const VkResult result = m_table.create_compute_pipelines(
        m_device,
        pipeline_cache,
        static_cast<u32>(create_infos.size()),
        create_infos.data(),
        allocator,
        pipelines.data());

    if (result == VK_SUCCESS) {
        return core::move(pipelines);
    } else {
        return core::make_unexpected(result);
    }
}

core::Expected<VkDescriptorPool, VkResult> Device::create_descriptor_pool(
    const VkDescriptorPoolCreateInfo& create_info,
    const VkAllocationCallbacks* allocator) const noexcept
{
    VkDescriptorPool pool;
    const VkResult result = m_table.create_descriptor_pool(
        m_device, &create_info, allocator, &pool);
    if (result == VK_SUCCESS) {
        return pool;
    } else {
        return core::make_unexpected(result);
    }
}

void Device::destroy_command_pool(
    const VkCommandPool pool, const VkAllocationCallbacks* allocator) const noexcept
{
    m_table.destroy_command_pool(m_device, pool, allocator);
}

void Device::destroy_descriptor_pool(
    const VkDescriptorPool pool, const VkAllocationCallbacks* allocator) const noexcept
{
    m_table.destroy_descriptor_pool(m_device, pool, allocator);
}

void Device::destroy_descriptor_set_layout(
    const VkDescriptorSetLayout layout,
    const VkAllocationCallbacks* allocator) const noexcept
{
    m_table.destroy_descriptor_set_layout(m_device, layout, allocator);
}

void Device::destroy_fence(
    const VkFence fence, const VkAllocationCallbacks* allocator) const noexcept
{
    m_table.destroy_fence(m_device, fence, allocator);
}

void Device::destroy_framebuffer(
    const VkFramebuffer framebuffer,
    const VkAllocationCallbacks* allocator) const noexcept
{
    m_table.destroy_framebuffer(m_device, framebuffer, allocator);
}

void Device::destroy_image_view(
    const VkImageView image_view, const VkAllocationCallbacks* allocator) const noexcept
{
    m_table.destroy_image_view(m_device, image_view, allocator);
}

void Device::destroy_pipeline(
    const VkPipeline pipeline, const VkAllocationCallbacks* allocator) const noexcept
{
    m_table.destroy_pipeline(m_device, pipeline, allocator);
}

void Device::destroy_pipeline_cache(
    const VkPipelineCache pipeline_cache,
    const VkAllocationCallbacks* allocator) const noexcept
{
    m_table.destroy_pipeline_cache(m_device, pipeline_cache, allocator);
}

void Device::destroy_sampler(
    const VkSampler sampler, const VkAllocationCallbacks* allocator) const noexcept
{
    m_table.destroy_sampler(m_device, sampler, allocator);
}

void Device::destroy_semaphore(
    const VkSemaphore semaphore, const VkAllocationCallbacks* allocator) const noexcept
{
    m_table.destroy_semaphore(m_device, semaphore, allocator);
}

void Device::destroy_shader_module(
    const VkShaderModule shader_module,
    const VkAllocationCallbacks* allocator) const noexcept
{
    m_table.destroy_shader_module(m_device, shader_module, allocator);
}

core::Expected<void, VkResult> Device::end_command_buffer(
    const VkCommandBuffer command_buffer) const noexcept
{
    const VkResult result = m_table.end_command_buffer(command_buffer);
    if (result == VK_SUCCESS) {
        return {};
    } else {
        return core::make_unexpected(result);
    }
}

void Device::destroy_pipeline_layout(
    const VkPipelineLayout layout, const VkAllocationCallbacks* allocator) const noexcept
{
    m_table.destroy_pipeline_layout(m_device, layout, allocator);
}

void Device::destroy_render_pass(
    const VkRenderPass render_pass, const VkAllocationCallbacks* allocator) const noexcept
{
    m_table.destroy_render_pass(m_device, render_pass, allocator);
}

core::Expected<void, VkResult> Device::device_wait_idle() const noexcept
{
    const VkResult result = m_table.device_wait_idle(m_device);
    if (result == VK_SUCCESS) {
        return {};
    } else {
        return core::make_unexpected(result);
    }
}

VkQueue Device::get_device_queue(
    const u32 queue_family_index, const u32 queue_index) const noexcept
{
    VkQueue queue;
    m_table.get_device_queue(m_device, queue_family_index, queue_index, &queue);
    return queue;
}

void Device::update_descriptor_sets(
    const core::Span<const VkWriteDescriptorSet>& descriptor_writes,
    const core::Span<const VkCopyDescriptorSet>& descriptor_copies) const noexcept
{
    m_table.update_descriptor_sets(
        m_device,
        static_cast<u32>(descriptor_writes.size()),
        descriptor_writes.data(),
        static_cast<u32>(descriptor_copies.size()),
        descriptor_copies.data());
}

core::Expected<core::Array<char>, VkResult> Device::get_pipeline_cache_data(
    const VkPipelineCache pipeline_cache) const noexcept
{
    usize num = 0;
    VkResult result = m_table.get_pipeline_cache_data(
        m_device, pipeline_cache, &num, nullptr);
    if (result != VK_SUCCESS) {
        return core::make_unexpected(result);
    }

    core::Array<char> buffer(num);
    result = m_table.get_pipeline_cache_data(
        m_device, pipeline_cache, &num, buffer.data());

    if (result != VK_SUCCESS) {
        return core::make_unexpected(result);
    }

    return core::move(buffer);
}

core::Expected<void, VkResult> Device::wait_for_fences(
    const core::Span<const VkFence>& fences,
    const bool wait_all,
    const u64 timeout) const noexcept
{
    const VkResult result = m_table.wait_for_fences(
        m_device, static_cast<u32>(fences.size()), fences.data(), wait_all, timeout);
    if (result == VK_SUCCESS) {
        return {};
    } else {
        return core::make_unexpected(result);
    }
}

core::Expected<void, VkResult> Device::queue_submit(
    const VkQueue queue,
    const core::Span<const VkSubmitInfo>& submit_infos,
    const VkFence fence) const noexcept
{
    const VkResult result = m_table.queue_submit(
        queue, static_cast<u32>(submit_infos.size()), submit_infos.data(), fence);
    if (result == VK_SUCCESS) {
        return {};
    } else {
        return core::make_unexpected(result);
    }
}

core::Expected<void, VkResult> Device::reset_command_pool(
    const VkCommandPool command_pool,
    const VkCommandBufferResetFlags flags) const noexcept
{
    const VkResult result = m_table.reset_command_pool(m_device, command_pool, flags);
    if (result == VK_SUCCESS) {
        return {};
    } else {
        return core::make_unexpected(result);
    }
}

core::Expected<void, VkResult> Device::reset_fences(
    const core::Span<const VkFence>& fences) const noexcept
{
    const VkResult result = m_table.reset_fences(
        m_device, static_cast<u32>(fences.size()), fences.data());
    if (result == VK_SUCCESS) {
        return {};
    } else {
        return core::make_unexpected(result);
    }
}

core::Expected<VkRenderPass, VkResult> Device::create_render_pass2(
    const VkRenderPassCreateInfo2& create_info,
    const VkAllocationCallbacks* allocator) const noexcept
{
    VkRenderPass render_pass;
    const VkResult result = m_table.create_render_pass2(
        m_device, &create_info, allocator, &render_pass);
    if (result == VK_SUCCESS) {
        return render_pass;
    } else {
        return core::make_unexpected(result);
    }
}

VkDevice Device::get_handle() const noexcept
{
    return m_device;
}

const Device::Table& Device::get_table() const noexcept
{
    return m_table;
}

} // namespace tundra::vulkan_rhi::loader
