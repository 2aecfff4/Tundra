#include "loader/extensions/khr/swapchain.h"

#if defined(VK_KHR_swapchain)
#include "loader/device.h"
#include "loader/instance.h"

namespace tundra::vulkan_rhi::loader::khr {

Swapchain::Swapchain(
    const loader::Instance& instance, const loader::Device& device) noexcept
    : m_device(device.get_handle())
{
    const auto load = [device = m_device,
                       get_device_proc_addr = instance.get_table().get_device_proc_addr](
                          const char* name) {
        return get_device_proc_addr(device, name);
    };

    // clang-format off
    m_table.acquire_next_image_khr = reinterpret_cast<PFN_vkAcquireNextImageKHR>(load("vkAcquireNextImageKHR"));
    m_table.create_swapchain_khr = reinterpret_cast<PFN_vkCreateSwapchainKHR>(load("vkCreateSwapchainKHR"));
    m_table.destroy_swapchain_khr = reinterpret_cast<PFN_vkDestroySwapchainKHR>(load("vkDestroySwapchainKHR"));
    m_table.get_swapchain_images_khr = reinterpret_cast<PFN_vkGetSwapchainImagesKHR>(load("vkGetSwapchainImagesKHR"));
    m_table.queue_present_khr = reinterpret_cast<PFN_vkQueuePresentKHR>(load("vkQueuePresentKHR"));
    // clang-format on
}

core::Expected<core::Tuple<u32, bool>, VkResult> Swapchain::acquire_next_image(
    const VkSwapchainKHR swapchain,
    const u64 timeout,
    const VkSemaphore semaphore,
    const VkFence fence) const noexcept
{
    u32 index;
    const VkResult result = m_table.acquire_next_image_khr(
        m_device, swapchain, timeout, semaphore, fence, &index);

    switch (result) {
        case VK_SUCCESS:
            return core::make_tuple(index, false);
        case VK_SUBOPTIMAL_KHR:
            return core::make_tuple(index, true);
        default:
            return core::make_unexpected(result);
    }
}

core::Expected<VkSwapchainKHR, VkResult> Swapchain::create_swapchain(
    const VkSwapchainCreateInfoKHR& create_info,
    const VkAllocationCallbacks* allocator) const noexcept
{
    VkSwapchainKHR swapchain;
    const VkResult result = m_table.create_swapchain_khr(
        m_device, &create_info, allocator, &swapchain);

    if (result == VK_SUCCESS) {
        return swapchain;
    } else {
        return core::make_unexpected(result);
    }
}

void Swapchain::destroy_swapchain(
    const VkSwapchainKHR swapchain, const VkAllocationCallbacks* allocator) const noexcept
{
    m_table.destroy_swapchain_khr(m_device, swapchain, allocator);
}

core::Expected<core::Array<VkImage>, VkResult> Swapchain::get_swapchain_images(
    const VkSwapchainKHR swapchain) const noexcept
{
    u32 count;
    VkResult result = m_table.get_swapchain_images_khr(
        m_device, swapchain, &count, nullptr);
    if (result != VK_SUCCESS) {
        return core::make_unexpected(result);
    }

    core::Array<VkImage> swapchain_images(count);
    result = m_table.get_swapchain_images_khr(
        m_device, swapchain, &count, swapchain_images.data());

    if (result == VK_SUCCESS) {
        return core::move(swapchain_images);
    } else {
        return core::make_unexpected(result);
    }
}

core::Expected<bool, VkResult> Swapchain::queue_present(
    const VkQueue queue, const VkPresentInfoKHR& present_info) const noexcept
{
    const VkResult result = m_table.queue_present_khr(queue, &present_info);
    switch (result) {
        case VK_SUCCESS:
            return false;
        case VK_SUBOPTIMAL_KHR:
            return true;
        default:
            return core::make_unexpected(result);
    }
}

const Swapchain::Table& Swapchain::get_table() const noexcept
{
    return m_table;
}

const char* Swapchain::name() noexcept
{
    return VK_KHR_SWAPCHAIN_EXTENSION_NAME;
}

} // namespace tundra::vulkan_rhi::loader::khr

#endif
