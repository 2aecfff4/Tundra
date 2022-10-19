#pragma once
#include "core/std/containers/array.h"
#include "core/std/shared_ptr.h"
#include "rhi/resources/buffer.h"
#include "vulkan_allocator.h"
#include "vulkan_utils.h"


namespace tundra::vulkan_rhi {

class VulkanAllocator;
class VulkanRawDevice;

///
class VulkanBuffer {
private:
    core::SharedPtr<VulkanAllocator> m_allocator;
    u64 m_buffer_capacity;
    rhi::BufferUsageFlags m_buffer_usage;
    rhi::MemoryType m_memory_type;
    VulkanAllocation<VkBuffer> m_allocation;

public:
    VulkanBuffer(
        const core::SharedPtr<VulkanRawDevice>& raw_device,
        core::SharedPtr<VulkanAllocator> allocator,
        const rhi::BufferCreateInfo& create_info) noexcept;
    ~VulkanBuffer() noexcept;

    VulkanBuffer(VulkanBuffer&& rhs) noexcept;
    VulkanBuffer& operator=(VulkanBuffer&& rhs) noexcept;
    VulkanBuffer(const VulkanBuffer&) noexcept = delete;
    VulkanBuffer& operator=(const VulkanBuffer&) noexcept = delete;

public:
    void update_buffer(
        const core::Array<rhi::BufferUpdateRegion>& update_regions) noexcept;

public:
    [[nodiscard]] VkBuffer get_buffer() const noexcept;
    [[nodiscard]] u64 get_capacity() const noexcept;
    [[nodiscard]] rhi::BufferUsageFlags get_usage_flags() const noexcept;
    [[nodiscard]] rhi::MemoryType get_memory_type() const noexcept;
};

} // namespace tundra::vulkan_rhi
