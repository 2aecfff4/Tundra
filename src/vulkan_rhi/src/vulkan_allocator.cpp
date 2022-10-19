#include "vulkan_allocator.h"
#include "core/profiler.h"
#include "core/std/assert.h"
#include "core/std/panic.h"
#include "vulkan_device.h"
#include "vulkan_instance.h"
#include "vulkan_utils.h"

#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0
#if TNDR_COMPILER_MSVC
#pragma warning(push, 0)
#include <vk_mem_alloc/vk_mem_alloc.h>
#pragma warning(pop)
#else
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wall"
#include <vk_mem_alloc/vk_mem_alloc.h>
#pragma GCC diagnostic pop
#endif

namespace tundra::vulkan_rhi {

[[nodiscard]] static VmaMemoryUsage map_memory_type(
    const rhi::MemoryType memory_type) noexcept
{
    switch (memory_type) {
        case rhi::MemoryType::GPU:
            return VMA_MEMORY_USAGE_GPU_ONLY;
        case rhi::MemoryType::Upload:
            return VMA_MEMORY_USAGE_CPU_ONLY;
        case rhi::MemoryType::Readback:
            return VMA_MEMORY_USAGE_GPU_TO_CPU;
        case rhi::MemoryType::Dynamic:
            return VMA_MEMORY_USAGE_CPU_TO_GPU;
    }

    core::panic("Invalid enum!");
}

VulkanAllocator::VulkanAllocator(core::SharedPtr<VulkanRawDevice> device) noexcept
    : m_device(core::move(device))
    , m_allocator(VK_NULL_HANDLE)
{
    TNDR_PROFILER_TRACE("VulkanAllocator::VulkanAllocator");

    const VmaVulkanFunctions vma_vulkan_functions {
        .vkGetPhysicalDeviceProperties = m_device->get_instance()
                                             ->get_instance()
                                             .get_table()
                                             .get_physical_device_properties,
        .vkGetPhysicalDeviceMemoryProperties = m_device->get_instance()
                                                   ->get_instance()
                                                   .get_table()
                                                   .get_physical_device_memory_properties,
        .vkAllocateMemory = m_device->get_device().get_table().allocate_memory,
        .vkFreeMemory = m_device->get_device().get_table().free_memory,
        .vkMapMemory = m_device->get_device().get_table().map_memory,
        .vkUnmapMemory = m_device->get_device().get_table().unmap_memory,
        .vkFlushMappedMemoryRanges =
            m_device->get_device().get_table().flush_mapped_memory_ranges,
        .vkInvalidateMappedMemoryRanges =
            m_device->get_device().get_table().invalidate_mapped_memory_ranges,
        .vkBindBufferMemory = m_device->get_device().get_table().bind_buffer_memory,
        .vkBindImageMemory = m_device->get_device().get_table().bind_image_memory,
        .vkGetBufferMemoryRequirements =
            m_device->get_device().get_table().get_buffer_memory_requirements,
        .vkGetImageMemoryRequirements =
            m_device->get_device().get_table().get_image_memory_requirements,
        .vkCreateBuffer = m_device->get_device().get_table().create_buffer,
        .vkDestroyBuffer = m_device->get_device().get_table().destroy_buffer,
        .vkCreateImage = m_device->get_device().get_table().create_image,
        .vkDestroyImage = m_device->get_device().get_table().destroy_image,
        .vkCmdCopyBuffer = m_device->get_device().get_table().cmd_copy_buffer,
        .vkGetBufferMemoryRequirements2KHR =
            m_device->get_device().get_table().get_buffer_memory_requirements2,
        .vkGetImageMemoryRequirements2KHR =
            m_device->get_device().get_table().get_image_memory_requirements2,
        .vkBindBufferMemory2KHR = m_device->get_device().get_table().bind_buffer_memory2,
        .vkBindImageMemory2KHR = m_device->get_device().get_table().bind_image_memory2,
        .vkGetPhysicalDeviceMemoryProperties2KHR =
            m_device->get_instance()
                ->get_instance()
                .get_table()
                .get_physical_device_memory_properties2,
        .vkGetDeviceBufferMemoryRequirements =
            m_device->get_device().get_table().get_device_buffer_memory_requirements,
        .vkGetDeviceImageMemoryRequirements =
            m_device->get_device().get_table().get_device_image_memory_requirements,
    };

    const VmaAllocatorCreateInfo allocator_create_info {
        .physicalDevice = m_device->get_physical_device(),
        .device = m_device->get_device().get_handle(),
        .pAllocationCallbacks = nullptr,
        .pVulkanFunctions = &vma_vulkan_functions,
        .instance = m_device->get_instance()->get_instance().get_handle(),
        .vulkanApiVersion = VK_API_VERSION_1_3,
    };

    const VkResult result = vmaCreateAllocator(&allocator_create_info, &m_allocator);
    if (result != VK_SUCCESS) {
        core::panic("`vmaCreateAllocator` failed! Error: {}", vk_result_to_str(result));
    }
}

VulkanAllocator::~VulkanAllocator() noexcept
{
    TNDR_PROFILER_TRACE("VulkanAllocator::~VulkanAllocator");

    vmaDestroyAllocator(m_allocator);
    m_allocator = nullptr;
}

core::Expected<VulkanAllocation<VkBuffer>, VkResult> VulkanAllocator::create_buffer(
    const VkBufferCreateInfo& create_info,
    const AllocationCreateInfo& allocation_create_info) noexcept
{
    TNDR_PROFILER_TRACE("VulkanAllocator::create_buffer");

    const VmaAllocationCreateInfo vma_allocation_create_info {
        .usage = map_memory_type(allocation_create_info.memory_type),
    };

    VkBuffer buffer;
    VmaAllocation allocation;
    VkResult result = vmaCreateBuffer(
        m_allocator,
        &create_info,
        &vma_allocation_create_info,
        &buffer,
        &allocation,
        nullptr);

    if (result != VK_SUCCESS) {
        return core::make_unexpected(result);
    }

    void* mapped_memory = nullptr;
    if ((allocation_create_info.memory_type == rhi::MemoryType::Readback) ||
        (allocation_create_info.memory_type == rhi::MemoryType::Upload) ||
        (allocation_create_info.memory_type == rhi::MemoryType::Dynamic)) {
        result = vmaMapMemory(m_allocator, allocation, &mapped_memory);

        if (result != VK_SUCCESS) {
            mapped_memory = nullptr;
        }
    }

    return VulkanAllocation<VkBuffer> {
        .object = buffer,
        .allocation = allocation,
        .mapped_memory = mapped_memory,
    };
}

void VulkanAllocator::destroy_buffer(const VulkanAllocation<VkBuffer>& allocation) noexcept
{
    TNDR_PROFILER_TRACE("VulkanAllocator::destroy_buffer");

    tndr_assert(
        allocation.object != VK_NULL_HANDLE,
        "`allocation.handle` must be a valid handle!");
    tndr_assert(
        allocation.allocation != nullptr, "`allocation.handle` must be a valid handle!");

    if (allocation.mapped_memory != nullptr) {
        vmaUnmapMemory(m_allocator, allocation.allocation);
    }

    vmaDestroyBuffer(m_allocator, allocation.object, allocation.allocation);
}

core::Expected<VulkanAllocation<VkImage>, VkResult> VulkanAllocator::create_image(
    const VkImageCreateInfo& create_info,
    const AllocationCreateInfo& allocation_create_info) noexcept
{
    TNDR_PROFILER_TRACE("VulkanAllocator::create_image");

    const VmaAllocationCreateInfo vma_allocation_create_info {
        .usage = map_memory_type(allocation_create_info.memory_type),
    };

    VkImage image;
    VmaAllocation allocation;
    VkResult result = vmaCreateImage(
        m_allocator,
        &create_info,
        &vma_allocation_create_info,
        &image,
        &allocation,
        nullptr);

    if (result != VK_SUCCESS) {
        return core::make_unexpected(result);
    }

    void* mapped_memory = nullptr;
    if ((allocation_create_info.memory_type == rhi::MemoryType::Readback) ||
        (allocation_create_info.memory_type == rhi::MemoryType::Upload) ||
        (allocation_create_info.memory_type == rhi::MemoryType::Dynamic)) {
        result = vmaMapMemory(m_allocator, allocation, &mapped_memory);

        if (result != VK_SUCCESS) {
            mapped_memory = nullptr;
        }
    }

    return VulkanAllocation<VkImage> {
        .object = image,
        .allocation = allocation,
        .mapped_memory = mapped_memory,
    };
}

void VulkanAllocator::destroy_image(const VulkanAllocation<VkImage>& allocation) noexcept
{
    TNDR_PROFILER_TRACE("VulkanAllocator::destroy_image");

    tndr_assert(
        allocation.object != VK_NULL_HANDLE,
        "`allocation.handle` must be a valid handle!");
    tndr_assert(
        allocation.allocation != nullptr, "`allocation.handle` must be a valid handle!");

    if (allocation.mapped_memory != nullptr) {
        vmaUnmapMemory(m_allocator, allocation.allocation);
    }

    vmaDestroyImage(m_allocator, allocation.object, allocation.allocation);
}

} // namespace tundra::vulkan_rhi
