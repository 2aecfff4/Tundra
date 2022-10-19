#pragma once
#include "core/core.h"
#include "commands/vulkan_submit_work_scheduler.h"
#include "core/std/option.h"
#include "core/std/shared_ptr.h"
#include "core/std/tuple.h"
#include "loader/device.h"
#include "loader/extensions/khr/surface.h"
#include "loader/extensions/khr/swapchain.h"
#include "managers/managers.h"
#include "rhi/resources/handle.h"

namespace tundra::rhi {
struct SwapchainCreateInfo;
struct BufferCreateInfo;
struct TextureCreateInfo;
struct ShaderCreateInfo;
struct GraphicsPipelineCreateInfo;
struct ComputePipelineCreateInfo;
struct SamplerCreateInfo;
} // namespace tundra::rhi

namespace tundra::vulkan_rhi {

class VulkanAllocator;
class VulkanInstance;
class VulkanRawDevice;
class VulkanDevice;

struct DeviceLimits {
    f32 max_sampler_anisotropy = 0.f;
};

struct VulkanQueues {
    core::Tuple<VkQueue, u32> graphics_queue;
    core::Tuple<VkQueue, u32> compute_queue;
    core::Tuple<VkQueue, u32> transfer_queue;
    core::Tuple<VkQueue, u32> present_queue;
};

struct DeviceProperties {
    u8 pipeline_cache_uuid[VK_UUID_SIZE];
    u32 api_version;
    u32 driver_version;
    u32 vendor_id;
    u32 device_id;
};

struct Extensions {
    loader::khr::Swapchain swapchain;
    loader::khr::Surface surface;

    Extensions(const loader::Instance& instance, const loader::Device& device) noexcept;
};

class VulkanRawDevice : public core::EnableSharedFromThis<VulkanRawDevice> {
private:
    core::SharedPtr<VulkanInstance> m_instance;
    loader::Device m_device;
    VkPhysicalDevice m_physical_device;
    VulkanQueues m_queues;
    DeviceProperties m_device_properties;
    DeviceLimits m_device_limits;
    Extensions m_extensions;

public:
    VulkanRawDevice(
        core::SharedPtr<VulkanInstance> instance,
        loader::Device device,
        const VkPhysicalDevice physical_device,
        VulkanQueues queues,
        DeviceProperties device_properties,
        DeviceLimits device_limits) noexcept;
    ~VulkanRawDevice() noexcept;

public:
    void wait_until_idle() const noexcept;
    void queue_submit(
        const rhi::QueueType queue_type,
        const core::Span<const VkSubmitInfo>& submit_infos,
        const VkFence fence) const noexcept;
    void queue_present(
        const rhi::QueueType queue_type, const VkPresentInfoKHR& present_info) noexcept;

public:
    [[nodiscard]] core::SharedPtr<VulkanInstance> get_instance() const noexcept;
    [[nodiscard]] const loader::Device& get_device() const noexcept;
    [[nodiscard]] VkPhysicalDevice get_physical_device() const noexcept;
    [[nodiscard]] const Extensions& get_extensions() const noexcept;
    [[nodiscard]] const VulkanQueues& get_queues() const noexcept;
    [[nodiscard]] const DeviceProperties& get_device_properties() const noexcept;
    [[nodiscard]] const DeviceLimits& get_device_limits() const noexcept;

private:
    friend class VulkanDevice;
};

///
class VulkanDevice : public core::EnableSharedFromThis<VulkanDevice> {
private:
    core::SharedPtr<VulkanInstance> m_instance;
    core::SharedPtr<VulkanRawDevice> m_raw_device;
    core::SharedPtr<VulkanAllocator> m_allocator;
    Managers m_managers;
    VulkanSubmitWorkScheduler m_submit_work_scheduler;

public:
    VulkanDevice(
        core::SharedPtr<VulkanInstance> instance,
        loader::Device device,
        const VkPhysicalDevice physical_device,
        VulkanQueues queues,
        DeviceProperties device_properties,
        DeviceLimits device_limits) noexcept;

    ~VulkanDevice() noexcept;

public:
    void wait_until_idle() const noexcept;

private:
    void gc() noexcept;

public:
    void submit(
        core::Array<rhi::SubmitInfo> submit_infos,
        core::Array<rhi::PresentInfo> present_infos) noexcept;

    [[nodiscard]] rhi::SwapchainHandle create_swapchain(
        const rhi::SwapchainCreateInfo& create_info) noexcept;
    void destroy_swapchain(const rhi::SwapchainHandle handle) noexcept;

    [[nodiscard]] rhi::BufferHandle create_buffer(
        const rhi::BufferCreateInfo& create_info) noexcept;
    void update_buffer(
        const rhi::BufferHandle handle,
        const core::Array<rhi::BufferUpdateRegion>& update_regions) noexcept;
    void destroy_buffer(const rhi::BufferHandle handle) noexcept;

    [[nodiscard]] rhi::TextureHandle create_texture(
        const rhi::TextureCreateInfo& create_info) noexcept;
    void destroy_texture(const rhi::TextureHandle handle) noexcept;

    [[nodiscard]] rhi::TextureViewHandle create_texture_view(
        const rhi::TextureViewCreateInfo& create_info) noexcept;
    void destroy_texture_view(const rhi::TextureViewHandle handle) noexcept;

    [[nodiscard]] rhi::ShaderHandle create_shader(
        const rhi::ShaderCreateInfo& create_info) noexcept;
    void destroy_shader(const rhi::ShaderHandle handle) noexcept;

    [[nodiscard]] rhi::GraphicsPipelineHandle create_graphics_pipeline(
        const rhi::GraphicsPipelineCreateInfo& create_info) noexcept;
    void destroy_graphics_pipeline(const rhi::GraphicsPipelineHandle handle) noexcept;

    [[nodiscard]] rhi::ComputePipelineHandle create_compute_pipeline(
        const rhi::ComputePipelineCreateInfo& create_info) noexcept;
    void destroy_compute_pipeline(const rhi::ComputePipelineHandle handle) noexcept;

    [[nodiscard]] rhi::SamplerHandle create_sampler(
        const rhi::SamplerCreateInfo& create_info) noexcept;
    void destroy_sampler(const rhi::SamplerHandle handle) noexcept;

public:
    [[nodiscard]] core::SharedPtr<VulkanInstance> get_instance() const noexcept;
    [[nodiscard]] const loader::Device& get_device() const noexcept;
    [[nodiscard]] VkPhysicalDevice get_physical_device() const noexcept;
    [[nodiscard]] Managers& get_managers() noexcept;
    [[nodiscard]] const VulkanQueues& get_queues() const noexcept;
    [[nodiscard]] const DeviceProperties& get_device_properties() const noexcept;
    [[nodiscard]] const DeviceLimits& get_device_limits() const noexcept;
};

} // namespace tundra::vulkan_rhi
