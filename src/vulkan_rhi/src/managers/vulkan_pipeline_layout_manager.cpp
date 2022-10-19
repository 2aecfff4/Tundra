#include "managers/vulkan_pipeline_layout_manager.h"
#include "core/profiler.h"
#include "vulkan_device.h"
#include "vulkan_helpers.h"

namespace tundra::vulkan_rhi {

VulkanPipelineLayoutManager::VulkanPipelineLayoutManager(
    core::SharedPtr<VulkanRawDevice> device) noexcept
    : m_raw_device(device)
{
    TNDR_PROFILER_TRACE("VulkanPipelineLayoutManager::VulkanPipelineLayoutManager");

    const VkDescriptorSetLayoutBinding descriptor_bindings[] = {
        // Buffers
        VkDescriptorSetLayoutBinding {
            .binding = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .descriptorCount = MAX_DESCRIPTOR_COUNT,
            .stageFlags = VK_SHADER_STAGE_ALL,
        },
        // Textures
        VkDescriptorSetLayoutBinding {
            .binding = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            .descriptorCount = MAX_DESCRIPTOR_COUNT,
            .stageFlags = VK_SHADER_STAGE_ALL,
        },
        // RWTextures
        VkDescriptorSetLayoutBinding {
            .binding = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
            .descriptorCount = MAX_DESCRIPTOR_COUNT,
            .stageFlags = VK_SHADER_STAGE_ALL,
        },
        // Samplers
        VkDescriptorSetLayoutBinding {
            .binding = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
            .descriptorCount = MAX_DESCRIPTOR_COUNT,
            .stageFlags = VK_SHADER_STAGE_ALL,
        },
    };

    static constexpr VkDescriptorBindingFlags FLAGS =
        VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
        VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
    VkDescriptorBindingFlags binding_flags[NUM_BINDINGS];
    std::fill(std::begin(binding_flags), std::end(binding_flags), FLAGS);

    VkDescriptorSetLayout descriptor_layouts[NUM_BINDINGS];
    for (usize i = 0; i < NUM_BINDINGS; ++i) {
        const VkDescriptorSetLayoutBindingFlagsCreateInfo binding_flags_create_info {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
            .bindingCount = 1,
            .pBindingFlags = &binding_flags[i],
        };

        const VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .pNext = &binding_flags_create_info,
            .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
            .bindingCount = 1,
            .pBindings = &descriptor_bindings[i],
        };

        descriptor_layouts[i] = vulkan_map_result(
            device->get_device().create_descriptor_set_layout(
                descriptor_set_layout_create_info, nullptr),
            "`create_descriptor_set_layout` failed");
    }

    const VkPushConstantRange push_constant_range {
        .stageFlags = VK_SHADER_STAGE_ALL,
        .size = 8,
    };

    const VkPipelineLayoutCreateInfo pipeline_layout_create_info {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = NUM_BINDINGS,
        .pSetLayouts = descriptor_layouts,
        .pushConstantRangeCount = 1,
        .pPushConstantRanges = &push_constant_range,
    };

    const VkPipelineLayout pipeline_layout = vulkan_map_result(
        device->get_device().create_pipeline_layout(pipeline_layout_create_info, nullptr),
        "`create_pipeline_layout` failed");

    m_pipeline_layout = PipelineLayout {
        .pipeline_layout = pipeline_layout,
    };

    std::memcpy(
        m_pipeline_layout.descriptor_layouts,
        descriptor_layouts,
        sizeof(descriptor_layouts));
}

VulkanPipelineLayoutManager::~VulkanPipelineLayoutManager() noexcept
{
    TNDR_PROFILER_TRACE("VulkanPipelineLayoutManager::~VulkanPipelineLayoutManager");

    for (const VkDescriptorSetLayout& descriptor_layout :
         m_pipeline_layout.descriptor_layouts) {
        m_raw_device->get_device().destroy_descriptor_set_layout(
            descriptor_layout, nullptr);
    }

    m_raw_device->get_device().destroy_pipeline_layout(
        m_pipeline_layout.pipeline_layout, nullptr);
}

const PipelineLayout& VulkanPipelineLayoutManager::get_pipeline_layout() const noexcept
{
    return m_pipeline_layout;
}

} // namespace tundra::vulkan_rhi
