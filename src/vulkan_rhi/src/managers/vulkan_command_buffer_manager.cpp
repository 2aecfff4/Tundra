#include "managers/vulkan_command_buffer_manager.h"
#include "core/profiler.h"
#include "core/std/panic.h"
#include "core/std/utils.h"
#include "vulkan_device.h"
#include "vulkan_helpers.h"
#include <thread>
#include <type_traits>

namespace tundra::vulkan_rhi {

/////////////////////////////////////////////////////////////////////////////////////////
// VulkanCommandBufferManager::QueueThreadData

thread_local static u64 THREAD_ID = std::hash<std::thread::id> {}(
    std::this_thread::get_id());

void VulkanCommandBufferManager::QueueThreadData::clear_used_commands() noexcept
{
    free_command_buffers.insert(
        free_command_buffers.end(),
        used_command_buffers.begin(),
        used_command_buffers.end());
    used_command_buffers.clear();
}

/////////////////////////////////////////////////////////////////////////////////////////
// VulkanCommandBufferManager

VulkanCommandBufferManager::VulkanCommandBufferManager(
    core::SharedPtr<VulkanRawDevice> raw_device,
    core::SharedPtr<rhi::ResourceTracker> resource_tracker) noexcept
    : m_raw_device(raw_device)
    , m_resource_tracker(resource_tracker)
{
    TNDR_PROFILER_TRACE("VulkanCommandBufferManager::VulkanCommandBufferManager");

    const VulkanQueues& queues = raw_device->get_queues();
    for (FrameData& frame_data : m_frame_data) {
        const VkFenceCreateInfo fence_create_info {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .flags = VK_FENCE_CREATE_SIGNALED_BIT,
        };

        const VkFence fence = vulkan_map_result(
            raw_device->get_device().create_fence(fence_create_info, nullptr),
            "`create_fence` failed");

        frame_data = FrameData {
            .graphics_queue =
                QueueData {
                    .queue_family_index = core::get<u32>(queues.graphics_queue),
                },
            .compute_queue =
                QueueData {
                    .queue_family_index = core::get<u32>(queues.compute_queue),
                },
            .transfer_queue =
                QueueData {
                    .queue_family_index = core::get<u32>(queues.transfer_queue),
                },
            .present_queue =
                QueueData {
                    .queue_family_index = core::get<u32>(queues.present_queue),
                },
            .fence = fence,
        };
    }
}

VulkanCommandBufferManager::~VulkanCommandBufferManager() noexcept
{
    TNDR_PROFILER_TRACE("VulkanCommandBufferManager::~VulkanCommandBufferManager");

    for (FrameData& frame_data : m_frame_data) {
        const auto cleanup_frame_data = [&](QueueData& queue_data) {
            for (auto& [_, thread_data] : queue_data.thread_to_storage) {
                auto thread_data_lock = thread_data->lock();

                // Destroy resources.
                for (const u64 resource : thread_data_lock->resources.resources) {
                    m_resource_tracker->remove_reference(resource);
                }
                thread_data_lock->resources.resources.clear();

                m_raw_device->get_device().destroy_command_pool(
                    thread_data_lock->command_pool, nullptr);
            }
        };

        cleanup_frame_data(frame_data.graphics_queue);
        cleanup_frame_data(frame_data.compute_queue);
        cleanup_frame_data(frame_data.transfer_queue);
        cleanup_frame_data(frame_data.present_queue);

        m_raw_device->get_device().destroy_fence(frame_data.fence, nullptr);
    }
}

VulkanCommandBufferManager::CommandBundle VulkanCommandBufferManager::get_command_bundle(
    const rhi::QueueType queue_type) noexcept
{
    TNDR_PROFILER_TRACE("VulkanCommandBufferManager::get_command_bundle");

    FrameData& frame_data =
        m_frame_data[*m_frame_counter.lock() % rhi::config::MAX_FRAMES_IN_FLIGHT];
    QueueData& queue_data = [&]() -> QueueData& {
        switch (queue_type) {
            case rhi::QueueType::Compute:
                return frame_data.compute_queue;
            case rhi::QueueType::Graphics:
                return frame_data.graphics_queue;
            case rhi::QueueType::Transfer:
                return frame_data.transfer_queue;
            case rhi::QueueType::Present:
                return frame_data.present_queue;
        }

        core::panic("Invalid enum");
    }();

    auto thread_data = [&] {
        TNDR_PROFILER_TRACE(
            "VulkanCommandBufferManager::get_command_bundle::thread_data");

        if (!queue_data.thread_to_storage.contains(THREAD_ID)) {
            const VkCommandPoolCreateInfo command_pool_create_info {
                .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                .queueFamilyIndex = queue_data.queue_family_index,
            };

            const VkCommandPool command_pool = vulkan_map_result(
                m_raw_device->get_device().create_command_pool(
                    command_pool_create_info, nullptr),
                "`create_command_pool` failed");

#if !TNDR_BUILD_SHIPPING
            const std::string command_pool_name = fmt::format(
                "Command Pool | Thread: {} | Queue: {}", THREAD_ID, queue_type);
            helpers::set_object_name(
                m_raw_device,
                reinterpret_cast<u64>(command_pool),
                VK_OBJECT_TYPE_COMMAND_POOL,
                command_pool_name.c_str());
#endif // TNDR_BUILD_SHIPPING

            queue_data.thread_to_storage.insert({
                THREAD_ID,
                std::make_shared<core::Lock<QueueThreadData>>(QueueThreadData {
                    .command_pool = command_pool,
                }),
            });
        }

        return queue_data.thread_to_storage[THREAD_ID];
    }();

    const VkCommandBuffer command_buffer = [&] {
        TNDR_PROFILER_TRACE(
            "VulkanCommandBufferManager::get_command_bundle::command_buffer");

        auto thread_data_lock = thread_data->lock();

        if (thread_data_lock->free_command_buffers.empty()) {
            const VkCommandBufferAllocateInfo command_buffer_allocate_info {
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                .commandPool = thread_data_lock->command_pool,
                .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                .commandBufferCount = 1,
            };

            const VkCommandBuffer command_buffer = vulkan_map_result(
                m_raw_device->get_device().allocate_command_buffers(
                    command_buffer_allocate_info),
                "`allocate_command_buffers` failed")[0];

#if !TNDR_BUILD_SHIPPING
            const std::string command_buffer_name = fmt::format(
                "Command Buffer | Thread: {} | Queue: {}", THREAD_ID, queue_type);
            helpers::set_object_name(
                m_raw_device,
                reinterpret_cast<u64>(command_buffer),
                VK_OBJECT_TYPE_COMMAND_BUFFER,
                command_buffer_name.c_str());
#endif // TNDR_BUILD_SHIPPING

            thread_data_lock->used_command_buffers.push_back(command_buffer);
            return command_buffer;
        } else {
            const VkCommandBuffer command_buffer = thread_data_lock->free_command_buffers
                                                       .front();
            thread_data_lock->free_command_buffers.pop_front();
            thread_data_lock->used_command_buffers.push_back(command_buffer);
            return command_buffer;
        }
    }();

    return VulkanCommandBufferManager::CommandBundle {
        .command_buffer = command_buffer,
        .thread_data = core::move(thread_data),
    };
}

void VulkanCommandBufferManager::wait_for_free_pool() noexcept
{
    TNDR_PROFILER_TRACE("VulkanCommandBufferManager::wait_for_free_pool");

    FrameData& frame_data =
        m_frame_data[*m_frame_counter.lock() % rhi::config::MAX_FRAMES_IN_FLIGHT];

    {
        TNDR_PROFILER_TRACE(
            "VulkanCommandBufferManager::wait_for_free_pool::wait_for_fences");

        vulkan_map_result(
            m_raw_device->get_device().wait_for_fences(
                core::as_span(frame_data.fence), true, UINT64_MAX),
            "`wait_for_fences` failed");
    }

    {
        TNDR_PROFILER_TRACE(
            "VulkanCommandBufferManager::wait_for_free_pool::reset_fences");

        vulkan_map_result(
            m_raw_device->get_device().reset_fences(core::as_span(frame_data.fence)),
            "`reset_fences` failed");
    }

    const auto reset_command_pool = [&](QueueData& queue_data) {
        for (auto& [_, thread_data] : queue_data.thread_to_storage) {
            auto thread_data_lock = thread_data->lock();

            vulkan_map_result(
                m_raw_device->get_device().reset_command_pool(
                    thread_data_lock->command_pool, 0),
                "`reset_command_pool` failed");

            thread_data_lock->clear_used_commands();

            // Destroy resources.
            for (const u64 resource : thread_data_lock->resources.resources) {
                m_resource_tracker->remove_reference(resource);
            }
            thread_data_lock->resources.resources.clear();
        }
    };

    TNDR_PROFILER_TRACE("VulkanCommandBufferManager::reset_command_pool");

    reset_command_pool(frame_data.graphics_queue);
    reset_command_pool(frame_data.compute_queue);
    reset_command_pool(frame_data.transfer_queue);
    reset_command_pool(frame_data.present_queue);
}

void VulkanCommandBufferManager::end_frame() noexcept
{
    *m_frame_counter.lock() += 1;
}

VkFence VulkanCommandBufferManager::get_fence() noexcept
{
    const FrameData& frame_data =
        m_frame_data[*m_frame_counter.lock() % rhi::config::MAX_FRAMES_IN_FLIGHT];
    return frame_data.fence;
}

} // namespace tundra::vulkan_rhi
