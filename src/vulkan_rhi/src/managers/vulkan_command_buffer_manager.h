#pragma once
#include "core/core.h"
#include "core/std/containers/array.h"
#include "core/std/containers/deque.h"
#include "core/std/containers/hash_map.h"
#include "core/std/option.h"
#include "core/std/shared_ptr.h"
#include "core/std/sync/lock.h"
#include "rhi/config.h"
#include "rhi/enums.h"
#include "rhi/resources/resource_tracker.h"
#include "vulkan_utils.h"

namespace tundra::vulkan_rhi {

class VulkanRawDevice;

class VulkanCommandBufferManager {
private:
    struct QueueThreadData {
        rhi::ResourceTracker::Resources resources;
        VkCommandPool command_pool;
        core::Deque<VkCommandBuffer> free_command_buffers;
        core::Deque<VkCommandBuffer> used_command_buffers;

        void clear_used_commands() noexcept;
    };

    struct QueueData {
        core::Lock<core::HashMap<u64, core::SharedPtr<QueueThreadData>>> thread_to_storage;
        u32 queue_family_index;
    };

    struct FrameData {
        QueueData graphics_queue;
        QueueData compute_queue;
        QueueData transfer_queue;
        QueueData present_queue;
        VkFence fence;
    };

public:
    struct CommandBundle {
        VkCommandBuffer command_buffer;
        core::SharedPtr<QueueThreadData> thread_data;

        [[nodiscard]] rhi::ResourceTracker::Resources& get_resources() const noexcept
        {
            return thread_data->resources;
        }
    };

private:
    core::SharedPtr<VulkanRawDevice> m_raw_device;
    core::SharedPtr<rhi::ResourceTracker> m_resource_tracker;
    FrameData m_frame_data[rhi::config::MAX_FRAMES_IN_FLIGHT];
    core::Lock<u64> m_frame_counter;

public:
    VulkanCommandBufferManager(
        core::SharedPtr<VulkanRawDevice> device,
        core::SharedPtr<rhi::ResourceTracker> resource_tracker) noexcept;
    ~VulkanCommandBufferManager() noexcept;

    VulkanCommandBufferManager(const VulkanCommandBufferManager&) = delete;
    VulkanCommandBufferManager(VulkanCommandBufferManager&&) noexcept = delete;
    VulkanCommandBufferManager& operator=(const VulkanCommandBufferManager&) = delete;
    VulkanCommandBufferManager& operator=(VulkanCommandBufferManager&&) noexcept = delete;

public:
    [[nodiscard]] CommandBundle get_command_bundle(
        const rhi::QueueType queue_type) noexcept;
    void wait_for_free_pool() noexcept;
    void end_frame() noexcept;
    [[nodiscard]] VkFence get_fence() noexcept;
};

} // namespace tundra::vulkan_rhi
