#include "vulkan_device.h"
#include "core/logger.h"
#include "core/profiler.h"
#include "core/std/panic.h"
#include "core/std/utils.h"
#include "managers/vulkan_command_buffer_manager.h"
#include "managers/vulkan_descriptor_bindless_manager.h"
#include "managers/vulkan_pipeline_cache_manager.h"
#include "managers/vulkan_pipeline_layout_manager.h"
#include "resources/vulkan_buffer.h"
#include "resources/vulkan_compute_pipeline.h"
#include "resources/vulkan_graphics_pipeline.h"
#include "resources/vulkan_sampler.h"
#include "resources/vulkan_shader.h"
#include "resources/vulkan_swapchain.h"
#include "resources/vulkan_texture.h"
#include "resources/vulkan_texture_view.h"
#include "vulkan_instance.h"

namespace tundra::vulkan_rhi {

/////////////////////////////////////////////////////////////////////////////////////////
// Extensions

Extensions::Extensions(
    const loader::Instance& instance, const loader::Device& device) noexcept
    : swapchain(instance, device)
    , surface(instance)
{
}

/////////////////////////////////////////////////////////////////////////////////////////
// VulkanRawDevice

VulkanRawDevice::VulkanRawDevice(
    core::SharedPtr<VulkanInstance> instance,
    loader::Device device,
    const VkPhysicalDevice physical_device,
    VulkanQueues queues,
    DeviceProperties device_properties,
    DeviceLimits device_limits) noexcept
    : m_instance(core::move(instance))
    , m_device(device)
    , m_physical_device(physical_device)
    , m_queues(queues)
    , m_device_properties(device_properties)
    , m_device_limits(device_limits)
    , m_extensions(m_instance->get_instance(), device)
{
}

VulkanRawDevice::~VulkanRawDevice() noexcept
{
    TNDR_PROFILER_TRACE("VulkanRawDevice::~VulkanRawDevice");

    this->wait_until_idle();
    m_device.destroy_device(nullptr);
}

void VulkanRawDevice::wait_until_idle() const noexcept
{
    TNDR_PROFILER_TRACE("VulkanRawDevice::wait_until_idle");

    vulkan_map_result(this->get_device().device_wait_idle(), "`device_wait_idle` failed");
}

void VulkanRawDevice::queue_submit(
    const rhi::QueueType queue_type,
    const core::Span<const VkSubmitInfo>& submit_infos,
    const VkFence fence) const noexcept
{
    TNDR_PROFILER_TRACE("VulkanRawDevice::queue_submit");

    const VkQueue queue = [&] {
        const VulkanQueues& queues = this->get_queues();
        switch (queue_type) {
            case rhi::QueueType::Compute:
                return core::get<VkQueue>(queues.compute_queue);
            case rhi::QueueType::Graphics:
                return core::get<VkQueue>(queues.graphics_queue);
            case rhi::QueueType::Transfer:
                return core::get<VkQueue>(queues.transfer_queue);
            case rhi::QueueType::Present:
                return core::get<VkQueue>(queues.present_queue);
        }

        core::panic("Invalid enum");
    }();

    vulkan_map_result(
        this->get_device().queue_submit(queue, submit_infos, fence),
        "`queue_submit` failed");
}

void VulkanRawDevice::queue_present(
    const rhi::QueueType queue_type, const VkPresentInfoKHR& present_info) noexcept
{
    TNDR_PROFILER_TRACE("VulkanRawDevice::queue_present");

    const VkQueue queue = [&] {
        const VulkanQueues& queues = this->get_queues();
        switch (queue_type) {
            case rhi::QueueType::Compute:
                return core::get<VkQueue>(queues.compute_queue);
            case rhi::QueueType::Graphics:
                return core::get<VkQueue>(queues.graphics_queue);
            case rhi::QueueType::Transfer:
                return core::get<VkQueue>(queues.transfer_queue);
            case rhi::QueueType::Present:
                return core::get<VkQueue>(queues.present_queue);
        }

        core::panic("Invalid enum");
    }();

    [[maybe_unused]] const bool is_optimal = vulkan_map_result(
        this->get_extensions().swapchain.queue_present(queue, present_info),
        "`queue_present` failed");
}

core::SharedPtr<VulkanInstance> VulkanRawDevice::get_instance() const noexcept
{
    return m_instance;
}

const loader::Device& VulkanRawDevice::get_device() const noexcept
{
    return m_device;
}

VkPhysicalDevice VulkanRawDevice::get_physical_device() const noexcept
{
    return m_physical_device;
}

const Extensions& VulkanRawDevice::get_extensions() const noexcept
{
    return m_extensions;
}

const VulkanQueues& VulkanRawDevice::get_queues() const noexcept
{
    return m_queues;
}

const DeviceProperties& VulkanRawDevice::get_device_properties() const noexcept
{
    return m_device_properties;
}

const DeviceLimits& VulkanRawDevice::get_device_limits() const noexcept
{
    return m_device_limits;
}

/////////////////////////////////////////////////////////////////////////////////////////
// VulkanDevice

VulkanDevice::VulkanDevice(
    core::SharedPtr<VulkanInstance> instance,
    loader::Device device,
    const VkPhysicalDevice physical_device,
    VulkanQueues queues,
    DeviceProperties device_properties,
    DeviceLimits device_limits) noexcept
    : m_instance(core::move(instance))
    , m_raw_device(core::make_shared<VulkanRawDevice>(
          m_instance, device, physical_device, queues, device_properties, device_limits))
    , m_allocator(core::make_shared<VulkanAllocator>(m_raw_device))
    , m_managers(m_raw_device)
    , m_submit_work_scheduler(m_raw_device, m_managers)
{
}

VulkanDevice::~VulkanDevice() noexcept
{
    TNDR_PROFILER_TRACE("VulkanDevice::~VulkanDevice");

    this->wait_until_idle();
}

void VulkanDevice::wait_until_idle() const noexcept
{
    TNDR_PROFILER_TRACE("VulkanDevice::wait_until_idle");

    vulkan_map_result(
        m_raw_device->get_device().device_wait_idle(), "`device_wait_idle` failed");
}

void VulkanDevice::gc() noexcept
{
    TNDR_PROFILER_TRACE("VulkanDevice::gc");
}

void VulkanDevice::submit(
    core::Array<rhi::SubmitInfo> submit_infos,
    core::Array<rhi::PresentInfo> present_infos) noexcept
{
    m_submit_work_scheduler.submit(core::move(submit_infos), core::move(present_infos));
    this->gc();
}

rhi::SwapchainHandle VulkanDevice::create_swapchain(
    const rhi::SwapchainCreateInfo& create_info) noexcept
{
    const rhi::SwapchainHandleType handle = m_managers.swapchain_manager->add(
        m_raw_device, create_info);

    m_managers.resource_tracker->add_resource(
        handle.get_id(), [swapchain_manager = m_managers.swapchain_manager, handle] {
            const bool result = swapchain_manager->destroy(handle);
            tndr_assert(result, "`swapchain_manager->destroy` failed!");
        });

    return rhi::SwapchainHandle { handle };
}

void VulkanDevice::destroy_swapchain(const rhi::SwapchainHandle handle) noexcept
{
    m_managers.resource_tracker->remove_reference(handle.get_handle().get_id());
}

rhi::BufferHandle VulkanDevice::create_buffer(
    const rhi::BufferCreateInfo& create_info) noexcept
{
    const rhi::BufferHandleType handle = m_managers.buffer_manager->add(
        m_raw_device, m_allocator, create_info);

    const rhi::BindableResource bindings = *m_managers.buffer_manager->with(
        handle, [&](const VulkanBuffer& b) {
            return m_managers.descriptor_bindless_manager->bind_buffer(b);
        });

    const rhi::BufferHandle buffer_handle { handle, bindings };

    m_managers.resource_tracker->add_resource(
        handle.get_id(),
        [buffer_manager = m_managers.buffer_manager,
         descriptor_bindless_manager = m_managers.descriptor_bindless_manager,
         buffer_handle] {
            descriptor_bindless_manager->unbind_buffer(buffer_handle.get_bindings());

            const bool result = buffer_manager->destroy(buffer_handle.get_handle());
            tndr_assert(result, "`buffer_manager->destroy` failed!");
        });

    return buffer_handle;
}

void VulkanDevice::update_buffer(
    const rhi::BufferHandle handle,
    const core::Array<rhi::BufferUpdateRegion>& update_regions) noexcept
{
    tndr_assert(
        m_managers.buffer_manager
            ->with_mut(
                handle.get_handle(),
                [&](VulkanBuffer& buffer) { buffer.update_buffer(update_regions); })
            .has_value(),
        "`handle` is not valid!");
}

void VulkanDevice::destroy_buffer(const rhi::BufferHandle handle) noexcept
{
    m_managers.resource_tracker->remove_reference(handle.get_handle().get_id());
}

rhi::TextureHandle VulkanDevice::create_texture(
    const rhi::TextureCreateInfo& create_info) noexcept
{
    const rhi::TextureHandleType handle = m_managers.texture_manager->add(
        m_raw_device, m_allocator, create_info);

    const rhi::BindableResource bindings = *m_managers.texture_manager->with(
        handle, [&](const VulkanTexture& texture) {
            return m_managers.descriptor_bindless_manager->bind_texture(texture);
        });

    const rhi::TextureHandle texture_handle { handle, bindings };

    m_managers.resource_tracker->add_resource(
        handle.get_id(),
        [texture_manager = m_managers.texture_manager,
         descriptor_bindless_manager = m_managers.descriptor_bindless_manager,
         texture_handle] {
            descriptor_bindless_manager->unbind_texture(texture_handle.get_bindings());

            const bool result = texture_manager->destroy(texture_handle.get_handle());
            tndr_assert(result, "`texture_manager->destroy` failed!");
        });

    return texture_handle;
}

void VulkanDevice::destroy_texture(const rhi::TextureHandle handle) noexcept
{
    m_managers.resource_tracker->remove_reference(handle.get_handle().get_id());
}

rhi::TextureViewHandle VulkanDevice::create_texture_view(
    const rhi::TextureViewCreateInfo& create_info) noexcept
{
    const rhi::TextureViewHandleType handle = m_managers.texture_view_manager->add(
        m_raw_device, m_managers, create_info);

    const rhi::BindableResource bindings = *m_managers.texture_view_manager->with(
        handle, [&](const VulkanTextureView& texture) {
            return m_managers.descriptor_bindless_manager->bind_texture_view(texture);
        });

    const rhi::TextureViewHandle texture_view_handle { handle, bindings };

    m_managers.resource_tracker->add_resource(
        handle.get_id(),
        [texture_view_manager = m_managers.texture_view_manager,
         descriptor_bindless_manager = m_managers.descriptor_bindless_manager,
         texture_view_handle] {
            descriptor_bindless_manager->unbind_texture_view(
                texture_view_handle.get_bindings());

            const bool result = texture_view_manager->destroy(
                texture_view_handle.get_handle());
            tndr_assert(result, "`texture_view_manager->destroy` failed!");
        });

    return texture_view_handle;
}

void VulkanDevice::destroy_texture_view(const rhi::TextureViewHandle handle) noexcept
{
    m_managers.resource_tracker->remove_reference(handle.get_handle().get_id());
}

rhi::ShaderHandle VulkanDevice::create_shader(
    const rhi::ShaderCreateInfo& create_info) noexcept
{
    const rhi::ShaderHandleType handle = m_managers.shader_manager->add(
        m_raw_device, create_info);

    m_managers.resource_tracker->add_resource(
        handle.get_id(), [shader_manager = m_managers.shader_manager, handle] {
            const bool result = shader_manager->destroy(handle);
            tndr_assert(result, "`shader_manager->destroy` failed!");
        });

    return rhi::ShaderHandle { handle };
}

void VulkanDevice::destroy_shader(const rhi::ShaderHandle handle) noexcept
{
    m_managers.resource_tracker->remove_reference(handle.get_handle().get_id());
}

rhi::GraphicsPipelineHandle VulkanDevice::create_graphics_pipeline(
    const rhi::GraphicsPipelineCreateInfo& create_info) noexcept
{
    const rhi::GraphicsPipelineHandleType handle =
        m_managers.graphics_pipeline_manager->add(m_raw_device, m_managers, create_info);

    m_managers.resource_tracker->add_resource(
        handle.get_id(),
        [graphics_pipeline_manager = m_managers.graphics_pipeline_manager, handle] {
            const bool result = graphics_pipeline_manager->destroy(handle);
            tndr_assert(result, "`graphics_pipeline_manager->destroy` failed!");
        });

    return rhi::GraphicsPipelineHandle { handle };
}

void VulkanDevice::destroy_graphics_pipeline(
    const rhi::GraphicsPipelineHandle handle) noexcept
{
    m_managers.resource_tracker->remove_reference(handle.get_handle().get_id());
}

rhi::ComputePipelineHandle VulkanDevice::create_compute_pipeline(
    const rhi::ComputePipelineCreateInfo& create_info) noexcept
{
    const rhi::ComputePipelineHandleType handle = m_managers.compute_pipeline_manager->add(
        m_raw_device, m_managers, create_info);

    m_managers.resource_tracker->add_resource(
        handle.get_id(),
        [compute_pipeline_manager = m_managers.compute_pipeline_manager, handle] {
            const bool result = compute_pipeline_manager->destroy(handle);
            tndr_assert(result, "`compute_pipeline_manager->destroy` failed!");
        });

    return rhi::ComputePipelineHandle { handle };
}

void VulkanDevice::destroy_compute_pipeline(
    const rhi::ComputePipelineHandle handle) noexcept
{
    m_managers.resource_tracker->remove_reference(handle.get_handle().get_id());
}

rhi::SamplerHandle VulkanDevice::create_sampler(
    const rhi::SamplerCreateInfo& create_info) noexcept
{
    const rhi::SamplerHandleType handle = m_managers.sampler_manager->add(
        m_raw_device, create_info);

    const rhi::BindableResource bindings = *m_managers.sampler_manager->with(
        handle, [&](const VulkanSampler& sampler) {
            return m_managers.descriptor_bindless_manager->bind_sampler(sampler);
        });

    const rhi::SamplerHandle sampler_handle { handle, bindings };

    m_managers.resource_tracker->add_resource(
        handle.get_id(),
        [sampler_manager = m_managers.sampler_manager,
         descriptor_bindless_manager = m_managers.descriptor_bindless_manager,
         sampler_handle] {
            descriptor_bindless_manager->unbind_sampler(sampler_handle.get_bindings());

            const bool result = sampler_manager->destroy(sampler_handle.get_handle());
            tndr_assert(result, "`sampler_manager->destroy` failed!");
        });

    return sampler_handle;
}

void VulkanDevice::destroy_sampler(const rhi::SamplerHandle handle) noexcept
{
    m_managers.resource_tracker->remove_reference(handle.get_handle().get_id());
}

core::SharedPtr<VulkanInstance> VulkanDevice::get_instance() const noexcept
{
    return m_instance;
}

const loader::Device& VulkanDevice::get_device() const noexcept
{
    return m_raw_device->get_device();
}

VkPhysicalDevice VulkanDevice::get_physical_device() const noexcept
{
    return m_raw_device->get_physical_device();
}

Managers& VulkanDevice::get_managers() noexcept
{
    return m_managers;
}

const VulkanQueues& VulkanDevice::get_queues() const noexcept
{
    return m_raw_device->get_queues();
}

const DeviceProperties& VulkanDevice::get_device_properties() const noexcept
{
    return m_raw_device->get_device_properties();
}

const DeviceLimits& VulkanDevice::get_device_limits() const noexcept
{
    return m_raw_device->get_device_limits();
}

} // namespace tundra::vulkan_rhi
