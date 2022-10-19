#pragma once
#include "core/std/containers/array.h"
#include "core/std/option.h"
#include "loader/extensions/ext/debug_utils.h"
#include "loader/instance.h"
#include "platform/window_native_handle.h"

namespace tundra::vulkan_rhi::loader {
class Entry;
} // namespace tundra::vulkan_rhi::loader

namespace tundra::vulkan_rhi {

struct DebugMessenger {
    loader::ext::DebugUtils debug_utils_loader;
    VkDebugUtilsMessengerEXT messenger;
};

class VulkanInstance {
private:
    loader::Instance m_instance;
    core::Option<DebugMessenger> m_debug_messenger;
    core::Array<VkExtensionProperties> m_instance_extensions;

public:
    VulkanInstance(const loader::Entry& entry) noexcept;
    ~VulkanInstance() noexcept;

public:
    [[nodiscard]] VkSurfaceKHR create_surface(
        const platform::WindowNativeHandle& window_handle) noexcept;

public:
    [[nodiscard]] const loader::Instance& get_instance() const noexcept;
    [[nodiscard]] core::Option<const loader::ext::DebugUtils*> get_debug_utils()
        const noexcept;
};

} // namespace tundra::vulkan_rhi
