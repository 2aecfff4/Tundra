#include "vulkan_instance.h"
#include "core/profiler.h"
#include "core/std/assert.h"
#include "core/std/containers/string.h"
#include "core/std/panic.h"
#include "core/std/utils.h"
#include "loader/entry.h"
#include "loader/extensions/ext/debug_utils.h"
#include "loader/extensions/khr/android_surface.h"
#include "loader/extensions/khr/surface.h"
#include "loader/extensions/khr/wayland_surface.h"
#include "loader/extensions/khr/win32_surface.h"
#include "loader/extensions/khr/xcb_surface.h"
#include "loader/extensions/khr/xlib_surface.h"
#include "vulkan_utils.h"
#include <fmt/core.h>
#include <algorithm>
#include <cstring>

namespace tundra::vulkan_rhi {

[[nodiscard]] static const char* message_severity_to_str(
    const VkDebugUtilsMessageSeverityFlagBitsEXT message_severity) noexcept
{
    switch (message_severity) {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            return "VERBOSE";
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            return "INFO";
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            return "WARNING";
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            return "ERROR";
        default:
            return "";
    }
}

VKAPI_ATTR VkBool32 VKAPI_CALL debug_utils_messenger_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_type,
    const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
    [[maybe_unused]] void* user_data)
{
    const i32 message_id_number = callback_data->messageIdNumber;
    const char* message_id_name = [&]() {
        if (callback_data->pMessageIdName != nullptr) {
            return callback_data->pMessageIdName;
        } else {
            return "";
        }
    }();

    // VUID-VkDebugUtilsMessengerCallbackDataEXT-pMessage-parameter
    // pMessage must be a null-terminated UTF-8 string
    const char* message = callback_data->pMessage;

    const core::String message_type_str = [&] {
        core::String temp;

        if ((message_type & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT) != 0) {
            temp += (temp.empty() ? "GENERAL" : " | GENERAL");
        }
        if ((message_type & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) != 0) {
            temp += (temp.empty() ? "VALIDATION" : " | VALIDATION");
        }
        if ((message_type & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT) != 0) {
            temp += (temp.empty() ? "PERFORMANCE" : " | PERFORMANCE");
        }

        return temp;
    }();

    fmt::print(
        "{}:{}\n[{} ({})] : {}\n",
        message_severity_to_str(message_severity),
        message_type_str,
        message_id_name,
        message_id_number,
        message);

    return false;
}

[[nodiscard]] loader::Instance load_instance(const loader::Entry& entry)
{
    TNDR_PROFILER_TRACE("load_instance");

    const core::Array<VkExtensionProperties> instance_extensions = vulkan_map_result(
        entry.enumerate_instance_extension_properties(),
        "Unable to enumerate instance extensions");

    core::Array<const char*> extensions {
        loader::khr::Surface::name(),
        loader::ext::DebugUtils::name(),
    };

#if TNDR_PLATFORM_WINDOWS
    extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif TNDR_PLATFORM_LINUX
    extensions.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
    extensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
    extensions.push_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
#elif TNDR_PLATFORM_ANDROID
    extensions.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#endif

    std::erase_if(extensions, [&instance_extensions](const char* extension) {
        return std::none_of(
            instance_extensions.begin(),
            instance_extensions.end(),
            [extension](const auto& instance_extension) {
                return std::strcmp(extension, instance_extension.extensionName) == 0;
            });
    });

    const core::Array<VkLayerProperties> instance_layers = vulkan_map_result(
        entry.enumerate_instance_layer_properties(),
        "Unable to enumerate instance layers");

    // #TODO: Add switch or something for validation layers. For now just always enable it.
    core::Array<const char*> layers {
        "VK_LAYER_KHRONOS_validation",
    };

    std::erase_if(layers, [&instance_layers](const char* layer) {
        return std::none_of(
            instance_layers.begin(),
            instance_layers.end(),
            [layer](const auto& instance_layer) {
                return std::strcmp(layer, instance_layer.layerName) == 0;
            });
    });

    const VkApplicationInfo application_info {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "tundra",
        .apiVersion = VK_API_VERSION_1_3,
    };

    const VkInstanceCreateInfo create_info {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &application_info,
        .enabledLayerCount = static_cast<u32>(layers.size()),
        .ppEnabledLayerNames = layers.data(),
        .enabledExtensionCount = static_cast<u32>(extensions.size()),
        .ppEnabledExtensionNames = extensions.data(),
    };

    const loader::Instance instance = vulkan_map_result(
        entry.create_instance(create_info, nullptr), "`entry.create_instance` failed!");

    return instance;
}

[[nodiscard]] core::Option<DebugMessenger> load_debug_messenger(
    const loader::Entry& entry, const loader::Instance& instance) noexcept
{
    TNDR_PROFILER_TRACE("load_debug_messenger");

    const core::Array<VkExtensionProperties> instance_extensions = vulkan_map_result(
        entry.enumerate_instance_extension_properties(),
        "Unable to enumerate instance extensions");

    const bool has_debug_extension = std::any_of(
        instance_extensions.begin(),
        instance_extensions.end(),
        [](const auto& instance_extension) {
            return std::strcmp(
                       instance_extension.extensionName,
                       loader::ext::DebugUtils::name()) == 0;
        });

    if (has_debug_extension) {
        const loader::ext::DebugUtils debug_utils_loader(instance);

        const VkDebugUtilsMessengerCreateInfoEXT create_info {
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
            .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
            .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
            .pfnUserCallback = &debug_utils_messenger_callback,
        };

        const VkDebugUtilsMessengerEXT messenger = vulkan_map_result(
            debug_utils_loader.create_debug_utils_messenger(create_info, nullptr),
            "`debug_utils_loader.create_debug_utils_messenger` failed");

        return DebugMessenger {
            .debug_utils_loader = debug_utils_loader,
            .messenger = messenger,
        };
    } else {
        return std::nullopt;
    }
}

VulkanInstance::VulkanInstance(const loader::Entry& entry) noexcept
    : m_instance(load_instance(entry))
    , m_debug_messenger(load_debug_messenger(entry, m_instance))
{
    TNDR_PROFILER_TRACE("VulkanInstance::VulkanInstance");

    m_instance_extensions = vulkan_map_result(
        entry.enumerate_instance_extension_properties(),
        "Unable to enumerate instance extensions");
}

VulkanInstance::~VulkanInstance() noexcept
{
    TNDR_PROFILER_TRACE("VulkanInstance::~VulkanInstance");

    if (m_debug_messenger.has_value()) {
        const auto& [debug_utils_loader, messenger] = *m_debug_messenger;
        debug_utils_loader.destroy_debug_utils_messenger(messenger, nullptr);
    }

    m_instance.destroy_instance(nullptr);
}

VkSurfaceKHR VulkanInstance::create_surface(
    const platform::WindowNativeHandle& window_handle) noexcept
{
    TNDR_PROFILER_TRACE("VulkanInstance::create_surface");

    const auto has_extension = [&](const char* name) {
        return std::any_of(
            m_instance_extensions.begin(),
            m_instance_extensions.end(),
            [name](const auto& instance_extension) {
                return std::strcmp(instance_extension.extensionName, name) == 0;
            });
    };

    const VkSurfaceKHR surface = core::visit(
        core::make_overload(
#if TNDR_PLATFORM_WINDOWS
            [&](const platform::Win32Handle& handle) {
                tndr_assert(handle.hwnd != nullptr, "`hwnd` must not be nullptr.");
                if (!has_extension(loader::khr::Win32Surface::name())) {
                    core::panic("Vulkan driver does not support "
                                "`VK_KHR_win32_surface` extension.");
                }

                const loader::khr::Win32Surface win32_loader(m_instance);

                return vulkan_map_result(
                    win32_loader.create_win32_surface(
                        VkWin32SurfaceCreateInfoKHR {
                            .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
                            .hinstance = reinterpret_cast<HINSTANCE>(handle.hinstance),
                            .hwnd = reinterpret_cast<HWND>(handle.hwnd),
                        },
                        nullptr),
                    "`create_win32_surface` failed");
            }
#elif TNDR_PLATFORM_LINUX
            [&](const platform::XlibHandle& handle) {
                if (!has_extension(loader::khr::XLibSurface::name())) {
                    core::panic("Vulkan driver does not support "
                                "`VK_KHR_xlib_surface` extension.");
                }

                const loader::khr::XLibSurface xlib_loader(m_instance);

                return vulkan_map_result(
                    xlib_loader.create_xlib_surface(
                        VkXlibSurfaceCreateInfoKHR {
                            .sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR,
                            .dpy = reinterpret_cast<Display*>(handle.display),
                            .window = reinterpret_cast<Window>(handle.window),
                        },
                        nullptr),
                    "`create_xlib_surface` failed");
            },
            [&](const platform::XcbHandle& handle) {
                if (!has_extension(loader::khr::XcbSurface::name())) {
                    core::panic("Vulkan driver does not support "
                                "`VK_KHR_xcb_surface` extension.");
                }

                const loader::khr::XcbSurface xcb_loader(m_instance);

                return vulkan_map_result(
                    xcb_loader.create_xcb_surface(
                        VkXcbSurfaceCreateInfoKHR {
                            .sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR,
                            .connection = reinterpret_cast<xcb_connection_t*>(
                                handle.connection),
                            .window = reinterpret_cast<xcb_window_t>(handle.window),
                        },
                        nullptr),
                    "`create_xcb_surface` failed");
            },
            [&](const platform::WaylandHandle& handle) {
                if (!has_extension(loader::khr::WaylandSurface::name())) {
                    core::panic("Vulkan driver does not support "
                                "`VK_KHR_wayland_surface` extension.");
                }

                const loader::khr::WaylandSurface wayland_loader(m_instance);

                return vulkan_map_result(
                    wayland_loader.create_xcb_surface(
                        VkWaylandSurfaceCreateInfoKHR {
                            .sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR,
                            .display = reinterpret_cast<struct wl_display*>(
                                handle.display),
                            .surface = reinterpret_cast<struct wl_surface*>(
                                handle.surface),
                        },
                        nullptr),
                    "`create_wayland_surface` failed");
            }
#elif TNDR_PLATFORM_ANDROID
            [&](const platform::AndroidNdkHandle& handle) {
                if (!has_extension(loader::khr::AndroidSurface::name())) {
                    core::panic("Vulkan driver does not support "
                                "`VK_KHR_android_surface` extension.");
                }

                const loader::khr::AndroidSurface android_loader(m_instance);

                return vulkan_map_result(
                    android_loader.create_android_surface(
                        VkAndroidSurfaceCreateInfoKHR {
                            .sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR,
                            .window = reinterpret_cast<struct ANativeWindow*>(
                                handle.window),
                        },
                        nullptr),
                    "`create_android_surface` failed");
            }
#else
            [](const auto&) { core::panic("Unsupported platform!"); }
#endif
            ),
        window_handle);

    return surface;
}

const loader::Instance& VulkanInstance::get_instance() const noexcept
{
    return m_instance;
}

core::Option<const loader::ext::DebugUtils*> VulkanInstance::get_debug_utils()
    const noexcept
{
    if (m_debug_messenger) {
        return &m_debug_messenger->debug_utils_loader;
    } else {
        return std::nullopt;
    }
}

} // namespace tundra::vulkan_rhi
