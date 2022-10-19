#pragma once
#include "core/std/shared_ptr.h"
#include "rhi/resources/texture.h"
#include "vulkan_allocator.h"
#include "vulkan_utils.h"

namespace tundra::vulkan_rhi {

class VulkanRawDevice;

///
class VulkanTexture {
private:
    core::SharedPtr<VulkanRawDevice> m_raw_device;
    core::SharedPtr<VulkanAllocator> m_allocator;
    rhi::TextureKind::Kind m_texture_kind;
    rhi::TextureFormat m_texture_format;
    rhi::TextureUsageFlags m_texture_usage;
    rhi::TextureTiling m_texture_tiling;
    VulkanAllocation<VkImage> m_allocation;
    VkImageView m_image_view;

public:
    VulkanTexture(
        core::SharedPtr<VulkanRawDevice> raw_device,
        core::SharedPtr<VulkanAllocator> allocator,
        const rhi::TextureCreateInfo& create_info) noexcept;
    ~VulkanTexture() noexcept;

    VulkanTexture(VulkanTexture&& rhs) noexcept;
    VulkanTexture& operator=(VulkanTexture&& rhs) noexcept;
    VulkanTexture(const VulkanTexture&) noexcept = delete;
    VulkanTexture& operator=(const VulkanTexture&) noexcept = delete;

public:
    [[nodiscard]] const rhi::TextureKind::Kind& get_texture_kind() const noexcept;
    [[nodiscard]] VkImage get_image() const noexcept;
    [[nodiscard]] VkImageView get_image_view() const noexcept;
    [[nodiscard]] rhi::Extent get_extent() const noexcept;
    [[nodiscard]] rhi::TextureFormat get_format() const noexcept;
    [[nodiscard]] rhi::TextureUsageFlags get_usage() const noexcept;
    [[nodiscard]] rhi::TextureTiling get_tiling() const noexcept;
    [[nodiscard]] u32 get_num_mips() const noexcept;
    [[nodiscard]] rhi::SampleCount get_sample_count() const noexcept;
    [[nodiscard]] u32 get_num_layers() const noexcept;
};

} // namespace tundra::vulkan_rhi
