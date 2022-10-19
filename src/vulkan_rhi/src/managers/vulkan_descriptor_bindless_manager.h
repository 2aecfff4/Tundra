#pragma once
#include "core/core.h"
#include "core/std/containers/array.h"
#include "core/std/shared_ptr.h"
#include "core/std/sync/lock.h"
#include "rhi/resources/handle.h"
#include "vulkan_config.h"
#include "vulkan_utils.h"

namespace tundra::vulkan_rhi {

class VulkanRawDevice;
class VulkanBuffer;
class VulkanTexture;
class VulkanTextureView;
class VulkanSampler;
class VulkanPipelineLayoutManager;

class VulkanDescriptorBindlessManager {
private:
    struct SetData {
        core::Array<u32> free_indices;
        u32 first_free = 0;
    };

private:
    core::SharedPtr<VulkanRawDevice> m_raw_device;
    core::Lock<SetData> m_sets_data[NUM_BINDINGS];
    VkPipelineLayout m_pipeline_layout;
    VkDescriptorPool m_descriptor_pool;
    VkDescriptorSet m_descriptor_sets[NUM_BINDINGS];

public:
    VulkanDescriptorBindlessManager(
        core::SharedPtr<VulkanRawDevice> device,
        const core::SharedPtr<VulkanPipelineLayoutManager>&
            pipeline_layout_manager) noexcept;
    ~VulkanDescriptorBindlessManager() noexcept;

public:
    [[nodiscard]] rhi::BindableResource bind_buffer(const VulkanBuffer& buffer) noexcept;
    void unbind_buffer(const rhi::BindableResource& resource) noexcept;

    [[nodiscard]] rhi::BindableResource bind_texture(
        const VulkanTexture& texture) noexcept;
    void unbind_texture(const rhi::BindableResource& resource) noexcept;

    [[nodiscard]] rhi::BindableResource bind_texture_view(
        const VulkanTextureView& texture_view) noexcept;
    void unbind_texture_view(const rhi::BindableResource& resource) noexcept;

    [[nodiscard]] rhi::BindableResource bind_sampler(
        const VulkanSampler& sampler) noexcept;
    void unbind_sampler(const rhi::BindableResource& resource) noexcept;

    void bind_descriptors(
        const VkCommandBuffer command_buffer,
        const VkPipelineBindPoint bind_point) noexcept;

private:
    void push_to_free_list(const u32 descriptor_type, const u32 descriptor_index) noexcept;
    [[nodiscard]] u32 get_descriptor_index(const u32 descriptor_type) noexcept;
};

} // namespace tundra::vulkan_rhi
