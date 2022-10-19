#include "resources/vulkan_swapchain.h"
#include "core/logger.h"
#include "core/profiler.h"
#include "rhi/config.h"
#include "vulkan_device.h"
#include "vulkan_helpers.h"
#include "vulkan_instance.h"
#include <algorithm>

namespace tundra::vulkan_rhi {

VulkanSwapchain::VulkanSwapchain(
    core::SharedPtr<VulkanRawDevice> raw_device,
    const rhi::SwapchainCreateInfo& create_info) noexcept
    : m_raw_device(core::move(raw_device))
    , m_window_handle(create_info.window_handle)
{
    TNDR_PROFILER_TRACE("VulkanSwapchain::VulkanSwapchain");

    m_surface = m_raw_device->get_instance()->create_surface(m_window_handle);

    const bool supports_presentation = vulkan_map_result(
        m_raw_device->get_extensions().surface.get_physical_device_surface_support(
            m_raw_device->get_physical_device(),
            core::get<u32>(m_raw_device->get_queues().present_queue),
            m_surface),
        "`get_physical_device_surface_support` failed");
    tndr_assert(supports_presentation, "");

    const core::Array<VkSurfaceFormatKHR> surface_formats = vulkan_map_result(
        m_raw_device->get_extensions().surface.get_physical_device_surface_formats(
            m_raw_device->get_physical_device(), m_surface),
        "`get_physical_device_surface_formats` failed");

    m_surface_format = [&] {
        const auto find = [&](const VkFormat format) {
            return std::find_if(
                surface_formats.begin(),
                surface_formats.end(),
                [&](const VkSurfaceFormatKHR& surface_format) {
                    return (surface_format.format == format) &&
                           (surface_format.colorSpace ==
                            VK_COLOR_SPACE_SRGB_NONLINEAR_KHR);
                });
        };

        if (const auto it = find(VK_FORMAT_R16G16B16A16_UNORM);
            it != surface_formats.end()) {
            return *it;
        }

        if (const auto it = find(VK_FORMAT_A2R10G10B10_UNORM_PACK32);
            it != surface_formats.end()) {
            return *it;
        }

        if (const auto it = find(VK_FORMAT_B8G8R8A8_UNORM); it != surface_formats.end()) {
            return *it;
        }

        // At this point we can say that device is not supported...
        core::panic("The device does not support `VK_FORMAT_B8G8R8A8_UNORM` format!");
    }();

    const VkSurfaceCapabilitiesKHR surface_capabilities = vulkan_map_result(
        m_raw_device->get_extensions().surface.get_physical_device_surface_capabilities(
            m_raw_device->get_physical_device(), m_surface),
        "`get_physical_device_surface_capabilities` failed");

    const VkCompositeAlphaFlagBitsKHR composite_alfa_flags[] = {
        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
        VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
        VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
    };

    const VkCompositeAlphaFlagBitsKHR composite_alfa_flag = [&] {
        for (const VkCompositeAlphaFlagBitsKHR& composite_alfa_flag :
             composite_alfa_flags) {
            if (surface_capabilities.supportedCompositeAlpha & composite_alfa_flag) {
                return composite_alfa_flag;
            }
        }
        return VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    }();

    const u32 min_image_count = surface_capabilities.minImageCount;
    const u32 max_image_count = surface_capabilities.maxImageCount;
    const u32 image_count = std::clamp(
        rhi::config::MAX_FRAMES_IN_FLIGHT, min_image_count, max_image_count);

    if (image_count != rhi::config::MAX_FRAMES_IN_FLIGHT) {
        tndr_warn(
            "Swapchain supports only {} images. We want {} images.",
            max_image_count,
            image_count);
    }

    const core::Array<VkPresentModeKHR> present_modes = vulkan_map_result(
        m_raw_device->get_extensions().surface.get_physical_device_surface_present_modes(
            m_raw_device->get_physical_device(), m_surface),
        "`get_physical_device_surface_present_modes` failed");

    const bool is_fifo_relaxed_supported = std::any_of(
        present_modes.begin(), present_modes.end(), [](auto mode) {
            return mode == VK_PRESENT_MODE_FIFO_RELAXED_KHR;
        });

    const VkSurfaceTransformFlagBitsKHR pre_transform = [&] {
        if ((surface_capabilities.supportedTransforms &
             VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) ==
            VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) {
            return VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
        } else {
            return surface_capabilities.currentTransform;
        }
    }();

    const VkExtent2D swapchain_extent = [&] {
        // #BUG: AMD driver(22.7.1) returns `minImageExtent = (120,0)` but the spec says:
        // 1. "imageExtent must be between minImageExtent and maxImageExtent, inclusive, where minImageExtent
        //    and maxImageExtent are members of the VkSurfaceCapabilitiesKHR structure returned by
        //    vkGetPhysicalDeviceSurfaceCapabilitiesKHR for the surface".
        //    https://vulkan.lunarg.com/doc/view/1.3.204.1/windows/1.3-extensions/vkspec.html#VUID-VkSwapchainCreateInfoKHR-imageExtent-01274
        // 2. "imageExtent members width and height must both be non-zero"
        //    https://vulkan.lunarg.com/doc/view/1.3.204.1/windows/1.3-extensions/vkspec.html#VUID-VkSwapchainCreateInfoKHR-imageExtent-01689
        //
        // So, the `minImageExtent` returned by `vkGetPhysicalDeviceSurfaceCapabilitiesKHR` is invalid.

        const u32 min_width = surface_capabilities.minImageExtent.width;
        const u32 min_height = surface_capabilities.minImageExtent.height;

        return VkExtent2D {
            .width = math::clamp(
                surface_capabilities.currentExtent.width,
                min_width,
                surface_capabilities.maxImageExtent.width),
            .height = math::clamp(
                surface_capabilities.currentExtent.height,
                min_height,
                surface_capabilities.maxImageExtent.height),
        };
    }();

    const VkSwapchainCreateInfoKHR swapchain_create_info {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = m_surface,
        .minImageCount = image_count,
        .imageFormat = m_surface_format.format,
        .imageColorSpace = m_surface_format.colorSpace,
        .imageExtent = swapchain_extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                      VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .preTransform = pre_transform,
        .compositeAlpha = composite_alfa_flag,
        .presentMode =
            (is_fifo_relaxed_supported ? VK_PRESENT_MODE_FIFO_RELAXED_KHR
                                       : VK_PRESENT_MODE_FIFO_KHR),
        .clipped = true,
    };

    m_swapchain = vulkan_map_result(
        m_raw_device->get_extensions().swapchain.create_swapchain(
            swapchain_create_info, nullptr),
        "`create_swapchain` failed");

    const core::Array<VkImage> swapchain_images = vulkan_map_result(
        m_raw_device->get_extensions().swapchain.get_swapchain_images(m_swapchain),
        "`get_swapchain_images` failed");

    std::transform(
        swapchain_images.begin(),
        swapchain_images.end(),
        std::back_inserter(m_attachments),
        [&, i = 0u](const VkImage& image) mutable {
            const auto name = fmt::format(
                "{} | Swapchain image: {}", create_info.name, i);
            helpers::set_object_name(
                m_raw_device,
                reinterpret_cast<u64>(image),
                VK_OBJECT_TYPE_IMAGE,
                name.c_str());

            i += 1;
            return SwapchainAttachment {
                .image = image,
            };
        });

    m_image_available_semaphores.resize(m_attachments.size());
    for (usize i = 0; i < m_attachments.size(); ++i) {
        const VkSemaphoreCreateInfo semaphore_create_info {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        };

        m_image_available_semaphores[i] = vulkan_map_result(
            m_raw_device->get_device().create_semaphore(semaphore_create_info, nullptr),
            "`create_semaphore` failed");
    }

    tndr_info(
        "Swapchain {:X} has been created. Width: {} Height: {}",
        reinterpret_cast<u64>(m_swapchain),
        swapchain_extent.width,
        swapchain_extent.height);
}

VulkanSwapchain::~VulkanSwapchain() noexcept
{
    TNDR_PROFILER_TRACE("VulkanSwapchain::~VulkanSwapchain");

    if ((m_surface != VK_NULL_HANDLE) && (m_swapchain != VK_NULL_HANDLE)) {
        // Swapchain images and semaphores may be still in use,
        // but we keep a reference to a swapchain in `ResourceTracker`.
        // In other words, this function will be only called when all references
        // to this swapchain dropped, so we can safely destroy all resources,
        // without need for call to `vkDeviceWaitIdle`.
        m_attachments.clear();

        m_raw_device->get_extensions().swapchain.destroy_swapchain(m_swapchain, nullptr);
        m_raw_device->get_extensions().surface.destroy_surface(m_surface, nullptr);

        for (const VkSemaphore& semaphore : m_image_available_semaphores) {
            m_raw_device->get_device().destroy_semaphore(semaphore, nullptr);
        }
        m_image_available_semaphores.clear();

        tndr_info(
            "Swapchain {:X} has been destroyed. Swap index: {}",
            reinterpret_cast<u64>(m_swapchain),
            m_swap_index);

        m_surface_format = VkSurfaceFormatKHR {};
        m_swap_index = 0;
    }
}

VulkanSwapchain::VulkanSwapchain(VulkanSwapchain&& rhs) noexcept
    : m_raw_device(core::move(rhs.m_raw_device))
    , m_window_handle(core::move(m_window_handle))
    , m_surface(core::exchange(rhs.m_surface, VK_NULL_HANDLE))
    , m_swapchain(core::exchange(rhs.m_swapchain, VK_NULL_HANDLE))
    , m_surface_format(rhs.m_surface_format)
    , m_attachments(core::move(m_attachments))
    , m_image_available_semaphores(core::move(m_image_available_semaphores))
    , m_swap_index(core::exchange(rhs.m_swap_index, 0))
{
}

VulkanSwapchain& tundra::vulkan_rhi::VulkanSwapchain::operator=(
    VulkanSwapchain&& rhs) noexcept
{
    if (this != &rhs) {
        m_raw_device = core::move(rhs.m_raw_device);
        m_window_handle = core::move(rhs.m_window_handle);
        m_surface = core::exchange(rhs.m_surface, VK_NULL_HANDLE);
        m_swapchain = core::exchange(rhs.m_swapchain, VK_NULL_HANDLE);
        m_surface_format = rhs.m_surface_format;
        m_attachments = core::move(rhs.m_attachments);
        m_image_available_semaphores = core::move(rhs.m_image_available_semaphores);
        m_swap_index = core::exchange(rhs.m_swap_index, 0);
    }
    return *this;
}

core::Tuple<u32, VkSemaphore, SwapchainAttachment&>
    VulkanSwapchain::acquire_image_index() noexcept
{
    TNDR_PROFILER_TRACE("VulkanSwapchain::acquire_image_index");

    const u64 current_swap_index = (m_swap_index + 1) % m_attachments.size();
    const VkSemaphore semaphore = m_image_available_semaphores[current_swap_index];

    const auto& [idx, _] = vulkan_map_result(
        m_raw_device->get_extensions().swapchain.acquire_next_image(
            m_swapchain, UINT64_MAX, semaphore, VK_NULL_HANDLE),
        "`swapchain.acquire_next_image` failed");

    m_swap_index += 1;
    SwapchainAttachment& attachment = m_attachments[idx];
    return core::forward_as_tuple(idx, semaphore, attachment);
}

VkSwapchainKHR VulkanSwapchain::get_swapchain() const noexcept
{
    return m_swapchain;
}

} // namespace tundra::vulkan_rhi
