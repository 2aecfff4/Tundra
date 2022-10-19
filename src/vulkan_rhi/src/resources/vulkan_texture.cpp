#include "resources/vulkan_texture.h"
#include "core/profiler.h"
#include "core/std/utils.h"
#include "vulkan_device.h"
#include "vulkan_helpers.h"
#include "vulkan_instance.h"
#include "vulkan_utils.h"

namespace tundra::vulkan_rhi {

VulkanTexture::VulkanTexture(
    core::SharedPtr<VulkanRawDevice> raw_device,
    core::SharedPtr<VulkanAllocator> allocator,
    const rhi::TextureCreateInfo& create_info) noexcept
    : m_raw_device(core::move(raw_device))
    , m_allocator(core::move(allocator))
    , m_texture_kind(create_info.kind)
    , m_texture_format(create_info.format)
    , m_texture_usage(helpers::patch_texture_usage_flags(create_info.usage))
    , m_texture_tiling(create_info.tiling)
{
    TNDR_PROFILER_TRACE("VulkanTexture::VulkanTexture");

    const VkImageCreateInfo image_create_info = helpers::map_texture_create_info(
        create_info);
    m_allocation = vulkan_map_result(
        m_allocator->create_image(
            image_create_info,
            AllocationCreateInfo {
                .memory_type = create_info.memory_type,
            }),
        "`create_image` failed");

    if (!create_info.name.empty()) {
        helpers::set_object_name(
            m_raw_device,
            reinterpret_cast<u64>(m_allocation.object),
            VK_OBJECT_TYPE_IMAGE,
            create_info.name.c_str());
    }

    const VkImageViewCreateInfo image_view_create_info {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = m_allocation.object,
        .viewType = core::visit(
            core::make_overload(
                [](const rhi::TextureKind::Texture1D&) { return VK_IMAGE_VIEW_TYPE_1D; },
                [](const rhi::TextureKind::Texture2D&) { return VK_IMAGE_VIEW_TYPE_2D; },
                [](const rhi::TextureKind::Texture3D&) { return VK_IMAGE_VIEW_TYPE_3D; },
                [](const rhi::TextureKind::TextureCube&) {
                    return VK_IMAGE_VIEW_TYPE_CUBE;
                }),
            create_info.kind),
        .format = helpers::map_texture_format(create_info.format),
        .components =
            VkComponentMapping {
                .r = VK_COMPONENT_SWIZZLE_R,
                .g = VK_COMPONENT_SWIZZLE_G,
                .b = VK_COMPONENT_SWIZZLE_B,
                .a = VK_COMPONENT_SWIZZLE_A,
            },
        .subresourceRange =
            VkImageSubresourceRange {
                // https://vulkan.lunarg.com/doc/view/1.3.224.1/windows/1.3-extensions/vkspec.html#VUID-VkDescriptorImageInfo-imageView-01976
                .aspectMask = helpers::map_texture_aspect(
                    rhi::get_texture_format_desc(create_info.format).aspect &
                    ~rhi::TextureAspectFlags::STENCIL),
                .baseMipLevel = 0,
                .levelCount = rhi::TextureKind::get_num_mips(create_info.kind),
                .baseArrayLayer = 0,
                .layerCount = rhi::TextureKind::get_num_layers(create_info.kind),
            },
    };

    m_image_view = vulkan_map_result(
        m_raw_device->get_device().create_image_view(image_view_create_info, nullptr),
        "`create_image_view` failed");

    if (!create_info.name.empty()) {
        const auto image_view_name = fmt::format(
            "{} | {}", create_info.name, "Default image view");

        helpers::set_object_name(
            m_raw_device,
            reinterpret_cast<u64>(m_image_view),
            VK_OBJECT_TYPE_IMAGE_VIEW,
            image_view_name.c_str());
    }
}

VulkanTexture::~VulkanTexture() noexcept
{
    TNDR_PROFILER_TRACE("VulkanTexture::~VulkanTexture");

    if (m_allocation.is_valid()) {
        m_raw_device->get_device().destroy_image_view(m_image_view, nullptr);
        m_allocator->destroy_image(m_allocation);
    }
}

VulkanTexture::VulkanTexture(VulkanTexture&& rhs) noexcept
    : m_raw_device(core::move(rhs.m_raw_device))
    , m_allocator(core::move(rhs.m_allocator))
    , m_texture_kind(rhs.m_texture_kind)
    , m_texture_format(rhs.m_texture_format)
    , m_texture_usage(rhs.m_texture_usage)
    , m_texture_tiling(rhs.m_texture_tiling)
    , m_allocation(core::exchange(rhs.m_allocation, VulkanAllocation<VkImage> {}))
    , m_image_view(core::exchange(rhs.m_image_view, VK_NULL_HANDLE))
{
}

VulkanTexture& VulkanTexture::operator=(VulkanTexture&& rhs) noexcept
{
    if (this != &rhs) {
        m_raw_device = core::move(rhs.m_raw_device);
        m_allocator = core::move(rhs.m_allocator);
        m_texture_kind = rhs.m_texture_kind;
        m_texture_format = rhs.m_texture_format;
        m_texture_usage = rhs.m_texture_usage;
        m_texture_tiling = rhs.m_texture_tiling;
        m_allocation = core::exchange(rhs.m_allocation, VulkanAllocation<VkImage> {});
        m_image_view = core::exchange(rhs.m_image_view, VK_NULL_HANDLE);
    }
    return *this;
}

const rhi::TextureKind::Kind& VulkanTexture::get_texture_kind() const noexcept
{
    return m_texture_kind;
}

VkImage VulkanTexture::get_image() const noexcept
{
    return m_allocation.object;
}

VkImageView VulkanTexture::get_image_view() const noexcept
{
    return m_image_view;
}

rhi::Extent VulkanTexture::get_extent() const noexcept
{
    return rhi::TextureKind::get_extent(m_texture_kind);
}

rhi::TextureFormat VulkanTexture::get_format() const noexcept
{
    return m_texture_format;
}

rhi::TextureUsageFlags VulkanTexture::get_usage() const noexcept
{
    return m_texture_usage;
}

rhi::TextureTiling VulkanTexture::get_tiling() const noexcept
{
    return m_texture_tiling;
}

u32 VulkanTexture::get_num_mips() const noexcept
{
    return rhi::TextureKind::get_num_mips(m_texture_kind);
}

rhi::SampleCount VulkanTexture::get_sample_count() const noexcept
{
    return rhi::TextureKind::get_sample_count(m_texture_kind);
}

u32 VulkanTexture::get_num_layers() const noexcept
{
    return rhi::TextureKind::get_num_layers(m_texture_kind);
}

} // namespace tundra::vulkan_rhi
