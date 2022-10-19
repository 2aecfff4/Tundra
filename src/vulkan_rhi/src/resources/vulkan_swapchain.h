#pragma once
#include "core/std/containers/array.h"
#include "core/std/shared_ptr.h"
#include "core/std/tuple.h"
#include "rhi/resources/swapchain.h"
#include "vulkan_utils.h"

namespace tundra::vulkan_rhi {

class VulkanRawDevice;

///
struct SwapchainAttachment {
    VkImage image;
    VkImageLayout image_layout = VK_IMAGE_LAYOUT_UNDEFINED;
};

///
class VulkanSwapchain {
private:
    core::SharedPtr<VulkanRawDevice> m_raw_device;
    platform::WindowNativeHandle m_window_handle;
    VkSurfaceKHR m_surface;
    VkSwapchainKHR m_swapchain;
    VkSurfaceFormatKHR m_surface_format;
    core::Array<SwapchainAttachment> m_attachments;
    core::Array<VkSemaphore> m_image_available_semaphores;
    u64 m_swap_index = 0;

public:
    VulkanSwapchain(
        core::SharedPtr<VulkanRawDevice> raw_device,
        const rhi::SwapchainCreateInfo& create_info) noexcept;
    ~VulkanSwapchain() noexcept;

    VulkanSwapchain(VulkanSwapchain&& rhs) noexcept;
    VulkanSwapchain& operator=(VulkanSwapchain&& rhs) noexcept;
    VulkanSwapchain(const VulkanSwapchain&) noexcept = delete;
    VulkanSwapchain& operator=(const VulkanSwapchain&) noexcept = delete;

public:
    /// Returns a tuple `(image_index, VkSemaphore, SwapchainAttachment&)`.
    [[nodiscard]] core::Tuple<u32, VkSemaphore, SwapchainAttachment&>
        acquire_image_index() noexcept;

public:
    [[nodiscard]] VkSwapchainKHR get_swapchain() const noexcept;
};

} // namespace tundra::vulkan_rhi
