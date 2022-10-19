#include "managers/vulkan_descriptor_bindless_manager.h"
#include "core/std/assert.h"
#include "managers/vulkan_pipeline_layout_manager.h"
#include "resources/vulkan_buffer.h"
#include "resources/vulkan_sampler.h"
#include "resources/vulkan_texture.h"
#include "resources/vulkan_texture_view.h"
#include "vulkan_device.h"
#include "vulkan_helpers.h"
#include <algorithm>

namespace tundra::vulkan_rhi {

VulkanDescriptorBindlessManager::VulkanDescriptorBindlessManager(
    core::SharedPtr<VulkanRawDevice> device,
    const core::SharedPtr<VulkanPipelineLayoutManager>& pipeline_layout_manager) noexcept
    : m_raw_device(device)
{
    const VkDescriptorPoolSize descriptor_pool_sizes[] = {
        // Buffers
        VkDescriptorPoolSize {
            .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .descriptorCount = MAX_DESCRIPTOR_COUNT,
        },
        // Textures
        VkDescriptorPoolSize {
            .type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            .descriptorCount = MAX_DESCRIPTOR_COUNT,
        },
        // RWTextures
        VkDescriptorPoolSize {
            .type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
            .descriptorCount = MAX_DESCRIPTOR_COUNT,
        },
        // Samplers
        VkDescriptorPoolSize {
            .type = VK_DESCRIPTOR_TYPE_SAMPLER,
            .descriptorCount = MAX_DESCRIPTOR_COUNT,
        },
    };

    const VkDescriptorPoolCreateInfo descriptor_pool_create_info {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
        .maxSets = 4,
        .poolSizeCount = static_cast<u32>(std::size(descriptor_pool_sizes)),
        .pPoolSizes = descriptor_pool_sizes,
    };

    m_descriptor_pool = vulkan_map_result(
        device->get_device().create_descriptor_pool(descriptor_pool_create_info, nullptr),
        "`create_descriptor_pool` failed");

    const VkDescriptorSetAllocateInfo descriptor_set_allocate_info {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = m_descriptor_pool,
        .descriptorSetCount = NUM_BINDINGS,
        .pSetLayouts = pipeline_layout_manager->get_pipeline_layout().descriptor_layouts,
    };

    const core::Array<VkDescriptorSet> descriptor_sets = vulkan_map_result(
        device->get_device().allocate_descriptor_sets(descriptor_set_allocate_info),
        "`allocate_descriptor_sets` failed");

    std::copy(
        std::begin(descriptor_sets),
        std::end(descriptor_sets),
        std::begin(m_descriptor_sets));

    static constexpr const char* names[] {
        "DescriptorSet: Buffers",
        "DescriptorSet: Textures",
        "DescriptorSet: RWTextures",
        "DescriptorSet: Samplers",
    };
    for (usize i = 0; i < 4; ++i) {
        helpers::set_object_name(
            m_raw_device,
            reinterpret_cast<u64>(m_descriptor_sets[i]),
            VK_OBJECT_TYPE_DESCRIPTOR_SET,
            names[i]);
    }

    m_pipeline_layout = pipeline_layout_manager->get_pipeline_layout().pipeline_layout;
}

VulkanDescriptorBindlessManager::~VulkanDescriptorBindlessManager() noexcept
{
    m_raw_device->get_device().destroy_descriptor_pool(m_descriptor_pool, nullptr);
}

rhi::BindableResource VulkanDescriptorBindlessManager::bind_buffer(
    const VulkanBuffer& buffer) noexcept
{
    constexpr u32 DESCRIPTOR_TYPE = BINDING_BUFFERS;
    const rhi::BufferUsageFlags usage_flags = buffer.get_usage_flags();

    if (contains(usage_flags, rhi::BufferUsageFlags::SRV) ||
        contains(usage_flags, rhi::BufferUsageFlags::UAV)) {
        const u32 index = this->get_descriptor_index(DESCRIPTOR_TYPE);

        const VkDescriptorBufferInfo buffer_info {
            .buffer = buffer.get_buffer(),
            .offset = 0,
            .range = VK_WHOLE_SIZE,
        };

        const VkWriteDescriptorSet write {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = m_descriptor_sets[DESCRIPTOR_TYPE],
            .dstBinding = 0,
            .dstArrayElement = index,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .pBufferInfo = &buffer_info,
        };

        // #NOTE: Descriptor layout bindings are created with `VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT` flag.
        // https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkDescriptorBindingFlagBits.html#_description
        // "Multiple descriptors with this flag set can be updated concurrently in different threads,
        // though the same descriptor must not be updated concurrently by two threads."
        m_raw_device->get_device().update_descriptor_sets(core::as_span(write), {});

        return rhi::BindableResource {
            .bindless_srv = contains(usage_flags, rhi::BufferUsageFlags::SRV)
                                ? index
                                : rhi::BindableResource::INVALID_INDEX,
            .bindless_uav = contains(usage_flags, rhi::BufferUsageFlags::UAV)
                                ? index
                                : rhi::BindableResource::INVALID_INDEX,
        };
    } else {
        return rhi::BindableResource {};
    }
}

void VulkanDescriptorBindlessManager::unbind_buffer(
    const rhi::BindableResource& resource) noexcept
{
    constexpr u32 DESCRIPTOR_TYPE = BINDING_BUFFERS;
    const u32 index = std::min(resource.bindless_srv, resource.bindless_uav);
    if (index != rhi::BindableResource::INVALID_INDEX) {
        this->push_to_free_list(DESCRIPTOR_TYPE, index);
    }
}

rhi::BindableResource VulkanDescriptorBindlessManager::bind_texture(
    const VulkanTexture& texture) noexcept
{
    VkDescriptorImageInfo image_infos[2];
    VkWriteDescriptorSet writes[2];
    usize write_count = 0;

    const rhi::TextureUsageFlags usage_flags = texture.get_usage();
    const VkImageView image_view = texture.get_image_view();

    u32 srv_index = rhi::BindableResource::INVALID_INDEX;
    if (contains(usage_flags, rhi::TextureUsageFlags::SRV)) {
        constexpr u32 DESCRIPTOR_TYPE = BINDING_TEXTURES;
        const u32 index = this->get_descriptor_index(DESCRIPTOR_TYPE);

        image_infos[write_count] = VkDescriptorImageInfo {
            .imageView = image_view,
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        };

        writes[write_count] = VkWriteDescriptorSet {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = m_descriptor_sets[DESCRIPTOR_TYPE],
            .dstBinding = 0,
            .dstArrayElement = index,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            .pImageInfo = &image_infos[write_count],
        };

        srv_index = index;
        write_count += 1;
    }

    u32 uav_index = rhi::BindableResource::INVALID_INDEX;
    if (contains(usage_flags, rhi::TextureUsageFlags::UAV)) {
        constexpr u32 DESCRIPTOR_TYPE = BINDING_RW_TEXTURES;
        const u32 index = this->get_descriptor_index(DESCRIPTOR_TYPE);

        image_infos[write_count] = VkDescriptorImageInfo {
            .imageView = image_view,
            .imageLayout = VK_IMAGE_LAYOUT_GENERAL,
        };

        writes[write_count] = VkWriteDescriptorSet {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = m_descriptor_sets[DESCRIPTOR_TYPE],
            .dstBinding = 0,
            .dstArrayElement = index,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
            .pImageInfo = &image_infos[write_count],
        };

        uav_index = index;
        write_count += 1;
    }

    m_raw_device->get_device().update_descriptor_sets(
        core::Span(writes, write_count), {});

    return rhi::BindableResource {
        .bindless_srv = srv_index,
        .bindless_uav = uav_index,
    };
}

void VulkanDescriptorBindlessManager::unbind_texture(
    const rhi::BindableResource& resource) noexcept
{
    if (resource.bindless_srv != rhi::BindableResource::INVALID_INDEX) {
        constexpr u32 DESCRIPTOR_TYPE = BINDING_TEXTURES;
        this->push_to_free_list(DESCRIPTOR_TYPE, resource.bindless_srv);
    }

    if (resource.bindless_uav != rhi::BindableResource::INVALID_INDEX) {
        constexpr u32 DESCRIPTOR_TYPE = BINDING_RW_TEXTURES;
        this->push_to_free_list(DESCRIPTOR_TYPE, resource.bindless_uav);
    }
}

rhi::BindableResource VulkanDescriptorBindlessManager::bind_texture_view(
    const VulkanTextureView& texture_view) noexcept
{
    VkDescriptorImageInfo image_infos[2];
    VkWriteDescriptorSet writes[2];
    usize write_count = 0;

    const rhi::TextureUsageFlags usage_flags = texture_view.get_usage();
    const VkImageView image_view = texture_view.get_image_view();

    u32 srv_index = rhi::BindableResource::INVALID_INDEX;
    if (contains(usage_flags, rhi::TextureUsageFlags::SRV)) {
        constexpr u32 DESCRIPTOR_TYPE = BINDING_TEXTURES;
        const u32 index = this->get_descriptor_index(DESCRIPTOR_TYPE);

        image_infos[write_count] = VkDescriptorImageInfo {
            .imageView = image_view,
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        };

        writes[write_count] = VkWriteDescriptorSet {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = m_descriptor_sets[DESCRIPTOR_TYPE],
            .dstBinding = 0,
            .dstArrayElement = index,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            .pImageInfo = &image_infos[write_count],
        };

        srv_index = index;
        write_count += 1;
    }

    u32 uav_index = rhi::BindableResource::INVALID_INDEX;
    if (contains(usage_flags, rhi::TextureUsageFlags::UAV)) {
        constexpr u32 DESCRIPTOR_TYPE = BINDING_RW_TEXTURES;
        const u32 index = this->get_descriptor_index(DESCRIPTOR_TYPE);

        image_infos[write_count] = VkDescriptorImageInfo {
            .imageView = image_view,
            .imageLayout = VK_IMAGE_LAYOUT_GENERAL,
        };

        writes[write_count] = VkWriteDescriptorSet {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = m_descriptor_sets[DESCRIPTOR_TYPE],
            .dstBinding = 0,
            .dstArrayElement = index,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
            .pImageInfo = &image_infos[write_count],
        };

        uav_index = index;
        write_count += 1;
    }

    m_raw_device->get_device().update_descriptor_sets(
        core::Span(writes, write_count), {});

    return rhi::BindableResource {
        .bindless_srv = srv_index,
        .bindless_uav = uav_index,
    };
}

void VulkanDescriptorBindlessManager::unbind_texture_view(
    const rhi::BindableResource& resource) noexcept
{
    if (resource.bindless_srv != rhi::BindableResource::INVALID_INDEX) {
        constexpr u32 DESCRIPTOR_TYPE = BINDING_TEXTURES;
        this->push_to_free_list(DESCRIPTOR_TYPE, resource.bindless_srv);
    }

    if (resource.bindless_uav != rhi::BindableResource::INVALID_INDEX) {
        constexpr u32 DESCRIPTOR_TYPE = BINDING_RW_TEXTURES;
        this->push_to_free_list(DESCRIPTOR_TYPE, resource.bindless_uav);
    }
}

rhi::BindableResource VulkanDescriptorBindlessManager::bind_sampler(
    const VulkanSampler& sampler) noexcept
{
    constexpr u32 DESCRIPTOR_TYPE = BINDING_SAMPLERS;
    const u32 index = this->get_descriptor_index(DESCRIPTOR_TYPE);

    const VkDescriptorImageInfo image_info {
        .sampler = sampler.get_sampler(),
    };

    const VkWriteDescriptorSet write {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = m_descriptor_sets[DESCRIPTOR_TYPE],
        .dstBinding = 0,
        .dstArrayElement = index,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
        .pImageInfo = &image_info,
    };

    m_raw_device->get_device().update_descriptor_sets(core::as_span(write), {});

    return rhi::BindableResource {
        .bindless_srv = index,
        .bindless_uav = index,
    };
}

void VulkanDescriptorBindlessManager::unbind_sampler(
    const rhi::BindableResource& resource) noexcept
{
    tndr_assert(resource.bindless_srv == resource.bindless_uav, "");
    tndr_assert(resource.bindless_srv == rhi::BindableResource::INVALID_INDEX, "");

    constexpr u32 DESCRIPTOR_TYPE = BINDING_SAMPLERS;
    this->push_to_free_list(DESCRIPTOR_TYPE, resource.bindless_srv);
}

void VulkanDescriptorBindlessManager::bind_descriptors(
    const VkCommandBuffer command_buffer, const VkPipelineBindPoint bind_point) noexcept
{
    m_raw_device->get_device().cmd_bind_descriptor_sets(
        command_buffer,
        bind_point,
        m_pipeline_layout,
        0,
        core::as_span(m_descriptor_sets),
        {});
}

void VulkanDescriptorBindlessManager::push_to_free_list(
    const u32 descriptor_type, const u32 descriptor_index) noexcept
{
    auto set_data = m_sets_data[descriptor_type].lock();
    set_data->free_indices.push_back(descriptor_index);
}

u32 VulkanDescriptorBindlessManager::get_descriptor_index(
    const u32 descriptor_type) noexcept
{
    auto set_data = m_sets_data[descriptor_type].lock();
    u32 index = 0;
    if (set_data->free_indices.empty()) {
        index = set_data->first_free;
        set_data->first_free += 1;
    } else {
        index = set_data->free_indices.back();
        set_data->free_indices.pop_back();
    }

    tndr_assert(index != rhi::BindableResource::INVALID_INDEX, "");
    return index;
}

} // namespace tundra::vulkan_rhi
