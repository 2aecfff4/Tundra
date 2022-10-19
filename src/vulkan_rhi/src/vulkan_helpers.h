#pragma once
#include "core/std/containers/string.h"
#include "core/std/shared_ptr.h"
#include "core/std/tuple.h"
#include "math/vector4.h"
#include "rhi/enums.h"
#include "rhi/resources/access_flags.h"
#include "rhi/resources/buffer.h"
#include "rhi/resources/graphics_pipeline.h"
#include "rhi/resources/render_pass.h"
#include "rhi/resources/sampler.h"
#include "rhi/resources/shader.h"
#include "rhi/resources/texture.h"
#include "vulkan_utils.h"

namespace tundra::vulkan_rhi {
class VulkanRawDevice;
} // namespace tundra::vulkan_rhi

namespace tundra::vulkan_rhi::helpers {

template <typename A>
constexpr void chain_structs([[maybe_unused]] A& a) noexcept
{
    // noop
}

template <typename A, typename B, typename... Args>
constexpr void chain_structs(A& a, B& b, Args&... args) noexcept
{
    a.pNext = &b;
    chain_structs(b, args...);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Texture

[[nodiscard]] VkImageCreateInfo map_texture_create_info(
    const rhi::TextureCreateInfo& create_info) noexcept;

[[nodiscard]] VkFormat map_texture_format(
    const rhi::TextureFormat texture_format) noexcept;

[[nodiscard]] VkExtent3D map_extent(const rhi::Extent& extent) noexcept;

[[nodiscard]] VkSampleCountFlagBits map_sample_count(
    const rhi::SampleCount sample_count) noexcept;

[[nodiscard]] VkImageTiling map_texture_tiling(
    const rhi::TextureTiling texture_tiling) noexcept;

[[nodiscard]] rhi::TextureUsageFlags patch_texture_usage_flags(
    rhi::TextureUsageFlags texture_usage) noexcept;

[[nodiscard]] VkImageUsageFlags map_texture_usage(
    const rhi::TextureUsageFlags texture_usage) noexcept;

[[nodiscard]] bool is_layout_allowed(
    const VkImageLayout image_layout, const rhi::TextureUsageFlags usage) noexcept;

[[nodiscard]] VkAttachmentLoadOp map_attachment_load_op(
    const rhi::AttachmentLoadOp op) noexcept;

[[nodiscard]] VkAttachmentStoreOp map_attachment_store_op(
    const rhi::AttachmentStoreOp op) noexcept;

[[nodiscard]] VkImageSubresourceRange image_subresource_range() noexcept;

[[nodiscard]] VkImageAspectFlags map_texture_aspect(
    const rhi::TextureAspectFlags aspect_flags) noexcept;

/////////////////////////////////////////////////////////////////////////////////////////
// Buffer

[[nodiscard]] VkBufferUsageFlags map_buffer_usage(
    const rhi::BufferUsageFlags buffer_usage) noexcept;

/////////////////////////////////////////////////////////////////////////////////////////
// Shader

[[nodiscard]] VkShaderStageFlagBits map_shader_stage(
    const rhi::ShaderStage shader_stage) noexcept;

/////////////////////////////////////////////////////////////////////////////////////////
// Graphics pipeline

[[nodiscard]] VkPrimitiveTopology map_primitive_type(
    const rhi::PrimitiveType primitive_type) noexcept;

[[nodiscard]] VkPolygonMode map_polygon_mode(const rhi::PolygonMode polygon_mode) noexcept;

[[nodiscard]] VkCullModeFlags map_culling_mode(
    const rhi::CullingMode culling_mode) noexcept;

[[nodiscard]] VkFrontFace map_front_face(const rhi::FrontFace front_face) noexcept;

[[nodiscard]] VkCompareOp map_compare_op(const rhi::CompareOp op) noexcept;

[[nodiscard]] VkStencilOp map_stencil_op(const rhi::StencilOp op) noexcept;

[[nodiscard]] VkStencilOpState map_stencil_op_desc(
    const rhi::StencilOpDesc& desc) noexcept;

[[nodiscard]] VkColorComponentFlags map_color_write_mask(
    const rhi::ColorWriteMask mask) noexcept;

[[nodiscard]] VkBlendFactor map_blend_factor(const rhi::BlendFactor blend_factor) noexcept;

[[nodiscard]] core::Tuple<VkBlendOp, VkBlendFactor, VkBlendFactor> map_blend_op(
    const rhi::BlendOp::Op& op) noexcept;

[[nodiscard]] VkClearValue map_clear_value(const rhi::ClearValue& clear_value) noexcept;

/////////////////////////////////////////////////////////////////////////////////////////
// Sampler

[[nodiscard]] VkFilter map_sampler_filer(const rhi::SamplerFilter filter) noexcept;
[[nodiscard]] VkSamplerAddressMode map_sampler_address_mode(
    const rhi::SamplerAddressMode address_mode) noexcept;

/////////////////////////////////////////////////////////////////////////////////////////
// Debug

/// If `name` is `nullptr`, then this function does nothing.
void set_object_name(
    const core::SharedPtr<VulkanRawDevice>& raw_device,
    const u64 handle,
    const VkObjectType object_type,
    const char* name) noexcept;

void begin_region(
    const core::SharedPtr<VulkanRawDevice>& raw_device,
    const VkCommandBuffer command_buffer,
    const core::String& name,
    const math::Vec4& color) noexcept;

void end_region(
    const core::SharedPtr<VulkanRawDevice>& raw_device,
    const VkCommandBuffer command_buffer) noexcept;

/////////////////////////////////////////////////////////////////////////////////////////
// Barriers

struct AccessInfo {
    VkAccessFlags access_flags;
    VkPipelineStageFlags stage_flags;
    VkImageLayout image_layout;
};

[[nodiscard]] AccessInfo get_access_info(const rhi::AccessFlags flag) noexcept;

[[nodiscard]] VkImageLayout map_access_flags_to_image_layout(
    rhi::AccessFlags flags) noexcept;

[[nodiscard]] VkAccessFlags to_access_flags(const VkImageLayout image_layout) noexcept;

[[nodiscard]] VkPipelineStageFlags image_layout_to_pipeline_stage(
    const VkImageLayout image_layout) noexcept;

[[nodiscard]] VkPipelineStageFlags map_synchronization_stage(
    const rhi::SynchronizationStage stage_mask) noexcept;

} // namespace tundra::vulkan_rhi::helpers
