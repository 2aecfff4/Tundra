#pragma once
#include "core/core.h"
#include "core/std/expected.h"
#include "core/std/shared_ptr.h"
#include "rhi/resources/buffer.h"
#include "vulkan_utils.h"

struct VmaAllocation_T;
struct VmaAllocator_T;

namespace tundra::vulkan_rhi {

class VulkanRawDevice;

///
struct AllocationCreateInfo {
    rhi::MemoryType memory_type = rhi::MemoryType::GPU;
};

///
template <typename T>
struct VulkanAllocation {
    T object;
    VmaAllocation_T* allocation = nullptr;
    /// Only valid if the memory is host visible.
    void* mapped_memory = nullptr;

    [[nodiscard]] bool is_valid() const noexcept
    {
        return allocation != nullptr;
    }
};

///
class VulkanAllocator {
private:
    core::SharedPtr<VulkanRawDevice> m_device;
    VmaAllocator_T* m_allocator;

public:
    VulkanAllocator(core::SharedPtr<VulkanRawDevice> device) noexcept;
    ~VulkanAllocator() noexcept;

public:
    [[nodiscard]] core::Expected<VulkanAllocation<VkBuffer>, VkResult> create_buffer(
        const VkBufferCreateInfo& create_info,
        const AllocationCreateInfo& allocation_create_info) noexcept;

    void destroy_buffer(const VulkanAllocation<VkBuffer>& allocation) noexcept;

    [[nodiscard]] core::Expected<VulkanAllocation<VkImage>, VkResult> create_image(
        const VkImageCreateInfo& create_info,
        const AllocationCreateInfo& allocation_create_info) noexcept;

    void destroy_image(const VulkanAllocation<VkImage>& allocation) noexcept;
};

} // namespace tundra::vulkan_rhi
