#include "commands/vulkan_submit_work_scheduler.h"
#include "commands/vulkan_command_decoder.h"
#include "core/profiler.h"
#include "resources/vulkan_swapchain.h"
#include "resources/vulkan_texture.h"
#include "vulkan_device.h"
#include "vulkan_helpers.h"

namespace tundra::vulkan_rhi {

VulkanSubmitWorkScheduler::VulkanSubmitWorkScheduler(
    core::SharedPtr<VulkanRawDevice> raw_device, Managers managers) noexcept
    : m_raw_device(raw_device)
    , m_managers(managers)
{
    TNDR_PROFILER_TRACE("VulkanSubmitWorkScheduler::VulkanSubmitWorkScheduler");

    for (VkSemaphore& semaphore : m_present_semaphores) {
        const VkSemaphoreCreateInfo create_info {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        };
        semaphore = vulkan_map_result(
            m_raw_device->get_device().create_semaphore(create_info, nullptr),
            "`create_semaphore` failed");
    }

    m_timeline_semaphore = [&] {
        VkSemaphoreTypeCreateInfo type_create_info {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
            .semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE,
            .initialValue = 0,
        };

        VkSemaphoreCreateInfo create_info {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        };

        helpers::chain_structs(create_info, type_create_info);
        const VkSemaphore semaphore = vulkan_map_result(
            m_raw_device->get_device().create_semaphore(create_info, nullptr),
            "`create_semaphore` failed");
        return core::make_tuple(semaphore, 0);
    }();
}

VulkanSubmitWorkScheduler::~VulkanSubmitWorkScheduler() noexcept
{
    TNDR_PROFILER_TRACE("VulkanSubmitWorkScheduler::~VulkanSubmitWorkScheduler");

    for (VkSemaphore& semaphore : m_present_semaphores) {
        m_raw_device->get_device().destroy_semaphore(semaphore, nullptr);
    }

    m_raw_device->get_device().destroy_semaphore(
        core::get<VkSemaphore>(m_timeline_semaphore), nullptr);
}

void VulkanSubmitWorkScheduler::submit(
    core::Array<rhi::SubmitInfo> submit_infos,
    core::Array<rhi::PresentInfo> present_infos) noexcept
{
    TNDR_PROFILER_TRACE("VulkanSubmitWorkScheduler::submit");

    Managers& managers = m_managers;
    managers.command_buffer_manager->wait_for_free_pool();

    struct SubmitData {
        core::Array<VkCommandBuffer> command_buffers;
        rhi::SynchronizationStage synchronization_stage;
        rhi::QueueType queue_type;
    };

    core::Array<SubmitData> submit_data;
    submit_data.reserve(submit_infos.size());

    for (const rhi::SubmitInfo& submit_info : submit_infos) {
        core::Array<VkCommandBuffer> command_buffers;

        for (const rhi::CommandEncoder& command_encoder : submit_info.encoders) {
            VulkanCommandBufferManager::CommandBundle bundle =
                managers.command_buffer_manager->get_command_bundle(
                    submit_info.queue_type);

            VulkanCommandDecoder decoder(m_raw_device, m_managers, core::move(bundle));
            const VkCommandBuffer command_buffer = decoder.decode(command_encoder);

            command_buffers.push_back(command_buffer);
        }

        submit_data.push_back(SubmitData {
            .command_buffers = core::move(command_buffers),
            .synchronization_stage = submit_info.synchronization_stage,
            .queue_type = submit_info.queue_type,
        });
    }

    const VkFence synchronization_fence = managers.command_buffer_manager->get_fence();
    const usize num_present_infos = present_infos.size();

    for (usize i = 0; i < submit_data.size(); ++i) {
        const SubmitData& data = submit_data[i];
        const bool submit_with_synchronization_fence = (i == (submit_data.size() - 1)) &&
                                                       (num_present_infos == 0);

        const u64 value_wait = core::get<u64>(m_timeline_semaphore);
        const u64 value_signal = value_wait + 1;
        const VkPipelineStageFlags flags = helpers::map_synchronization_stage(
            data.synchronization_stage);

        VkTimelineSemaphoreSubmitInfo timeline_semaphore_submit_info = [&] {
            VkTimelineSemaphoreSubmitInfo submit_info {
                .sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO,
                .signalSemaphoreValueCount = 1,
                .pSignalSemaphoreValues = &value_signal,
            };

            if (i != 0) {
                submit_info.waitSemaphoreValueCount = 1;
                submit_info.pWaitSemaphoreValues = &value_wait;
            }

            return submit_info;
        }();

        VkSubmitInfo submit_info = [&] {
            VkSubmitInfo submit_info {
                .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                .commandBufferCount = static_cast<u32>(data.command_buffers.size()),
                .pCommandBuffers = data.command_buffers.data(),
                .signalSemaphoreCount = 1,
                .pSignalSemaphores = &core::get<VkSemaphore>(m_timeline_semaphore),
            };

            if (i != 0) {
                submit_info.waitSemaphoreCount = 1;
                submit_info.pWaitSemaphores = &core::get<VkSemaphore>(
                    m_timeline_semaphore);
                submit_info.pWaitDstStageMask = &flags;
            }

            return submit_info;
        }();

        helpers::chain_structs(submit_info, timeline_semaphore_submit_info);

        m_raw_device->queue_submit(
            data.queue_type,
            core::as_span(submit_info),
            submit_with_synchronization_fence ? synchronization_fence : VK_NULL_HANDLE);

        core::get<u64>(m_timeline_semaphore) += 1;
    }

    // Copy textures to swapchains.
    if (num_present_infos > 0) {
        // Frame graph should transfer ownership of resources to correct queues,
        // our job it's just to copy textures to swapchains, and present.

        core::Array<VkSemaphore> swapchain_image_semaphores;
        core::Array<u32> swapchain_image_indices;
        core::Array<VkSwapchainKHR> swapchains;

        VulkanCommandBufferManager::CommandBundle command_bundle =
            managers.command_buffer_manager->get_command_bundle(rhi::QueueType::Present);
        auto resources = command_bundle.get_resources();

        const VkCommandBufferBeginInfo command_buffer_begin_info {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        };

        vulkan_map_result(
            m_raw_device->get_device().begin_command_buffer(
                command_bundle.command_buffer, command_buffer_begin_info),
            "`begin_command_buffer` failed");

        helpers::begin_region(
            m_raw_device,
            command_bundle.command_buffer,
            "copy_textures_to_swapchains",
            { 1.0, 0.75, 0.05, 1.0 });

        for (const rhi::PresentInfo& present_info : present_infos) {
            resources->add_reference(
                *managers.resource_tracker, present_info.swapchain.get_handle().get_id());
            resources->add_reference(
                *managers.resource_tracker, present_info.texture.get_handle().get_id());

            const auto
                [swapchain_image_index,
                 swapchain_semaphore,
                 swapchain_attachment,
                 swapchain] =
                    *managers.swapchain_manager->with_mut(
                        present_info.swapchain.get_handle(),
                        [&](VulkanSwapchain& swapchain) {
                            auto result = swapchain.acquire_image_index();

                            const u32 image_index = core::get<u32>(result);
                            const VkSemaphore semaphore = core::get<VkSemaphore>(result);
                            const SwapchainAttachment attachment =
                                core::get<SwapchainAttachment&>(result);
                            const VkSwapchainKHR vk_swapchain = swapchain.get_swapchain();

                            core::get<SwapchainAttachment&>(result)
                                .image_layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

                            return core::make_tuple(
                                image_index, semaphore, attachment, vk_swapchain);
                        });

            const auto [texture_image, texture_extent, texture_usage] =
                *managers.texture_manager->with(
                    present_info.texture.get_handle(), [](const VulkanTexture& texture) {
                        return core::make_tuple(
                            texture.get_image(),
                            texture.get_extent(),
                            texture.get_usage());
                    });

            swapchain_image_indices.push_back(swapchain_image_index);
            swapchain_image_semaphores.push_back(swapchain_semaphore);
            swapchains.push_back(swapchain);

            VulkanBarrier barrier(m_raw_device);
            // Swapchain image barrier. image_layout -> TRANSFER_DST_OPTIMAL.
            barrier.image_layout_transition(
                swapchain_attachment.image,
                swapchain_attachment.image_layout,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                helpers::image_subresource_range());

            const VkImageLayout texture_old_layout =
                helpers::map_access_flags_to_image_layout(
                    present_info.texture_previous_access);

            if (!helpers::is_layout_allowed(texture_old_layout, texture_usage)) {
                core::panic(
                    "{} is not allowed with: {}.", texture_old_layout, texture_usage);
            }

            // Texture barrier. present_info.texture_previous_access -> TRANSFER_SRC_OPTIMAL.
            if (present_info.texture_previous_access != rhi::AccessFlags::TRANSFER_READ) {
                barrier.image_layout_transition(
                    texture_image,
                    texture_old_layout,
                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    helpers::image_subresource_range());
            }

            barrier.execute(command_bundle.command_buffer);
            barrier.reset();

            // Copy
            // #NOTE: This will work on ~99% of vulkan devices.
            const VkExtent3D extent = helpers::map_extent(texture_extent);
            const VkImageBlit region {
                .srcSubresource =
                    VkImageSubresourceLayers {
                        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                        .layerCount = 1,
                    },
                .srcOffsets = { VkOffset3D {},
                                VkOffset3D {
                                    .x = (i32)extent.width,
                                    .y = (i32)extent.height,
                                    .z = 1,
                                } },
                .dstSubresource =
                    VkImageSubresourceLayers {
                        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                        .layerCount = 1,
                    },
                .dstOffsets = { VkOffset3D {},
                                VkOffset3D {
                                    .x = (i32)extent.width,
                                    .y = (i32)extent.height,
                                    .z = 1,
                                } },
            };

            // #TODO:
            // [VUID-vkCmdBlitImage-srcImage-00230 (569695033)] : Validation Error: [ VUID-vkCmdBlitImage-srcImage-00230 ]
            // Object 0: handle = 0x20b0b4721c0, name = Command Buffer | Thread: 3333171275362006274 | Queue: 3,
            // type = VK_OBJECT_TYPE_COMMAND_BUFFER; | MessageID = 0x21f4db39 |
            // vkCmdBlitImage: If one of srcImage and dstImage images has unsigned integer format,
            // the other one must also have unsigned integer format.  Source format is VK_FORMAT_R32_UINT Destination format
            // is VK_FORMAT_R16G16B16A16_UNORM. The Vulkan spec states: If either of srcImage or dstImage was created with an unsigned integer VkFormat,
            // the other must also have been created with an unsigned integer VkFormat
            // (https://vulkan.lunarg.com/doc/view/1.3.204.1/windows/1.3-extensions/vkspec.html#VUID-vkCmdBlitImage-srcImage-00230)
            m_raw_device->get_device().cmd_blit_image(
                command_bundle.command_buffer,
                texture_image,
                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                swapchain_attachment.image,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                core::as_span(region),
                VK_FILTER_LINEAR);

            // Swapchain image barrier. TRANSFER_DST_OPTIMAL -> PRESENT_SRC_KHR
            barrier.image_layout_transition(
                swapchain_attachment.image,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                helpers::image_subresource_range());

            barrier.execute(command_bundle.command_buffer);
            barrier.reset();
        }

        helpers::end_region(m_raw_device, command_bundle.command_buffer);

        vulkan_map_result(
            m_raw_device->get_device().end_command_buffer(command_bundle.command_buffer),
            "`end_command_buffer` failed");

        // Submit work to a GPU.
        const VkSemaphore present_semaphore =
            m_present_semaphores[m_submit_counter % rhi::config::MAX_FRAMES_IN_FLIGHT];
        swapchain_image_semaphores.push_back(present_semaphore);

        const u64 value_wait = core::get<u64>(m_timeline_semaphore);
        const u64 values_signal[] = {
            value_wait + 1,
            0,
        };
        const VkSemaphore signal_semaphores[] = {
            core::get<VkSemaphore>(m_timeline_semaphore),
            present_semaphore,
        };

        VkTimelineSemaphoreSubmitInfo timeline_semaphore_submit_info {
            .sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO,
            .waitSemaphoreValueCount = 1,
            .pWaitSemaphoreValues = &value_wait,
            .signalSemaphoreValueCount = static_cast<u32>(std::size(values_signal)),
            .pSignalSemaphoreValues = values_signal,
        };

        const VkPipelineStageFlags flags = helpers::map_synchronization_stage(
            rhi::SynchronizationStage::BOTTOM_OF_PIPE);

        VkSubmitInfo submit_info {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &core::get<VkSemaphore>(m_timeline_semaphore),
            .pWaitDstStageMask = &flags,
            .commandBufferCount = 1,
            .pCommandBuffers = &command_bundle.command_buffer,
            .signalSemaphoreCount = static_cast<u32>(std::size(signal_semaphores)),
            .pSignalSemaphores = signal_semaphores,
        };
        helpers::chain_structs(submit_info, timeline_semaphore_submit_info);

        m_raw_device->queue_submit(
            rhi::QueueType::Present, core::as_span(submit_info), synchronization_fence);

        core::get<u64>(m_timeline_semaphore) += 1;

        // Present
        const VkPresentInfoKHR present_info {
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .waitSemaphoreCount = static_cast<u32>(swapchain_image_semaphores.size()),
            .pWaitSemaphores = swapchain_image_semaphores.data(),
            .swapchainCount = static_cast<u32>(swapchains.size()),
            .pSwapchains = swapchains.data(),
            .pImageIndices = swapchain_image_indices.data(),
        };
        m_raw_device->queue_present(rhi::QueueType::Present, present_info);
    }

    managers.command_buffer_manager->end_frame();
    m_submit_counter += 1;
}

} // namespace tundra::vulkan_rhi
