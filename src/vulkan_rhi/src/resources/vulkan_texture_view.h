#pragma once
#include "core/std/shared_ptr.h"
#include "rhi/resources/resource_tracker.h"
#include "rhi/resources/texture.h"
#include "vulkan_utils.h"

namespace tundra::vulkan_rhi {

class VulkanRawDevice;
struct Managers;

///
class VulkanTextureView {
private:
    core::SharedPtr<VulkanRawDevice> m_raw_device;
    core::SharedPtr<rhi::ResourceTracker> m_resource_tracker;
    rhi::TextureFormat m_texture_format;
    rhi::TextureUsageFlags m_texture_usage;
    rhi::TextureHandle m_parent;
    VkImageView m_image_view;

public:
    VulkanTextureView(
        core::SharedPtr<VulkanRawDevice> raw_device,
        const Managers& managers,
        const rhi::TextureViewCreateInfo& create_info) noexcept;
    ~VulkanTextureView() noexcept;

    VulkanTextureView(VulkanTextureView&& rhs) noexcept;
    VulkanTextureView& operator=(VulkanTextureView&& rhs) noexcept;
    VulkanTextureView(const VulkanTextureView&) noexcept = delete;
    VulkanTextureView& operator=(const VulkanTextureView&) noexcept = delete;

public:
    [[nodiscard]] VkImageView get_image_view() const noexcept;
    [[nodiscard]] rhi::TextureFormat get_format() const noexcept;
    [[nodiscard]] rhi::TextureUsageFlags get_usage() const noexcept;
};

} // namespace tundra::vulkan_rhi
