#pragma once
#include "core/core.h"
#include "core/std/containers/array.h"
#include "core/std/shared_ptr.h"
#include "core/std/tuple.h"
#include "managers/managers.h"
#include "rhi/config.h"
#include "rhi/submit_info.h"
#include "vulkan_utils.h"

namespace tundra::vulkan_rhi {

///
class VulkanSubmitWorkScheduler {
private:
    core::SharedPtr<VulkanRawDevice> m_raw_device;
    Managers m_managers;
    VkSemaphore m_present_semaphores[rhi::config::MAX_FRAMES_IN_FLIGHT] {};
    /// `(semaphore, value)`.
    core::Tuple<VkSemaphore, u64> m_timeline_semaphore {};
    u64 m_submit_counter = 0;

public:
    VulkanSubmitWorkScheduler(
        core::SharedPtr<VulkanRawDevice> raw_device, Managers managers) noexcept;
    ~VulkanSubmitWorkScheduler() noexcept;

public:
    void submit(
        core::Array<rhi::SubmitInfo> submit_infos,
        core::Array<rhi::PresentInfo> present_infos) noexcept;
};

} // namespace tundra::vulkan_rhi
