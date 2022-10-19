#pragma once
#include "core/core.h"
#include "core/std/expected.h"
#include "vulkan_utils.h"

namespace tundra::vulkan_rhi::loader {
class Instance;
} // namespace tundra::vulkan_rhi::loader

namespace tundra::vulkan_rhi::loader::ext {

class DebugUtils {
private:
    VkInstance m_instance;

    struct Table {
        // clang-format off
        PFN_vkCmdBeginDebugUtilsLabelEXT cmd_begin_debug_utils_label_ext;
        PFN_vkCmdEndDebugUtilsLabelEXT cmd_end_debug_utils_label_ext;
        PFN_vkCmdInsertDebugUtilsLabelEXT cmd_insert_debug_utils_label_ext;
        PFN_vkCreateDebugUtilsMessengerEXT create_debug_utils_messenger_ext;
        PFN_vkDestroyDebugUtilsMessengerEXT destroy_debug_utils_messenger_ext;
        PFN_vkQueueBeginDebugUtilsLabelEXT queue_begin_debug_utils_label_ext;
        PFN_vkQueueEndDebugUtilsLabelEXT queue_end_debug_utils_label_ext;
        PFN_vkQueueInsertDebugUtilsLabelEXT queue_insert_debug_utils_label_ext;
        PFN_vkSetDebugUtilsObjectNameEXT set_debug_utils_object_name_ext;
        PFN_vkSetDebugUtilsObjectTagEXT set_debug_utils_object_tag_ext;
        PFN_vkSubmitDebugUtilsMessageEXT submit_debug_utils_message_ext;
        // clang-format on
    } m_table;

public:
    DebugUtils(const loader::Instance& instance) noexcept;

public:
    void cmd_begin_debug_utils_label(
        const VkCommandBuffer command_buffer,
        const VkDebugUtilsLabelEXT& label) const noexcept;
    void cmd_end_debug_utils_label(const VkCommandBuffer command_buffer) const noexcept;

public:
    [[nodiscard]] core::Expected<VkDebugUtilsMessengerEXT, VkResult>
        create_debug_utils_messenger(
            const VkDebugUtilsMessengerCreateInfoEXT& create_info,
            const VkAllocationCallbacks* allocator) const noexcept;
    void destroy_debug_utils_messenger(
        const VkDebugUtilsMessengerEXT messenger,
        const VkAllocationCallbacks* allocator) const noexcept;
    [[nodiscard]] VkResult set_debug_utils_object_name(
        const VkDevice device,
        const VkDebugUtilsObjectNameInfoEXT& object_name_info) const noexcept;

public:
    [[nodiscard]] const Table& get_table() const noexcept;
    [[nodiscard]] static const char* name() noexcept;
};

} // namespace tundra::vulkan_rhi::loader::ext
