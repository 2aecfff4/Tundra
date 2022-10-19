#pragma once
#include "core/core.h"
#include "core/std/containers/array.h"
#include "core/std/shared_ptr.h"
#include "rhi/commands/barrier.h"
#include "vulkan_utils.h"

namespace tundra::vulkan_rhi {

class VulkanBuffer;
class VulkanRawDevice;
class VulkanTexture;

///
class VulkanBarrier {
private:
    core::SharedPtr<VulkanRawDevice> m_raw_device;

private:
    core::Array<VkBufferMemoryBarrier> m_buffer_barriers;
    core::Array<VkImageMemoryBarrier> m_image_barriers;
    core::Array<VkMemoryBarrier> m_memory_barriers;

    VkPipelineStageFlags m_src_stage_mask = 0;
    VkPipelineStageFlags m_dst_stage_mask = 0;

public:
    VulkanBarrier(const core::SharedPtr<VulkanRawDevice>& raw_device) noexcept;

public:
    void image_layout_transition(
        const VkImage image,
        const VkImageLayout old_layout,
        const VkImageLayout new_layout,
        const VkImageSubresourceRange& subresource_range) noexcept;

    void global_barrier(const rhi::GlobalBarrier& barrier) noexcept;
    void texture_barrier(
        const VulkanTexture& texture, const rhi::TextureBarrier& barrier) noexcept;
    void buffer_barrier(
        const VulkanBuffer& buffer, const rhi::BufferBarrier& barrier) noexcept;

public:
    void execute(const VkCommandBuffer command_buffer) const noexcept;
    void reset() noexcept;
};

} // namespace tundra::vulkan_rhi
