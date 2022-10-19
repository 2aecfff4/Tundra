#include "loader/extensions/ext/debug_utils.h"
#include "loader/entry.h"
#include "loader/instance.h"

namespace tundra::vulkan_rhi::loader::ext {

DebugUtils::DebugUtils(const loader::Instance& instance) noexcept
    : m_instance(instance.get_handle())
{

    const auto load =
        [instance = m_instance,
         get_instance_proc_addr = instance.get_table().get_instance_proc_addr](
            const char* name) {
            return get_instance_proc_addr(instance, name);
        };

    // clang-format off
    m_table.cmd_begin_debug_utils_label_ext = reinterpret_cast<PFN_vkCmdBeginDebugUtilsLabelEXT>(load("vkCmdBeginDebugUtilsLabelEXT"));
    m_table.cmd_end_debug_utils_label_ext = reinterpret_cast<PFN_vkCmdEndDebugUtilsLabelEXT>(load("vkCmdEndDebugUtilsLabelEXT"));
    m_table.cmd_insert_debug_utils_label_ext = reinterpret_cast<PFN_vkCmdInsertDebugUtilsLabelEXT>(load("vkCmdInsertDebugUtilsLabelEXT"));
    m_table.create_debug_utils_messenger_ext = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(load("vkCreateDebugUtilsMessengerEXT"));
    m_table.destroy_debug_utils_messenger_ext = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(load("vkDestroyDebugUtilsMessengerEXT"));
    m_table.queue_begin_debug_utils_label_ext = reinterpret_cast<PFN_vkQueueBeginDebugUtilsLabelEXT>(load("vkQueueBeginDebugUtilsLabelEXT"));
    m_table.queue_end_debug_utils_label_ext = reinterpret_cast<PFN_vkQueueEndDebugUtilsLabelEXT>(load("vkQueueEndDebugUtilsLabelEXT"));
    m_table.queue_insert_debug_utils_label_ext = reinterpret_cast<PFN_vkQueueInsertDebugUtilsLabelEXT>(load("vkQueueInsertDebugUtilsLabelEXT"));
    m_table.set_debug_utils_object_name_ext = reinterpret_cast<PFN_vkSetDebugUtilsObjectNameEXT>(load("vkSetDebugUtilsObjectNameEXT"));
    m_table.set_debug_utils_object_tag_ext = reinterpret_cast<PFN_vkSetDebugUtilsObjectTagEXT>(load("vkSetDebugUtilsObjectTagEXT"));
    m_table.submit_debug_utils_message_ext = reinterpret_cast<PFN_vkSubmitDebugUtilsMessageEXT>(load("vkSubmitDebugUtilsMessageEXT"));
    // clang-format on
}

void DebugUtils::cmd_begin_debug_utils_label(
    const VkCommandBuffer command_buffer,
    const VkDebugUtilsLabelEXT& label) const noexcept
{
    m_table.cmd_begin_debug_utils_label_ext(command_buffer, &label);
}

void DebugUtils::cmd_end_debug_utils_label(
    const VkCommandBuffer command_buffer) const noexcept
{
    m_table.cmd_end_debug_utils_label_ext(command_buffer);
}

core::Expected<VkDebugUtilsMessengerEXT, VkResult>
    DebugUtils::create_debug_utils_messenger(
        const VkDebugUtilsMessengerCreateInfoEXT& create_info,
        const VkAllocationCallbacks* allocator) const noexcept
{
    VkDebugUtilsMessengerEXT messenger;
    const VkResult result = m_table.create_debug_utils_messenger_ext(
        m_instance, &create_info, allocator, &messenger);
    if (result == VK_SUCCESS) {
        return messenger;
    } else {
        return core::make_unexpected(result);
    }
}

void DebugUtils::destroy_debug_utils_messenger(
    const VkDebugUtilsMessengerEXT messenger,
    const VkAllocationCallbacks* allocator) const noexcept
{
    m_table.destroy_debug_utils_messenger_ext(m_instance, messenger, allocator);
}

VkResult DebugUtils::set_debug_utils_object_name(
    const VkDevice device,
    const VkDebugUtilsObjectNameInfoEXT& object_name_info) const noexcept
{
    return m_table.set_debug_utils_object_name_ext(device, &object_name_info);
}

const DebugUtils::Table& DebugUtils::get_table() const noexcept
{
    return m_table;
}

const char* DebugUtils::name() noexcept
{
    return VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
}

} // namespace tundra::vulkan_rhi::loader::ext
