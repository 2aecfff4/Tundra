#pragma once
#include "vulkan_utils.h"

#if defined(VK_KHR_swapchain)
#include "core/core.h"
#include "core/std/containers/array.h"
#include "core/std/expected.h"
#include "core/std/tuple.h"

namespace tundra::vulkan_rhi::loader {
class Device;
class Instance;
} // namespace tundra::vulkan_rhi::loader

namespace tundra::vulkan_rhi::loader::khr {

class Swapchain {
private:
    VkDevice m_device;

    struct Table {
        // clang-format off
        PFN_vkAcquireNextImageKHR acquire_next_image_khr;
        PFN_vkCreateSwapchainKHR create_swapchain_khr;
        PFN_vkDestroySwapchainKHR destroy_swapchain_khr;
        PFN_vkGetSwapchainImagesKHR get_swapchain_images_khr;
        PFN_vkQueuePresentKHR queue_present_khr;
        // clang-format on
    } m_table;

public:
    Swapchain(const loader::Instance& instance, const loader::Device& device) noexcept;

public:
    /// Returns `core::Tuple<image_index, is_suboptimal>`.
    /// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkAcquireNextImageKHR.html
    [[nodiscard]] core::Expected<core::Tuple<u32, bool>, VkResult> acquire_next_image(
        const VkSwapchainKHR swapchain,
        const u64 timeout,
        const VkSemaphore semaphore,
        const VkFence fence) const noexcept;

    /// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkCreateSwapchainKHR.html
    [[nodiscard]] core::Expected<VkSwapchainKHR, VkResult> create_swapchain(
        const VkSwapchainCreateInfoKHR& create_info,
        const VkAllocationCallbacks* allocator) const noexcept;

    /// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkDestroySwapchainKHR.html
    void destroy_swapchain(
        const VkSwapchainKHR swapchain,
        const VkAllocationCallbacks* allocator) const noexcept;

    /// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkGetSwapchainImagesKHR.html
    [[nodiscard]] core::Expected<core::Array<VkImage>, VkResult> get_swapchain_images(
        const VkSwapchainKHR swapchain) const noexcept;

    /// On success returns whether swapchain is suboptimal, or not.
    /// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkQueuePresentKHR.html
    [[nodiscard]] core::Expected<bool, VkResult> queue_present(
        const VkQueue queue, const VkPresentInfoKHR& present_info) const noexcept;

public:
    [[nodiscard]] const Table& get_table() const noexcept;
    [[nodiscard]] static const char* name() noexcept;
};

} // namespace tundra::vulkan_rhi::loader::khr

#endif
