#include "commands/vulkan_barrier.h"
#include "core/std/assert.h"
#include "core/std/panic.h"
#include "core/std/tuple.h"
#include "resources/vulkan_buffer.h"
#include "resources/vulkan_texture.h"
#include "rhi/resources/access_flags.h"
#include "vulkan/vulkan_core.h"
#include "vulkan_device.h"
#include "vulkan_helpers.h"

namespace tundra::vulkan_rhi {

[[nodiscard]] core::Tuple<VkAccessFlags2, VkPipelineStageFlags2, VkImageLayout>
    translate_access_flags(
        const rhi::TextureAccessFlags flags, const bool supports_mesh_shaders)
{
    VkAccessFlags2 access_flags = VK_ACCESS_2_NONE;
    VkPipelineStageFlags2 pipeline_stage_flags = VK_PIPELINE_STAGE_2_NONE;
    const VkImageLayout image_layout //
        = helpers::map_texture_access_flags_to_image_layout(flags, supports_mesh_shaders);

    if (flags != rhi::TextureAccessFlags::NONE) {
        constexpr usize num_bits = (sizeof(flags) * 8);
        for (usize i = 0; i <= num_bits; ++i) {
            const rhi::TextureAccessFlags flag //
                = static_cast<rhi::TextureAccessFlags>(1 << i);

            if (contains(flags, flag)) {
                const helpers::AccessInfo access_info //
                    = helpers::get_access_info(flag, supports_mesh_shaders);
                pipeline_stage_flags |= access_info.stage_flags;

                // #NOTE: `_READ` flags passed into the `srcAccessMask` are redundant.
                // It does not make sense to make reads available (flush caches),
                // but the spec does not forbid it.
                access_flags |= access_info.access_flags;
            }
        }
    }

    return core::make_tuple(access_flags, pipeline_stage_flags, image_layout);
}

[[nodiscard]] core::Tuple<VkAccessFlags2, VkPipelineStageFlags2> translate_access_flags(
    const rhi::BufferAccessFlags flags, const bool supports_mesh_shaders)
{
    VkAccessFlags2 access_flags = VK_ACCESS_2_NONE;
    VkPipelineStageFlags2 pipeline_stage_flags = VK_PIPELINE_STAGE_2_NONE;

    if (flags != rhi::BufferAccessFlags::NONE) {
        constexpr usize num_bits = (sizeof(flags) * 8);
        for (usize i = 0; i <= num_bits; ++i) {
            const rhi::BufferAccessFlags flag //
                = static_cast<rhi::BufferAccessFlags>(1 << i);

            if (contains(flags, flag)) {
                const helpers::AccessInfo access_info //
                    = helpers::get_access_info(flag, supports_mesh_shaders);
                pipeline_stage_flags |= access_info.stage_flags;

                // #NOTE: `_READ` flags passed into the `srcAccessMask` are redundant.
                // It does not make sense to make reads available (flush caches),
                // but the spec does not forbid it.
                access_flags |= access_info.access_flags;
            }
        }
    }

    return core::make_tuple(access_flags, pipeline_stage_flags);
}

[[nodiscard]] core::Tuple<VkAccessFlags2, VkPipelineStageFlags2> translate_access_flags(
    const rhi::GlobalAccessFlags flags, const bool supports_mesh_shaders)
{
    VkAccessFlags2 access_flags = VK_ACCESS_2_NONE;
    VkPipelineStageFlags2 pipeline_stage_flags = VK_PIPELINE_STAGE_2_NONE;

    if (flags != rhi::GlobalAccessFlags::NONE) {
        constexpr usize num_bits = (sizeof(flags) * 8);
        for (usize i = 0; i <= num_bits; ++i) {
            const rhi::GlobalAccessFlags flag //
                = static_cast<rhi::GlobalAccessFlags>(1 << i);

            if (contains(flags, flag)) {
                const helpers::AccessInfo access_info //
                    = helpers::get_access_info(flag, supports_mesh_shaders);
                pipeline_stage_flags |= access_info.stage_flags;

                // #NOTE: `_READ` flags passed into `srcAccessMask` are redundant.
                // It does not make sense to make reads available,
                // but the spec does not forbid this
                access_flags |= access_info.access_flags;
            }
        }
    }

    return core::make_tuple(access_flags, pipeline_stage_flags);
}

[[nodiscard]] core::Tuple<u32, u32> get_queue_family_indices(
    const core::SharedPtr<VulkanRawDevice>& raw_device,
    core::Option<rhi::QueueType> source_queue,
    core::Option<rhi::QueueType> destination_queue) noexcept
{
    tndr_assert(
        (source_queue.has_value() == destination_queue.has_value()),
        "Both `source_queue` and `destination_queue` must be a valid value, or "
        "`std::nullopt`.");

    if (!source_queue && !destination_queue) {
        return core::make_tuple(VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED);
    } else {
        const VulkanQueues& queues = raw_device->get_queues();
        const auto get_queue_index = [&](const rhi::QueueType queue) {
            switch (queue) {
                case rhi::QueueType::Compute:
                    return core::get<u32>(queues.compute_queue);
                case rhi::QueueType::Graphics:
                    return core::get<u32>(queues.graphics_queue);
                case rhi::QueueType::Transfer:
                    return core::get<u32>(queues.transfer_queue);
                case rhi::QueueType::Present:
                    return core::get<u32>(queues.present_queue);
            }

            core::panic("Invalid enum!");
        };

        // https://www.khronos.org/registry/vulkan/specs/1.2/html/vkspec.html#synchronization-queue-transfers
        // "If the values of srcQueueFamilyIndex and dstQueueFamilyIndex are equal, no ownership transfer is performed,
        // and the barrier operates as if they were both set to VK_QUEUE_FAMILY_IGNORED."
        return core::make_tuple(
            get_queue_index(*source_queue), get_queue_index(*destination_queue));
    }
}

VulkanBarrier::VulkanBarrier(const core::SharedPtr<VulkanRawDevice>& raw_device) noexcept
    : m_raw_device(raw_device)
    , m_supports_mesh_shaders(raw_device->supported_features().mesh_shaders)
{
    m_buffer_barriers.reserve(16);
    m_image_barriers.reserve(16);
    m_memory_barriers.reserve(16);
}

void VulkanBarrier::image_layout_transition(
    const VkImage image,
    const VkImageLayout old_layout,
    const VkImageLayout new_layout,
    const VkImageSubresourceRange& subresource_range) noexcept
{
    const VkPipelineStageFlags2 src_stage_mask //
        = helpers::image_layout_to_pipeline_stage(old_layout);
    const VkPipelineStageFlags2 dst_stage_mask //
        = helpers::image_layout_to_pipeline_stage(new_layout);

    tndr_assert(src_stage_mask != VK_PIPELINE_STAGE_2_NONE, "Invalid stage mask");
    tndr_assert(dst_stage_mask != VK_PIPELINE_STAGE_2_NONE, "Invalid stage mask");

    VkImageMemoryBarrier2 image_barrier {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .srcStageMask = src_stage_mask,
        .srcAccessMask = helpers::to_access_flags(old_layout),
        .dstStageMask = dst_stage_mask,
        .dstAccessMask = helpers::to_access_flags(new_layout),
        .oldLayout = old_layout,
        .newLayout = new_layout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = image,
        .subresourceRange = subresource_range,
    };
    m_image_barriers.push_back(core::move(image_barrier));
}

void VulkanBarrier::global_barrier(const rhi::GlobalBarrier& barrier) noexcept
{
    const auto [src_access_mask, src_stage_mask] //
        = translate_access_flags(barrier.previous_access, m_supports_mesh_shaders);
    const auto [dst_access_mask, dst_stage_mask] //
        = translate_access_flags(barrier.next_access, m_supports_mesh_shaders);

    tndr_assert(src_stage_mask != VK_PIPELINE_STAGE_2_NONE, "Invalid stage mask");
    tndr_assert(dst_stage_mask != VK_PIPELINE_STAGE_2_NONE, "Invalid stage mask");

    VkMemoryBarrier2 memory_barrier {
        .sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2,
        .srcStageMask = src_stage_mask,
        .srcAccessMask = src_access_mask,
        .dstStageMask = dst_stage_mask,
        .dstAccessMask = dst_access_mask,
    };
    m_memory_barriers.push_back(core::move(memory_barrier));
}

void VulkanBarrier::texture_barrier(
    const VulkanTexture& texture, const rhi::TextureBarrier& barrier) noexcept
{
    const auto [src_access_mask, src_stage_mask, old_layout] //
        = translate_access_flags(barrier.previous_access, m_supports_mesh_shaders);
    const auto [dst_access_mask, dst_stage_mask, new_layout] //
        = translate_access_flags(barrier.next_access, m_supports_mesh_shaders);
    const auto [src_queue_family_index, dst_queue_family_index] //
        = get_queue_family_indices(
            m_raw_device, //
            barrier.source_queue,
            barrier.destination_queue);

    tndr_assert(src_stage_mask != VK_PIPELINE_STAGE_2_NONE, "Invalid stage mask");
    tndr_assert(dst_stage_mask != VK_PIPELINE_STAGE_2_NONE, "Invalid stage mask");

    const rhi::TextureUsageFlags texture_usage = texture.get_usage();
    if (!helpers::is_layout_allowed(old_layout, texture_usage)) {
        core::panic("{} is not allowed with: {}.", old_layout, texture_usage);
    }

    if (!helpers::is_layout_allowed(new_layout, texture_usage)) {
        core::panic("{} is not allowed with: {}.", new_layout, texture_usage);
    }

    const VkImageAspectFlags aspect_mask = [&] {
        const rhi::TextureFormat texture_format = texture.get_format();
        const rhi::TextureFormatDesc desc = rhi::get_texture_format_desc(texture_format);

        return helpers::map_texture_aspect(desc.aspect);
    }();

    VkImageMemoryBarrier2 image_barrier {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .srcStageMask = src_stage_mask,
        .srcAccessMask = src_access_mask,
        .dstStageMask = dst_stage_mask,
        .dstAccessMask = dst_access_mask,
        .oldLayout = old_layout,
        .newLayout = new_layout,
        .srcQueueFamilyIndex = src_queue_family_index,
        .dstQueueFamilyIndex = dst_queue_family_index,
        .image = texture.get_image(),
        .subresourceRange =
            VkImageSubresourceRange {
                .aspectMask = aspect_mask,
                .baseMipLevel = barrier.subresource_range.first_mip_level,
                .levelCount = barrier.subresource_range.mip_count,
                .baseArrayLayer = barrier.subresource_range.first_layer,
                .layerCount = barrier.subresource_range.layer_count,
            },
    };

    m_image_barriers.push_back(core::move(image_barrier));
}

void VulkanBarrier::buffer_barrier(
    const VulkanBuffer& buffer, const rhi::BufferBarrier& barrier) noexcept
{
    const auto [src_access_mask, src_stage_mask] //
        = translate_access_flags(barrier.previous_access, m_supports_mesh_shaders);
    const auto [dst_access_mask, dst_stage_mask] //
        = translate_access_flags(barrier.next_access, m_supports_mesh_shaders);
    const auto [src_queue_family_index, dst_queue_family_index] //
        = get_queue_family_indices(
            m_raw_device, //
            barrier.source_queue,
            barrier.destination_queue);

    VkBufferMemoryBarrier2 buffer_barrier {
        .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
        .srcStageMask = src_stage_mask,
        .srcAccessMask = src_access_mask,
        .dstStageMask = dst_stage_mask,
        .dstAccessMask = dst_access_mask,
        .srcQueueFamilyIndex = src_queue_family_index,
        .dstQueueFamilyIndex = dst_queue_family_index,
        .buffer = buffer.get_buffer(),
        .offset = barrier.subresource_range.offset,
        .size = barrier.subresource_range.size,
    };
    m_buffer_barriers.push_back(core::move(buffer_barrier));
}

void VulkanBarrier::execute(const VkCommandBuffer command_buffer) const noexcept
{
    if (!m_memory_barriers.empty() || !m_buffer_barriers.empty() ||
        !m_image_barriers.empty()) {
        const VkDependencyInfo dependency_info {
            .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .memoryBarrierCount = static_cast<u32>(m_memory_barriers.size()),
            .pMemoryBarriers = m_memory_barriers.data(),
            .bufferMemoryBarrierCount = static_cast<u32>(m_buffer_barriers.size()),
            .pBufferMemoryBarriers = m_buffer_barriers.data(),
            .imageMemoryBarrierCount = static_cast<u32>(m_image_barriers.size()),
            .pImageMemoryBarriers = m_image_barriers.data(),
        };
        m_raw_device->get_device().cmd_pipeline_barrier2(
            command_buffer, &dependency_info);
    }
}

void VulkanBarrier::reset() noexcept
{
    m_buffer_barriers.clear();
    m_image_barriers.clear();
    m_memory_barriers.clear();
}

} // namespace tundra::vulkan_rhi
