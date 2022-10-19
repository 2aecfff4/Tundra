#include "resources/vulkan_texture_view.h"
#include "managers/managers.h"
#include "resources/vulkan_texture.h"
#include "vulkan_device.h"
#include "vulkan_helpers.h"

namespace tundra::vulkan_rhi {

VulkanTextureView::VulkanTextureView(
    core::SharedPtr<VulkanRawDevice> raw_device,
    const Managers& managers,
    const rhi::TextureViewCreateInfo& create_info) noexcept
    : m_raw_device(core::move(raw_device))
    , m_resource_tracker(managers.resource_tracker)
    , m_parent(create_info.texture)
{
    m_resource_tracker->add_reference(m_parent.get_handle().get_id());

    const auto [image, format] = *managers.texture_manager->with(
        create_info.texture.get_handle(), [&](const VulkanTexture& texture) {
            m_texture_format = texture.get_format();
            m_texture_usage = texture.get_usage();
            return core::make_tuple(texture.get_image(), texture.get_format());
        });

    const VkImageViewCreateInfo image_view_create_info {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = image,
        .viewType = core::visit(
            core::make_overload(
                [](const rhi::TextureViewSubresource::Texture1D&) {
                    return VK_IMAGE_VIEW_TYPE_1D;
                },
                [](const rhi::TextureViewSubresource::Texture2D&) {
                    return VK_IMAGE_VIEW_TYPE_2D;
                },
                [](const rhi::TextureViewSubresource::Texture3D&) {
                    return VK_IMAGE_VIEW_TYPE_3D;
                },
                [](const rhi::TextureViewSubresource::TextureCube&) {
                    return VK_IMAGE_VIEW_TYPE_CUBE;
                }),
            create_info.subresource),
        .format = helpers::map_texture_format(format),
        .components =
            VkComponentMapping {
                .r = VK_COMPONENT_SWIZZLE_R,
                .g = VK_COMPONENT_SWIZZLE_G,
                .b = VK_COMPONENT_SWIZZLE_B,
                .a = VK_COMPONENT_SWIZZLE_A,
            },
        .subresourceRange = core::visit(
            core::make_overload(
                [&](const rhi::TextureViewSubresource::Texture1D& texture) {
                    return VkImageSubresourceRange {
                        .aspectMask = helpers::map_texture_aspect(
                            rhi::get_texture_format_desc(format).aspect),
                        .baseMipLevel = texture.first_mip,
                        .levelCount = texture.mip_count,
                        .baseArrayLayer = 0,
                        .layerCount = 1,
                    };
                },
                [&](const rhi::TextureViewSubresource::Texture2D& texture) {
                    return VkImageSubresourceRange {
                        .aspectMask = helpers::map_texture_aspect(
                            rhi::get_texture_format_desc(format).aspect),
                        .baseMipLevel = texture.first_mip,
                        .levelCount = texture.mip_count,
                        .baseArrayLayer = 0,
                        .layerCount = 1,
                    };
                },
                [&](const rhi::TextureViewSubresource::Texture3D& texture) {
                    return VkImageSubresourceRange {
                        .aspectMask = helpers::map_texture_aspect(
                            rhi::get_texture_format_desc(format).aspect),
                        .baseMipLevel = texture.first_mip,
                        .levelCount = texture.mip_count,
                        .baseArrayLayer = texture.first_layer,
                        .layerCount = texture.layer_count,
                    };
                },
                [&](const rhi::TextureViewSubresource::TextureCube& texture) {
                    return VkImageSubresourceRange {
                        .aspectMask = helpers::map_texture_aspect(
                            rhi::get_texture_format_desc(format).aspect),
                        .baseMipLevel = texture.first_mip,
                        .levelCount = texture.mip_count,
                        .baseArrayLayer = texture.first_layer,
                        .layerCount = texture.layer_count,
                    };
                }),
            create_info.subresource),
    };

    m_image_view = vulkan_map_result(
        m_raw_device->get_device().create_image_view(image_view_create_info, nullptr),
        "`create_image_view` failed");

    if (!create_info.name.empty()) {
        const auto image_view_name = fmt::format(create_info.name);

        helpers::set_object_name(
            m_raw_device,
            reinterpret_cast<u64>(m_image_view),
            VK_OBJECT_TYPE_IMAGE_VIEW,
            image_view_name.c_str());
    }
}

VulkanTextureView::~VulkanTextureView() noexcept
{
    if (m_image_view != VK_NULL_HANDLE) {
        m_resource_tracker->remove_reference(m_parent.get_handle().get_id());

        m_raw_device->get_device().destroy_image_view(m_image_view, nullptr);
        m_image_view = VK_NULL_HANDLE;
    }
}

VulkanTextureView::VulkanTextureView(VulkanTextureView&& rhs) noexcept
    : m_raw_device(core::move(rhs.m_raw_device))
    , m_resource_tracker(core::move(rhs.m_resource_tracker))
    , m_texture_format(rhs.m_texture_format)
    , m_texture_usage(rhs.m_texture_usage)
    , m_parent(rhs.m_parent)
    , m_image_view(core::exchange(rhs.m_image_view, VK_NULL_HANDLE))
{
}

VulkanTextureView& VulkanTextureView::operator=(VulkanTextureView&& rhs) noexcept
{
    if (this != &rhs) {
        m_raw_device = core::move(rhs.m_raw_device);
        m_resource_tracker = core::move(rhs.m_resource_tracker);
        m_texture_format = rhs.m_texture_format;
        m_texture_usage = rhs.m_texture_usage;
        m_parent = rhs.m_parent;
        m_image_view = core::exchange(rhs.m_image_view, VK_NULL_HANDLE);
    }
    return *this;
}

VkImageView VulkanTextureView::get_image_view() const noexcept
{
    return m_image_view;
}

rhi::TextureFormat VulkanTextureView::get_format() const noexcept
{
    return m_texture_format;
}

rhi::TextureUsageFlags VulkanTextureView::get_usage() const noexcept
{
    return m_texture_usage;
}

} // namespace tundra::vulkan_rhi
