#include "resources/vulkan_buffer.h"
#include "core/memory/pointer_math.h"
#include "core/profiler.h"
#include "vulkan_device.h"
#include "vulkan_helpers.h"
#include "vulkan_instance.h"

namespace tundra::vulkan_rhi {

VulkanBuffer::VulkanBuffer(
    const core::SharedPtr<VulkanRawDevice>& raw_device,
    core::SharedPtr<VulkanAllocator> allocator,
    const rhi::BufferCreateInfo& create_info) noexcept
    : m_allocator(core::move(allocator))
    , m_buffer_capacity(create_info.size)
    , m_buffer_usage(create_info.usage)
    , m_memory_type(create_info.memory_type)
{
    TNDR_PROFILER_TRACE("VulkanBuffer::VulkanBuffer");

    const VkBufferCreateInfo buffer_create_info {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = create_info.size,
        .usage = helpers::map_buffer_usage(create_info.usage),
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };

    m_allocation = vulkan_map_result(
        m_allocator->create_buffer(
            buffer_create_info,
            AllocationCreateInfo {
                .memory_type = create_info.memory_type,
            }),
        "`create_buffer` failed");

    if (!create_info.name.empty()) {
        helpers::set_object_name(
            raw_device,
            reinterpret_cast<u64>(m_allocation.object),
            VK_OBJECT_TYPE_BUFFER,
            create_info.name.c_str());
    }
}

VulkanBuffer::~VulkanBuffer() noexcept
{
    TNDR_PROFILER_TRACE("VulkanBuffer::~VulkanBuffer");

    if (m_allocation.is_valid()) {
        m_allocator->destroy_buffer(m_allocation);
    }
}

VulkanBuffer::VulkanBuffer(VulkanBuffer&& rhs) noexcept
    : m_allocator(core::move(rhs.m_allocator))
    , m_buffer_capacity(rhs.m_buffer_capacity)
    , m_buffer_usage(rhs.m_buffer_usage)
    , m_memory_type(rhs.m_memory_type)
    , m_allocation(core::exchange(rhs.m_allocation, VulkanAllocation<VkBuffer> {}))
{
}

VulkanBuffer& VulkanBuffer::operator=(VulkanBuffer&& rhs) noexcept
{
    if (this != &rhs) {
        m_allocator = core::move(rhs.m_allocator);
        m_buffer_capacity = rhs.m_buffer_capacity;
        m_buffer_usage = rhs.m_buffer_usage;
        m_memory_type = rhs.m_memory_type;
        m_allocation = core::exchange(rhs.m_allocation, VulkanAllocation<VkBuffer> {});
    }
    return *this;
}

void VulkanBuffer::update_buffer(
    const core::Array<rhi::BufferUpdateRegion>& update_regions) noexcept
{
    TNDR_PROFILER_TRACE("VulkanBuffer::update_buffer");

    tndr_assert(m_allocation.mapped_memory != nullptr, "`mapped_memory` is nullptr.");
    tndr_assert(
        (m_memory_type == rhi::MemoryType::Upload) ||
            (m_memory_type == rhi::MemoryType::Readback) ||
            (m_memory_type == rhi::MemoryType::Dynamic),
        "Invalid memory type.");

    for (const rhi::BufferUpdateRegion& update_region : update_regions) {
        tndr_assert(
            update_region.src.size() <= this->get_capacity(),
            "`update_region.src` is bigger than buffer capacity.");
        tndr_assert(
            (update_region.dst_offset + static_cast<u64>(update_region.src.size())) <=
                this->get_capacity(),
            "`update_region.src` + `update_region.dst_offset` is bigger than buffer "
            "capacity.");

        std::memcpy(
            core::pointer_math::add(m_allocation.mapped_memory, update_region.dst_offset),
            update_region.src.data(),
            update_region.src.size());
    }
}

VkBuffer VulkanBuffer::get_buffer() const noexcept
{
    return m_allocation.object;
}

u64 VulkanBuffer::get_capacity() const noexcept
{
    return m_buffer_capacity;
}

rhi::BufferUsageFlags VulkanBuffer::get_usage_flags() const noexcept
{
    return m_buffer_usage;
}

rhi::MemoryType VulkanBuffer::get_memory_type() const noexcept
{
    return m_memory_type;
}

} // namespace tundra::vulkan_rhi
