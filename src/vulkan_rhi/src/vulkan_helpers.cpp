#include "vulkan_helpers.h"
#include "core/std/option.h"
#include "core/std/panic.h"
#include "vulkan_device.h"
#include "vulkan_instance.h"

namespace tundra::vulkan_rhi::helpers {

// warning C4715: 'tundra::vulkan_rhi::helpers::map_texture_format': not all control paths return a value
// ????????

VkFormat map_texture_format(const rhi::TextureFormat texture_format) noexcept
{
    switch (texture_format) {
        case rhi::TextureFormat::R4_G4:
            return VK_FORMAT_R4G4_UNORM_PACK8;
        case rhi::TextureFormat::R4_G4_B4_A4:
            return VK_FORMAT_R4G4B4A4_UNORM_PACK16;
        case rhi::TextureFormat::B4_G4_R4_A4:
            return VK_FORMAT_B4G4R4A4_UNORM_PACK16;
        case rhi::TextureFormat::R5_G6_B5:
            return VK_FORMAT_R5G6B5_UNORM_PACK16;
        case rhi::TextureFormat::B5_G6_R5:
            return VK_FORMAT_B5G6R5_UNORM_PACK16;
        case rhi::TextureFormat::R5_G5_B5_A1:
            return VK_FORMAT_R5G5B5A1_UNORM_PACK16;
        case rhi::TextureFormat::B5_G5_R5_A1:
            return VK_FORMAT_B5G5R5A1_UNORM_PACK16;
        case rhi::TextureFormat::A1_R5_G5_B5:
            return VK_FORMAT_A1R5G5B5_UNORM_PACK16;
        case rhi::TextureFormat::R8_UINT:
            return VK_FORMAT_R8_UINT;
        case rhi::TextureFormat::R8_SINT:
            return VK_FORMAT_R8_SINT;
        case rhi::TextureFormat::R8_UNORM:
            return VK_FORMAT_R8_UNORM;
        case rhi::TextureFormat::R8_SNORM:
            return VK_FORMAT_R8_SNORM;
        case rhi::TextureFormat::R8_G8_UINT:
            return VK_FORMAT_R8G8_UINT;
        case rhi::TextureFormat::R8_G8_SINT:
            return VK_FORMAT_R8G8_SINT;
        case rhi::TextureFormat::R8_G8_UNORM:
            return VK_FORMAT_R8G8_UNORM;
        case rhi::TextureFormat::R8_G8_SNORM:
            return VK_FORMAT_R8G8_SNORM;
        case rhi::TextureFormat::R8_G8_B8_UINT:
            return VK_FORMAT_R8G8B8_UINT;
        case rhi::TextureFormat::R8_G8_B8_SINT:
            return VK_FORMAT_R8G8B8_SINT;
        case rhi::TextureFormat::R8_G8_B8_UNORM:
            return VK_FORMAT_R8G8B8_UNORM;
        case rhi::TextureFormat::R8_G8_B8_SNORM:
            return VK_FORMAT_R8G8B8_SNORM;
        case rhi::TextureFormat::B8_G8_R8_UINT:
            return VK_FORMAT_B8G8R8_UINT;
        case rhi::TextureFormat::B8_G8_R8_SINT:
            return VK_FORMAT_B8G8R8_SINT;
        case rhi::TextureFormat::B8_G8_R8_UNORM:
            return VK_FORMAT_B8G8R8_UNORM;
        case rhi::TextureFormat::B8_G8_R8_SNORM:
            return VK_FORMAT_B8G8R8_SNORM;
        case rhi::TextureFormat::R8_G8_B8_A8_UINT:
            return VK_FORMAT_R8G8B8A8_UINT;
        case rhi::TextureFormat::R8_G8_B8_A8_SINT:
            return VK_FORMAT_R8G8B8A8_SINT;
        case rhi::TextureFormat::R8_G8_B8_A8_UNORM:
            return VK_FORMAT_R8G8B8A8_UNORM;
        case rhi::TextureFormat::R8_G8_B8_A8_SNORM:
            return VK_FORMAT_R8G8B8A8_SNORM;
        case rhi::TextureFormat::B8_G8_R8_A8_UINT:
            return VK_FORMAT_B8G8R8A8_UINT;
        case rhi::TextureFormat::B8_G8_R8_A8_SINT:
            return VK_FORMAT_B8G8R8A8_SINT;
        case rhi::TextureFormat::B8_G8_R8_A8_UNORM:
            return VK_FORMAT_B8G8R8A8_UNORM;
        case rhi::TextureFormat::B8_G8_R8_A8_SNORM:
            return VK_FORMAT_B8G8R8A8_SNORM;
        case rhi::TextureFormat::A8_B8_G8_R8_UINT:
            return VK_FORMAT_A8B8G8R8_UINT_PACK32;
        case rhi::TextureFormat::A8_B8_G8_R8_SINT:
            return VK_FORMAT_A8B8G8R8_SINT_PACK32;
        case rhi::TextureFormat::A8_B8_G8_R8_UNORM:
            return VK_FORMAT_A8B8G8R8_UNORM_PACK32;
        case rhi::TextureFormat::A8_B8_G8_R8_SNORM:
            return VK_FORMAT_A8B8G8R8_SNORM_PACK32;
        case rhi::TextureFormat::A2_R10_G10_B10_UINT:
            return VK_FORMAT_A2R10G10B10_UINT_PACK32;
        case rhi::TextureFormat::A2_R10_G10_B10_SINT:
            return VK_FORMAT_A2R10G10B10_SINT_PACK32;
        case rhi::TextureFormat::A2_R10_G10_B10_UNORM:
            return VK_FORMAT_A2R10G10B10_UNORM_PACK32;
        case rhi::TextureFormat::A2_R10_G10_B10_SNORM:
            return VK_FORMAT_A2R10G10B10_SNORM_PACK32;
        case rhi::TextureFormat::A2_B10_G10_R10_UINT:
            return VK_FORMAT_A2B10G10R10_UINT_PACK32;
        case rhi::TextureFormat::A2_B10_G10_R10_SINT:
            return VK_FORMAT_A2B10G10R10_SINT_PACK32;
        case rhi::TextureFormat::A2_B10_G10_R10_UNORM:
            return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
        case rhi::TextureFormat::A2_B10_G10_R10_SNORM:
            return VK_FORMAT_A2B10G10R10_SNORM_PACK32;
        case rhi::TextureFormat::R16_UINT:
            return VK_FORMAT_R16_UINT;
        case rhi::TextureFormat::R16_SINT:
            return VK_FORMAT_R16_SINT;
        case rhi::TextureFormat::R16_UNORM:
            return VK_FORMAT_R16_UNORM;
        case rhi::TextureFormat::R16_SNORM:
            return VK_FORMAT_R16_SNORM;
        case rhi::TextureFormat::R16_FLOAT:
            return VK_FORMAT_R16_SFLOAT;
        case rhi::TextureFormat::R16_G16_UINT:
            return VK_FORMAT_R16G16_UINT;
        case rhi::TextureFormat::R16_G16_SINT:
            return VK_FORMAT_R16G16_SINT;
        case rhi::TextureFormat::R16_G16_UNORM:
            return VK_FORMAT_R16G16_UNORM;
        case rhi::TextureFormat::R16_G16_SNORM:
            return VK_FORMAT_R16G16_SNORM;
        case rhi::TextureFormat::R16_G16_FLOAT:
            return VK_FORMAT_R16G16_SFLOAT;
        case rhi::TextureFormat::R16_G16_B16_UINT:
            return VK_FORMAT_R16G16B16_UINT;
        case rhi::TextureFormat::R16_G16_B16_SINT:
            return VK_FORMAT_R16G16B16_SINT;
        case rhi::TextureFormat::R16_G16_B16_UNORM:
            return VK_FORMAT_R16G16B16_UNORM;
        case rhi::TextureFormat::R16_G16_B16_SNORM:
            return VK_FORMAT_R16G16B16_SNORM;
        case rhi::TextureFormat::R16_G16_B16_FLOAT:
            return VK_FORMAT_R16G16B16_SFLOAT;
        case rhi::TextureFormat::R16_G16_B16_A16_UINT:
            return VK_FORMAT_R16G16B16A16_UINT;
        case rhi::TextureFormat::R16_G16_B16_A16_SINT:
            return VK_FORMAT_R16G16B16A16_SINT;
        case rhi::TextureFormat::R16_G16_B16_A16_UNORM:
            return VK_FORMAT_R16G16B16A16_UNORM;
        case rhi::TextureFormat::R16_G16_B16_A16_SNORM:
            return VK_FORMAT_R16G16B16A16_SNORM;
        case rhi::TextureFormat::R16_G16_B16_A16_FLOAT:
            return VK_FORMAT_R16G16B16A16_SFLOAT;
        case rhi::TextureFormat::R32_UINT:
            return VK_FORMAT_R32_UINT;
        case rhi::TextureFormat::R32_SINT:
            return VK_FORMAT_R32_SINT;
        case rhi::TextureFormat::R32_FLOAT:
            return VK_FORMAT_R32_SFLOAT;
        case rhi::TextureFormat::R32_G32_UINT:
            return VK_FORMAT_R32G32_UINT;
        case rhi::TextureFormat::R32_G32_SINT:
            return VK_FORMAT_R32G32_SINT;
        case rhi::TextureFormat::R32_G32_FLOAT:
            return VK_FORMAT_R32G32_SFLOAT;
        case rhi::TextureFormat::R32_G32_B32_UINT:
            return VK_FORMAT_R32G32B32_UINT;
        case rhi::TextureFormat::R32_G32_B32_SINT:
            return VK_FORMAT_R32G32B32_SINT;
        case rhi::TextureFormat::R32_G32_B32_FLOAT:
            return VK_FORMAT_R32G32B32_SFLOAT;
        case rhi::TextureFormat::R32_G32_B32_A32_UINT:
            return VK_FORMAT_R32G32B32A32_UINT;
        case rhi::TextureFormat::R32_G32_B32_A32_SINT:
            return VK_FORMAT_R32G32B32A32_SINT;
        case rhi::TextureFormat::R32_G32_B32_A32_FLOAT:
            return VK_FORMAT_R32G32B32A32_SFLOAT;
        case rhi::TextureFormat::R64_UINT:
            return VK_FORMAT_R64_UINT;
        case rhi::TextureFormat::R64_SINT:
            return VK_FORMAT_R64_SINT;
        case rhi::TextureFormat::R64_FLOAT:
            return VK_FORMAT_R64_SFLOAT;
        case rhi::TextureFormat::R64_G64_UINT:
            return VK_FORMAT_R64G64_UINT;
        case rhi::TextureFormat::R64_G64_SINT:
            return VK_FORMAT_R64G64_SINT;
        case rhi::TextureFormat::R64_G64_FLOAT:
            return VK_FORMAT_R64G64_SFLOAT;
        case rhi::TextureFormat::R64_G64_B64_UINT:
            return VK_FORMAT_R64G64B64_UINT;
        case rhi::TextureFormat::R64_G64_B64_SINT:
            return VK_FORMAT_R64G64B64_SINT;
        case rhi::TextureFormat::R64_G64_B64_FLOAT:
            return VK_FORMAT_R64G64B64_SFLOAT;
        case rhi::TextureFormat::R64_G64_B64_A64_UINT:
            return VK_FORMAT_R64G64B64A64_UINT;
        case rhi::TextureFormat::R64_G64_B64_A64_SINT:
            return VK_FORMAT_R64G64B64A64_SINT;
        case rhi::TextureFormat::R64_G64_B64_A64_FLOAT:
            return VK_FORMAT_R64G64B64A64_SFLOAT;
        case rhi::TextureFormat::B10_G11_R11_FLOAT:
            return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
        case rhi::TextureFormat::D16_UNORM:
            return VK_FORMAT_D16_UNORM;
        case rhi::TextureFormat::D32_FLOAT:
            return VK_FORMAT_D32_SFLOAT;
        case rhi::TextureFormat::S8_UINT:
            return VK_FORMAT_S8_UINT;
        case rhi::TextureFormat::D16_UNORM_S8_UINT:
            return VK_FORMAT_D16_UNORM_S8_UINT;
        case rhi::TextureFormat::D24_UNORM_S8_UINT:
            return VK_FORMAT_D24_UNORM_S8_UINT;
        case rhi::TextureFormat::D32_FLOAT_S8_UINT:
            return VK_FORMAT_D32_SFLOAT_S8_UINT;
        case rhi::TextureFormat::BC1_RGBA_UNORM:
            return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
        case rhi::TextureFormat::BC1_RGBA_SRGB:
            return VK_FORMAT_BC1_RGBA_SRGB_BLOCK;
        case rhi::TextureFormat::BC2_UNORM:
            return VK_FORMAT_BC2_UNORM_BLOCK;
        case rhi::TextureFormat::BC2_SRGB:
            return VK_FORMAT_BC2_SRGB_BLOCK;
        case rhi::TextureFormat::BC3_UNORM:
            return VK_FORMAT_BC3_UNORM_BLOCK;
        case rhi::TextureFormat::BC3_SRGB:
            return VK_FORMAT_BC3_SRGB_BLOCK;
        case rhi::TextureFormat::BC4_UNORM:
            return VK_FORMAT_BC4_UNORM_BLOCK;
        case rhi::TextureFormat::BC4_SNORM:
            return VK_FORMAT_BC4_SNORM_BLOCK;
        case rhi::TextureFormat::BC5_UNORM:
            return VK_FORMAT_BC5_UNORM_BLOCK;
        case rhi::TextureFormat::BC5_SNORM:
            return VK_FORMAT_BC5_SNORM_BLOCK;
        case rhi::TextureFormat::BC7_UNORM:
            return VK_FORMAT_BC7_UNORM_BLOCK;
        case rhi::TextureFormat::BC7_SRGB:
            return VK_FORMAT_BC7_SRGB_BLOCK;
    }

    core::panic("Invalid enum");
}

VkExtent3D map_extent(const rhi::Extent& extent) noexcept
{
    return VkExtent3D {
        .width = extent.width,
        .height = extent.height,
        .depth = extent.depth,
    };
}

VkSampleCountFlagBits map_sample_count(const rhi::SampleCount sample_count) noexcept
{
    switch (sample_count) {
        case rhi::SampleCount::Count1:
            return VK_SAMPLE_COUNT_1_BIT;
        case rhi::SampleCount::Count2:
            return VK_SAMPLE_COUNT_2_BIT;
        case rhi::SampleCount::Count4:
            return VK_SAMPLE_COUNT_4_BIT;
        case rhi::SampleCount::Count8:
            return VK_SAMPLE_COUNT_8_BIT;
    }

    core::panic("Invalid enum");
}

VkImageTiling map_texture_tiling(const rhi::TextureTiling texture_tiling) noexcept
{
    switch (texture_tiling) {
        case rhi::TextureTiling::Optimal:
            return VK_IMAGE_TILING_OPTIMAL;
        case rhi::TextureTiling::Linear:
            return VK_IMAGE_TILING_LINEAR;
    }

    core::panic("Invalid enum");
}

rhi::TextureUsageFlags patch_texture_usage_flags(
    rhi::TextureUsageFlags texture_usage) noexcept
{
    if (contains(texture_usage, rhi::TextureUsageFlags::PRESENT)) {
        texture_usage |= rhi::TextureUsageFlags::TRANSFER_SOURCE;
    }

    // If think this stuff is only related to Vulkan, so we are gonna silently pass "VK_IMAGE_USAGE_SAMPLED_BIT" if needed.
    // https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkCmdBeginRenderPass.html#VUID-vkCmdBeginRenderPass-initialLayout-00897
    // #TODO: Check how this stuff works in DX12.
    const bool has_color_attachment = contains(
        texture_usage, rhi::TextureUsageFlags::COLOR_ATTACHMENT);
    const bool has_stencil_attachment = contains(
        texture_usage, rhi::TextureUsageFlags::STENCIL_ATTACHMENT);
    const bool has_depth_attachment = contains(
        texture_usage, rhi::TextureUsageFlags::DEPTH_ATTACHMENT);
    if (has_color_attachment || has_stencil_attachment || has_depth_attachment) {
        texture_usage |= rhi::TextureUsageFlags::SRV;
    }

    return texture_usage;
}

VkImageUsageFlags map_texture_usage(const rhi::TextureUsageFlags texture_usage) noexcept
{
    VkImageUsageFlags flags {};

    if (contains(texture_usage, rhi::TextureUsageFlags::COLOR_ATTACHMENT)) {
        flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    }
    if (contains(texture_usage, rhi::TextureUsageFlags::DEPTH_ATTACHMENT)) {
        flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    }
    if (contains(texture_usage, rhi::TextureUsageFlags::STENCIL_ATTACHMENT)) {
        flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    }
    if (contains(texture_usage, rhi::TextureUsageFlags::SRV)) {
        flags |= VK_IMAGE_USAGE_SAMPLED_BIT;
    }
    if (contains(texture_usage, rhi::TextureUsageFlags::UAV)) {
        flags |= VK_IMAGE_USAGE_STORAGE_BIT;
    }
    if (contains(texture_usage, rhi::TextureUsageFlags::TRANSFER_SOURCE)) {
        flags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    }
    if (contains(texture_usage, rhi::TextureUsageFlags::TRANSFER_DESTINATION)) {
        flags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }
    if (contains(texture_usage, rhi::TextureUsageFlags::PRESENT)) {
        flags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    }

    return flags;
}

bool is_layout_allowed(
    const VkImageLayout image_layout, const rhi::TextureUsageFlags usage) noexcept
{
    switch (image_layout) {
        // UNDEFINED and GENERAL are always supported.
        case VK_IMAGE_LAYOUT_UNDEFINED:
        case VK_IMAGE_LAYOUT_GENERAL:
            return true;
        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            return contains(usage, rhi::TextureUsageFlags::SRV);
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            return contains(usage, rhi::TextureUsageFlags::COLOR_ATTACHMENT);
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            return intersects(
                usage,
                rhi::TextureUsageFlags::DEPTH_ATTACHMENT |
                    rhi::TextureUsageFlags::STENCIL_ATTACHMENT);
        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            return contains(usage, rhi::TextureUsageFlags::TRANSFER_SOURCE);
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            return contains(usage, rhi::TextureUsageFlags::TRANSFER_DESTINATION);
        default:
            core::panic("Invalid texture layout!");
    }
}

VkAttachmentLoadOp map_attachment_load_op(const rhi::AttachmentLoadOp op) noexcept
{
    switch (op) {
        case rhi::AttachmentLoadOp::Load:
            return VK_ATTACHMENT_LOAD_OP_LOAD;
        case rhi::AttachmentLoadOp::Clear:
            return VK_ATTACHMENT_LOAD_OP_CLEAR;
        case rhi::AttachmentLoadOp::DontCare:
            return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    }

    core::panic("Invalid enum");
}

VkAttachmentStoreOp map_attachment_store_op(const rhi::AttachmentStoreOp op) noexcept
{
    switch (op) {
        case rhi::AttachmentStoreOp::Store:
            return VK_ATTACHMENT_STORE_OP_STORE;
        case rhi::AttachmentStoreOp::DontCare:
            return VK_ATTACHMENT_STORE_OP_DONT_CARE;
    }

    core::panic("Invalid enum");
}

VkImageSubresourceRange image_subresource_range() noexcept
{
    return VkImageSubresourceRange {
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .baseMipLevel = 0,
        .levelCount = 1,
        .baseArrayLayer = 0,
        .layerCount = 1,
    };
}

VkImageAspectFlags map_texture_aspect(const rhi::TextureAspectFlags aspect_flags) noexcept
{
    VkImageAspectFlags flags {};

    if (contains(aspect_flags, rhi::TextureAspectFlags::COLOR)) {
        flags |= VK_IMAGE_ASPECT_COLOR_BIT;
    }

    if (contains(aspect_flags, rhi::TextureAspectFlags::DEPTH)) {
        flags |= VK_IMAGE_ASPECT_DEPTH_BIT;
    }

    if (contains(aspect_flags, rhi::TextureAspectFlags::STENCIL)) {
        flags |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }

    return flags;
}

VkImageCreateInfo map_texture_create_info(
    const rhi::TextureCreateInfo& create_info) noexcept
{
    const VkImageCreateInfo image_create_info {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .flags = core::visit(
            core::make_overload(
                [](const rhi::TextureKind::TextureCube&) -> VkImageCreateFlags {
                    return VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
                },
                [](const auto&) -> VkImageCreateFlags { return 0; }),
            create_info.kind),
        .imageType = core::visit(
            core::make_overload(
                [](const rhi::TextureKind::Texture1D&) { return VK_IMAGE_TYPE_1D; },
                [](const rhi::TextureKind::Texture2D&) { return VK_IMAGE_TYPE_2D; },
                [](const rhi::TextureKind::Texture3D&) { return VK_IMAGE_TYPE_3D; },
                [](const rhi::TextureKind::TextureCube&) { return VK_IMAGE_TYPE_2D; }),
            create_info.kind),
        .format = map_texture_format(create_info.format),
        .extent = map_extent(rhi::TextureKind::get_extent(create_info.kind)),
        .mipLevels = rhi::TextureKind::get_num_mips(create_info.kind),
        .arrayLayers = rhi::TextureKind::get_num_layers(create_info.kind),
        .samples = map_sample_count(rhi::TextureKind::get_sample_count(create_info.kind)),
        .tiling = map_texture_tiling(create_info.tiling),
        .usage = map_texture_usage(patch_texture_usage_flags(create_info.usage)),
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };

    return image_create_info;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Buffer

VkBufferUsageFlags map_buffer_usage(const rhi::BufferUsageFlags buffer_usage) noexcept
{
    VkBufferUsageFlags flags {};

    if (contains(buffer_usage, rhi::BufferUsageFlags::TRANSFER_SOURCE)) {
        flags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    }
    if (contains(buffer_usage, rhi::BufferUsageFlags::TRANSFER_DESTINATION)) {
        flags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    }
    if (contains(buffer_usage, rhi::BufferUsageFlags::SRV) ||
        contains(buffer_usage, rhi::BufferUsageFlags::UAV)) {
        // #NOTE: From vulkan perspective SRV and UAV are the same. The only difference is in the syntax in glsl.
        // https://www.khronos.org/opengl/wiki/Shader_Storage_Buffer_Object#Memory_qualifiers
        // SRV allows only reads - `readonly buffer BufferName`.
        // UAV allows reads and writes - `buffer BufferName`.
        flags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    }
    if (contains(buffer_usage, rhi::BufferUsageFlags::CBV)) {
        flags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    }
    if (contains(buffer_usage, rhi::BufferUsageFlags::INDEX_BUFFER)) {
        flags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    }
    if (contains(buffer_usage, rhi::BufferUsageFlags::VERTEX_BUFFER)) {
        flags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    }
    if (contains(buffer_usage, rhi::BufferUsageFlags::INDIRECT_BUFFER)) {
        flags |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
    }

    return flags;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Shader

VkShaderStageFlagBits map_shader_stage(const rhi::ShaderStage shader_stage) noexcept
{
    switch (shader_stage) {
        case rhi::ShaderStage::VertexShader:
            return VK_SHADER_STAGE_VERTEX_BIT;
        case rhi::ShaderStage::FragmentShader:
            return VK_SHADER_STAGE_FRAGMENT_BIT;
        case rhi::ShaderStage::ComputeShader:
            return VK_SHADER_STAGE_COMPUTE_BIT;
    }

    core::panic("Invalid enum");
}
/////////////////////////////////////////////////////////////////////////////////////////
// Graphics pipeline

VkPrimitiveTopology map_primitive_type(const rhi::PrimitiveType primitive_type) noexcept
{
    switch (primitive_type) {
        case rhi::PrimitiveType::Point:
            return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        case rhi::PrimitiveType::Lines:
            return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
        case rhi::PrimitiveType::Triangle:
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    }

    core::panic("Invalid enum");
}

VkPolygonMode map_polygon_mode(const rhi::PolygonMode polygon_mode) noexcept
{
    switch (polygon_mode) {
        case rhi::PolygonMode::Fill:
            return VK_POLYGON_MODE_FILL;
        case rhi::PolygonMode::Line:
            return VK_POLYGON_MODE_LINE;
        case rhi::PolygonMode::Point:
            return VK_POLYGON_MODE_POINT;
    }

    core::panic("Invalid enum");
}

VkCullModeFlags map_culling_mode(const rhi::CullingMode culling_mode) noexcept
{
    switch (culling_mode) {
        case rhi::CullingMode::None:
            return VK_CULL_MODE_NONE;
        case rhi::CullingMode::Front:
            return VK_CULL_MODE_FRONT_BIT;
        case rhi::CullingMode::Back:
            return VK_CULL_MODE_BACK_BIT;
        case rhi::CullingMode::FrontBack:
            return VK_CULL_MODE_FRONT_AND_BACK;
    }

    core::panic("Invalid enum");
}

VkFrontFace map_front_face(const rhi::FrontFace front_face) noexcept
{
    switch (front_face) {
        case rhi::FrontFace::Clockwise:
            return VK_FRONT_FACE_CLOCKWISE;
        case rhi::FrontFace::CounterClockwise:
            return VK_FRONT_FACE_COUNTER_CLOCKWISE;
    }

    core::panic("Invalid enum");
}

VkCompareOp map_compare_op(const rhi::CompareOp op) noexcept
{
    switch (op) {
        case rhi::CompareOp::Never:
            return VK_COMPARE_OP_NEVER;
        case rhi::CompareOp::Less:
            return VK_COMPARE_OP_LESS;
        case rhi::CompareOp::Equal:
            return VK_COMPARE_OP_EQUAL;
        case rhi::CompareOp::LessOrEqual:
            return VK_COMPARE_OP_LESS_OR_EQUAL;
        case rhi::CompareOp::Greater:
            return VK_COMPARE_OP_GREATER;
        case rhi::CompareOp::NotEqual:
            return VK_COMPARE_OP_NOT_EQUAL;
        case rhi::CompareOp::GreaterOrEqual:
            return VK_COMPARE_OP_GREATER_OR_EQUAL;
        case rhi::CompareOp::Always:
            return VK_COMPARE_OP_ALWAYS;
    }

    core::panic("Invalid enum");
}

VkStencilOp map_stencil_op(const rhi::StencilOp op) noexcept
{
    switch (op) {
        case rhi::StencilOp::Keep:
            return VK_STENCIL_OP_KEEP;
        case rhi::StencilOp::Zero:
            return VK_STENCIL_OP_ZERO;
        case rhi::StencilOp::Replace:
            return VK_STENCIL_OP_REPLACE;
        case rhi::StencilOp::IncrementAndClamp:
            return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
        case rhi::StencilOp::DecrementAndClamp:
            return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
        case rhi::StencilOp::Invert:
            return VK_STENCIL_OP_INVERT;
        case rhi::StencilOp::IncrementAndWrap:
            return VK_STENCIL_OP_INCREMENT_AND_WRAP;
        case rhi::StencilOp::DecrementAndWrap:
            return VK_STENCIL_OP_DECREMENT_AND_WRAP;
    }

    core::panic("Invalid enum");
}

VkStencilOpState map_stencil_op_desc(const rhi::StencilOpDesc& desc) noexcept
{
    return VkStencilOpState {
        .failOp = map_stencil_op(desc.fail_op),
        .passOp = map_stencil_op(desc.pass_op),
        .depthFailOp = map_stencil_op(desc.depth_fail_op),
        .compareOp = map_compare_op(desc.compare_op),
        .compareMask = desc.compare_mask,
        .writeMask = desc.write_mask,
        .reference = desc.reference,
    };
}

VkColorComponentFlags map_color_write_mask(const rhi::ColorWriteMask mask) noexcept
{
    VkColorComponentFlags flags {};

    if (contains(mask, rhi::ColorWriteMask::RED)) {
        flags |= VK_COLOR_COMPONENT_R_BIT;
    }
    if (contains(mask, rhi::ColorWriteMask::GREEN)) {
        flags |= VK_COLOR_COMPONENT_G_BIT;
    }
    if (contains(mask, rhi::ColorWriteMask::BLUE)) {
        flags |= VK_COLOR_COMPONENT_B_BIT;
    }
    if (contains(mask, rhi::ColorWriteMask::ALPHA)) {
        flags |= VK_COLOR_COMPONENT_A_BIT;
    }

    return flags;
}

VkBlendFactor map_blend_factor(const rhi::BlendFactor blend_factor) noexcept
{
    switch (blend_factor) {
        case rhi::BlendFactor::Zero:
            return VK_BLEND_FACTOR_ZERO;
        case rhi::BlendFactor::One:
            return VK_BLEND_FACTOR_ONE;
        case rhi::BlendFactor::SourceColor:
            return VK_BLEND_FACTOR_SRC_COLOR;
        case rhi::BlendFactor::OneMinusSourceColor:
            return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
        case rhi::BlendFactor::DestinationColor:
            return VK_BLEND_FACTOR_DST_COLOR;
        case rhi::BlendFactor::OneMinusDestinationColor:
            return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
        case rhi::BlendFactor::SourceAlpha:
            return VK_BLEND_FACTOR_SRC_ALPHA;
        case rhi::BlendFactor::OneMinusSourceAlpha:
            return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        case rhi::BlendFactor::DestinationAlpha:
            return VK_BLEND_FACTOR_DST_ALPHA;
        case rhi::BlendFactor::OneMinusDestinationAlpha:
            return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
        case rhi::BlendFactor::ConstantColor:
            return VK_BLEND_FACTOR_CONSTANT_COLOR;
        case rhi::BlendFactor::OneMinusConstantColor:
            return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
        case rhi::BlendFactor::ConstantAlpha:
            return VK_BLEND_FACTOR_CONSTANT_ALPHA;
        case rhi::BlendFactor::OneMinusConstantAlpha:
            return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA;
        case rhi::BlendFactor::SourceAlphaSaturate:
            return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
        case rhi::BlendFactor::Source1Color:
            return VK_BLEND_FACTOR_SRC1_COLOR;
        case rhi::BlendFactor::OneMinusSource1Color:
            return VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR;
        case rhi::BlendFactor::Source1Alpha:
            return VK_BLEND_FACTOR_SRC1_ALPHA;
        case rhi::BlendFactor::OneMinusSource1Alpha:
            return VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA;
    }

    core::panic("Invalid enum");
}

core::Tuple<VkBlendOp, VkBlendFactor, VkBlendFactor> map_blend_op(
    const rhi::BlendOp::Op& op) noexcept
{
    return core::visit(
        core::make_overload(
            [](const rhi::BlendOp::Add& add) {
                const auto& [src, dst] = add;
                return core::make_tuple(
                    VK_BLEND_OP_ADD, map_blend_factor(src), map_blend_factor(dst));
            },
            [](const rhi::BlendOp::Subtract& subtract) {
                const auto& [src, dst] = subtract;
                return core::make_tuple(
                    VK_BLEND_OP_SUBTRACT, map_blend_factor(src), map_blend_factor(dst));
            },
            [](const rhi::BlendOp::ReverseSubtract& reverse_subtract) {
                const auto& [src, dst] = reverse_subtract;
                return core::make_tuple(
                    VK_BLEND_OP_REVERSE_SUBTRACT,
                    map_blend_factor(src),
                    map_blend_factor(dst));
            },
            [](const rhi::BlendOp::Min&) {
                return core::make_tuple(
                    VK_BLEND_OP_MIN, VK_BLEND_FACTOR_ZERO, VK_BLEND_FACTOR_ZERO);
            },
            [](const rhi::BlendOp::Max&) {
                return core::make_tuple(
                    VK_BLEND_OP_MAX, VK_BLEND_FACTOR_ZERO, VK_BLEND_FACTOR_ZERO);
            }),
        op);
}

VkClearValue map_clear_value(const rhi::ClearValue& clear_value) noexcept
{
    return core::visit(
        core::make_overload(
            [](const math::Vec4& v) {
                VkClearValue clear_value;
                std::memcpy(clear_value.color.float32, &v, sizeof(v));
                return clear_value;
            },
            [](const math::IVec4& v) {
                VkClearValue clear_value;
                std::memcpy(clear_value.color.int32, &v, sizeof(v));
                return clear_value;
            },
            [](const math::UVec4& v) {
                VkClearValue clear_value;
                std::memcpy(clear_value.color.uint32, &v, sizeof(v));
                return clear_value;
            }),
        clear_value);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Sampler

VkFilter map_sampler_filer(const rhi::SamplerFilter filter) noexcept
{
    switch (filter) {
        case rhi::SamplerFilter::Nearest:
            return VK_FILTER_NEAREST;
        case rhi::SamplerFilter::Linear:
            return VK_FILTER_LINEAR;
    }

    core::panic("Invalid enum");
}

VkSamplerAddressMode map_sampler_address_mode(
    const rhi::SamplerAddressMode address_mode) noexcept
{
    switch (address_mode) {
        case rhi::SamplerAddressMode::Repeat:
            return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        case rhi::SamplerAddressMode::MirroredRepeat:
            return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        case rhi::SamplerAddressMode::ClampToEdge:
            return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        case rhi::SamplerAddressMode::ClampToBorder:
            return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        case rhi::SamplerAddressMode::MirrorClampToEdge:
            return VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
    }

    core::panic("Invalid enum");
}

/////////////////////////////////////////////////////////////////////////////////////////
// Debug

void set_object_name(
    const core::SharedPtr<VulkanRawDevice>& device,
    const u64 handle,
    const VkObjectType object_type,
    const char* name) noexcept
{
    if (name != nullptr) {
        if (auto debug_utils = device->get_instance()->get_debug_utils(); debug_utils) {
            const VkResult result =
                (*debug_utils)
                    ->set_debug_utils_object_name(
                        device->get_device().get_handle(),
                        VkDebugUtilsObjectNameInfoEXT {
                            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                            .objectType = object_type,
                            .objectHandle = handle,
                            .pObjectName = name,
                        });
            tndr_assert(result == VK_SUCCESS, "`set_debug_utils_object_name` failed!");
        }
    }
}

void begin_region(
    const core::SharedPtr<VulkanRawDevice>& device,
    const VkCommandBuffer command_buffer,
    const core::String& name,
    const math::Vec4& color) noexcept
{
    if (auto debug_utils = device->get_instance()->get_debug_utils()) {
        VkDebugUtilsLabelEXT debug_utils_label {
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT,
            .pLabelName = name.c_str(),
        };

        std::memcpy(debug_utils_label.color, &color, sizeof(color));
        (*debug_utils)->cmd_begin_debug_utils_label(command_buffer, debug_utils_label);
    }
}

void end_region(
    const core::SharedPtr<VulkanRawDevice>& device,
    const VkCommandBuffer command_buffer) noexcept
{
    if (auto debug_utils = device->get_instance()->get_debug_utils()) {
        (*debug_utils)->cmd_end_debug_utils_label(command_buffer);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////
// Barriers

AccessInfo get_access_info(const rhi::AccessFlags flag) noexcept
{
    switch (flag) {
        case rhi::AccessFlags::NONE:
            return AccessInfo {
                .access_flags = 0,
                .stage_flags = 0,
                .image_layout = VK_IMAGE_LAYOUT_UNDEFINED,
            };
        case rhi::AccessFlags::INDIRECT_BUFFER:
            return AccessInfo {
                .access_flags = VK_ACCESS_INDIRECT_COMMAND_READ_BIT,
                .stage_flags = VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT,
                .image_layout = VK_IMAGE_LAYOUT_UNDEFINED,
            };
        case rhi::AccessFlags::INDEX_BUFFER:
            return AccessInfo {
                .access_flags = VK_ACCESS_INDEX_READ_BIT,
                .stage_flags = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
                .image_layout = VK_IMAGE_LAYOUT_UNDEFINED,
            };
        case rhi::AccessFlags::VERTEX_BUFFER:
            return AccessInfo {
                .access_flags = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT,
                .stage_flags = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
                .image_layout = VK_IMAGE_LAYOUT_UNDEFINED,
            };
        case rhi::AccessFlags::SRV_GRAPHICS:
            return AccessInfo {
                .access_flags = VK_ACCESS_UNIFORM_READ_BIT | VK_ACCESS_SHADER_READ_BIT,
                .stage_flags = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT |
                               VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
                .image_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            };
        case rhi::AccessFlags::SRV_COMPUTE:
            return AccessInfo {
                .access_flags = VK_ACCESS_UNIFORM_READ_BIT | VK_ACCESS_SHADER_READ_BIT,
                .stage_flags = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                .image_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            };
        case rhi::AccessFlags::TRANSFER_READ:
            return AccessInfo {
                .access_flags = VK_ACCESS_TRANSFER_READ_BIT,
                .stage_flags = VK_PIPELINE_STAGE_TRANSFER_BIT,
                .image_layout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            };
        case rhi::AccessFlags::HOST_READ:
            return AccessInfo {
                .access_flags = VK_ACCESS_HOST_READ_BIT,
                .stage_flags = VK_PIPELINE_STAGE_HOST_BIT,
                .image_layout = VK_IMAGE_LAYOUT_GENERAL,
            };
        case rhi::AccessFlags::COLOR_ATTACHMENT_READ:
            return AccessInfo {
                .access_flags = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT,
                .stage_flags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                .image_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            };
        case rhi::AccessFlags::DEPTH_STENCIL_ATTACHMENT_READ:
            return AccessInfo {
                .access_flags = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
                .stage_flags = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                               VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
                .image_layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
            };
        case rhi::AccessFlags::PRESENT:
            return AccessInfo {
                .access_flags = 0,
                .stage_flags = 0,
                .image_layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            };
        case rhi::AccessFlags::UAV_GRAPHICS:
            return AccessInfo {
                .access_flags = VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT,
                .stage_flags = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT |
                               VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                .image_layout = VK_IMAGE_LAYOUT_GENERAL,
            };
        case rhi::AccessFlags::UAV_COMPUTE:
            return AccessInfo {
                .access_flags = VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT,
                .stage_flags = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                .image_layout = VK_IMAGE_LAYOUT_GENERAL,
            };
        case rhi::AccessFlags::TRANSFER_WRITE:
            return AccessInfo {
                .access_flags = VK_ACCESS_TRANSFER_WRITE_BIT,
                .stage_flags = VK_PIPELINE_STAGE_TRANSFER_BIT,
                .image_layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            };
        case rhi::AccessFlags::HOST_WRITE:
            return AccessInfo {
                .access_flags = VK_ACCESS_HOST_WRITE_BIT,
                .stage_flags = VK_PIPELINE_STAGE_HOST_BIT,
                .image_layout = VK_IMAGE_LAYOUT_GENERAL,
            };
        case rhi::AccessFlags::COLOR_ATTACHMENT_WRITE:
            return AccessInfo {
                .access_flags = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                .stage_flags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                .image_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            };
        case rhi::AccessFlags::DEPTH_STENCIL_ATTACHMENT_WRITE:
            return AccessInfo {
                .access_flags = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                .stage_flags = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                               VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
                .image_layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            };
    }

    core::panic("Invalid enum");
}

VkImageLayout map_access_flags_to_image_layout(rhi::AccessFlags flags) noexcept
{
    core::Option<VkImageLayout> image_layout = std::nullopt;
    if (flags != rhi::AccessFlags::NONE) {
        for (usize i = 0; i <= static_cast<usize>(rhi::AccessFlags::MAX_VALUE); ++i) {
            const rhi::AccessFlags flag = static_cast<rhi::AccessFlags>(1 << i);

            if (contains(flags, flag)) {
                const AccessInfo access_info = get_access_info(flag);

                if (image_layout) {
                    const VkImageLayout new_layout = access_info.image_layout;
                    const VkImageLayout old_layout = *image_layout;
                    image_layout = [&] {
                        if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED) {
                            // In this case we allow any layout.
                            return new_layout;
                        } else if (new_layout == VK_IMAGE_LAYOUT_UNDEFINED) {
                            return old_layout;
                        } else if (
                            (old_layout ==
                             VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL) &&
                            (new_layout ==
                             VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)) {
                            // `DEPTH_STENCIL_ATTACHMENT_OPTIMAL` allows read and write access.
                            return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                        } else if (
                            (old_layout ==
                             VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) &&
                            (new_layout ==
                             VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL)) {
                            // `DEPTH_STENCIL_ATTACHMENT_OPTIMAL` allows read and write access.
                            return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                        } else if (
                            (old_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) &&
                            (new_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)) {
                            return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                        } else {
                            // At this point we don't have any valid combinations, so go with GENERAL.
                            return VK_IMAGE_LAYOUT_GENERAL;
                        }
                    }();
                } else {
                    image_layout = access_info.image_layout;
                }
            }
        }
    }

    return image_layout.value_or(VK_IMAGE_LAYOUT_UNDEFINED);
}

VkAccessFlags to_access_flags(const VkImageLayout image_layout) noexcept
{
    switch (image_layout) {
        case VK_IMAGE_LAYOUT_UNDEFINED:
        case VK_IMAGE_LAYOUT_GENERAL:
        case VK_IMAGE_LAYOUT_PREINITIALIZED: {
            return 0;
        }
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL: {
            return VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        }
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
        case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:
        case VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL: {
            return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                   VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        }
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
        case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL:
        case VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL: {
            return VK_ACCESS_SHADER_READ_BIT |
                   VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
        }
        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL: {
            return VK_ACCESS_SHADER_READ_BIT;
        }
        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL: {
            return VK_ACCESS_TRANSFER_READ_BIT;
        }
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL: {
            return VK_ACCESS_TRANSFER_WRITE_BIT;
        }
        case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL:
        case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL:
            return VK_ACCESS_SHADER_READ_BIT |
                   VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                   VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR: {
            return VK_ACCESS_MEMORY_READ_BIT;
        }
        case VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT: {
            return VK_ACCESS_FRAGMENT_DENSITY_MAP_READ_BIT_EXT;
        }

        default:
            core::panic("Unsupported layout!");
    }
}

VkPipelineStageFlags image_layout_to_pipeline_stage(
    const VkImageLayout image_layout) noexcept
{
    switch (image_layout) {
        case VK_IMAGE_LAYOUT_UNDEFINED:
        case VK_IMAGE_LAYOUT_GENERAL:
        case VK_IMAGE_LAYOUT_PREINITIALIZED: {
            return 0;
        }
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL: {
            return VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        }
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
        case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:
        case VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL: {
            return VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                   VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        }
        case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL:
        case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL:
        case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL:
        case VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL:
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL: {
            return VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT |
                   VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                   VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        }
        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL: {
            return VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL: {
            return VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR: {
            return VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        }

        default:
            core::panic("Unsupported layout!");
    }
}

VkPipelineStageFlags map_synchronization_stage(
    const rhi::SynchronizationStage stage_mask) noexcept
{
    using rhi::SynchronizationStage;
    VkPipelineStageFlags flags {};

    if (stage_mask == SynchronizationStage::NONE) {
        return flags;
    }

    if (contains(stage_mask, SynchronizationStage::TOP_OF_PIPE)) {
        flags |= VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    }
    if (contains(stage_mask, SynchronizationStage::BOTTOM_OF_PIPE)) {
        flags |= VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    }
    if (contains(stage_mask, SynchronizationStage::EARLY_FRAGMENT_TESTS)) {
        flags |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    }
    if (contains(stage_mask, SynchronizationStage::LATE_FRAGMENT_TESTS)) {
        flags |= VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    }
    if (contains(stage_mask, SynchronizationStage::VERTEX_SHADER)) {
        flags |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
    }
    if (contains(stage_mask, SynchronizationStage::FRAGMENT_SHADER)) {
        flags |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    if (contains(stage_mask, SynchronizationStage::COMPUTE_SHADER)) {
        flags |= VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    }
    if (contains(stage_mask, SynchronizationStage::TRANSFER)) {
        flags |= VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    if (contains(stage_mask, SynchronizationStage::ALL_GRAPHICS)) {
        flags |= VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
    }

    return flags;
}

} // namespace tundra::vulkan_rhi::helpers
