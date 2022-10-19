#include "loader/extensions/khr/android_surface.h"

#if defined(VK_KHR_android_surface)
#include "loader/instance.h"

namespace tundra::vulkan_rhi::loader::khr {

AndroidSurface::AndroidSurface(const loader::Instance& instance) noexcept
    : m_instance(instance.get_handle())
{
    const auto load =
        [instance = m_instance,
         get_instance_proc_addr = instance.get_table().get_instance_proc_addr](
            const char* name) {
            return get_instance_proc_addr(instance, name);
        };

    // clang-format off
    m_table.create_android_surface_khr = reinterpret_cast<PFN_vkCreateAndroidSurfaceKHR>(load("vkCreateAndroidSurfaceKHR"));
    // clang-format on
}

core::Unexpected<VkSurfaceKHR, VkResult> AndroidSurface::create_android_surface(
    const VkAndroidSurfaceCreateInfoKHR& create_info,
    const VkAllocationCallbacks* allocator) const noexcept
{
    VkSurfaceKHR surface;
    const auto result = m_table.create_android_surface_khr(
        m_instance, &create_info, allocator, &surface);
    if (result == VK_SUCCESS) {
        return surface;
    } else {
        return core::make_unexpected(result);
    }
}

const AndroidSurface::Table& AndroidSurface::get_table() const noexcept
{
    return m_table;
}

const char* AndroidSurface::name() noexcept
{
    return VK_KHR_ANDROID_SURFACE_EXTENSION_NAME;
}

} // namespace tundra::vulkan_rhi::loader::khr

#endif
