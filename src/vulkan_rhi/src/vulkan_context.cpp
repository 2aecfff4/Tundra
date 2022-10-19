#include "vulkan_context.h"
#include "core/logger.h"
#include "core/profiler.h"
#include "core/std/assert.h"
#include "loader/extensions/khr/swapchain.h"
#include "vulkan_device.h"
#include "vulkan_helpers.h"
#include "vulkan_instance.h"
#include <algorithm>

namespace tundra::vulkan_rhi {

VulkanContext::VulkanContext() noexcept
    : m_entry()
    , m_instance(core::make_shared<VulkanInstance>(m_entry))
{
    TNDR_PROFILER_TRACE("VulkanContext::VulkanContext");

    this->create_device();
}

void VulkanContext::create_device() noexcept
{
    TNDR_PROFILER_TRACE("VulkanContext::create_device");

    const core::Array<VulkanContext::Device> devices = this->enumerate_devices();

    // Get first device
    const VulkanContext::Device& device = devices.front();

    const f32 graphics_queue_priority = 1.f;
    const f32 compute_queue_priority = 0.5f;
    const f32 transfer_queue_priority = 0.2f;
    const f32 queue_priorities[] = {
        graphics_queue_priority,
        compute_queue_priority,
        transfer_queue_priority,
    };

    core::Array<VkDeviceQueueCreateInfo> queue_create_infos;

    // Graphics queue
    queue_create_infos.push_back(VkDeviceQueueCreateInfo {
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = device.queues.graphics,
        .queueCount = 1,
        .pQueuePriorities = &queue_priorities[0],
    });

    // Compute queue
    if (device.queues.compute != device.queues.graphics) {
        queue_create_infos.push_back(VkDeviceQueueCreateInfo {
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = device.queues.compute,
            .queueCount = 1,
            .pQueuePriorities = &queue_priorities[1],
        });
    }

    // Transfer queue
    if ((device.queues.transfer != device.queues.graphics) &&
        (device.queues.transfer != device.queues.compute)) {
        queue_create_infos.push_back(VkDeviceQueueCreateInfo {
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = device.queues.transfer,
            .queueCount = 1,
            .pQueuePriorities = &queue_priorities[2],
        });
    }

    VkPhysicalDeviceFeatures2 physical_device_features2 {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
        .features =
            VkPhysicalDeviceFeatures {
                .imageCubeArray = true,
                .geometryShader =
                    true, // Geometry shaders are required for `gl_PrimitiveID`.
                .multiDrawIndirect = true,
                .depthBounds = true,
                .samplerAnisotropy = true,
                .vertexPipelineStoresAndAtomics = true,
                .fragmentStoresAndAtomics = true,
                .shaderInt64 = true,
            },
    };

    VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT
        physical_device_shader_image_atomic_int64_features {
            .sType =
                VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_IMAGE_ATOMIC_INT64_FEATURES_EXT,
            .shaderImageInt64Atomics = true,
        };

    VkPhysicalDeviceVulkan12Features physical_device_vulkan12_features {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
        .drawIndirectCount = true,
        .shaderSampledImageArrayNonUniformIndexing = true,
        .shaderStorageBufferArrayNonUniformIndexing = true,
        .shaderStorageImageArrayNonUniformIndexing = true,
        .descriptorBindingSampledImageUpdateAfterBind = true,
        .descriptorBindingStorageImageUpdateAfterBind = true,
        .descriptorBindingStorageBufferUpdateAfterBind = true,
        .descriptorBindingPartiallyBound = true,
        .runtimeDescriptorArray = true,
        .timelineSemaphore = true,
    };

    VkPhysicalDeviceVulkan13Features physical_device_vulkan13_features {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
        .dynamicRendering = true,
        .maintenance4 = true,
    };

    core::Array<const char*> device_extensions_names {
        loader::khr::Swapchain::name(),
        VK_EXT_SHADER_IMAGE_ATOMIC_INT64_EXTENSION_NAME,
    };

    VkDeviceCreateInfo device_create_info {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .queueCreateInfoCount = static_cast<u32>(queue_create_infos.size()),
        .pQueueCreateInfos = queue_create_infos.data(),
        .enabledExtensionCount = static_cast<u32>(device_extensions_names.size()),
        .ppEnabledExtensionNames = device_extensions_names.data(),
    };

    helpers::chain_structs(
        device_create_info,
        physical_device_features2,
        physical_device_shader_image_atomic_int64_features,
        physical_device_vulkan12_features,
        physical_device_vulkan13_features);

    const loader::Device loader_device = vulkan_map_result(
        m_instance->get_instance().create_device(
            device.physical_device, device_create_info, nullptr),
        "`create_device` failed");

    const VulkanQueues queues {
        .graphics_queue = core::make_tuple(
            loader_device.get_device_queue(device.queues.graphics, 0),
            device.queues.graphics),
        .compute_queue = core::make_tuple(
            loader_device.get_device_queue(device.queues.compute, 0),
            device.queues.compute),
        .transfer_queue = core::make_tuple(
            loader_device.get_device_queue(device.queues.transfer, 0),
            device.queues.transfer),
        // #TODO: For now just always use graphics queue.
        .present_queue = core::make_tuple(
            loader_device.get_device_queue(device.queues.graphics, 0),
            device.queues.graphics),
    };

    VkPhysicalDeviceProperties2 physical_device_properties {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
    };

    m_instance->get_instance().get_physical_device_properties2(
        device.physical_device, physical_device_properties);

    DeviceProperties device_properties {
        .api_version = physical_device_properties.properties.apiVersion,
        .driver_version = physical_device_properties.properties.driverVersion,
        .vendor_id = physical_device_properties.properties.vendorID,
        .device_id = physical_device_properties.properties.deviceID,
    };
    std::memcpy(
        device_properties.pipeline_cache_uuid,
        physical_device_properties.properties.pipelineCacheUUID,
        std::size(device_properties.pipeline_cache_uuid));

    const DeviceLimits device_limits {
        .max_sampler_anisotropy = physical_device_properties.properties.limits
                                      .maxSamplerAnisotropy,
    };

    m_devices.push_back(std::make_shared<VulkanDevice>(
        m_instance,
        loader_device,
        device.physical_device,
        queues,
        device_properties,
        device_limits));
}

core::Array<VulkanContext::Device> VulkanContext::enumerate_devices() noexcept
{
    TNDR_PROFILER_TRACE("VulkanContext::enumerate_devices");

    const auto physical_devices = vulkan_map_result(
        m_instance->get_instance().enumerate_physical_devices(),
        "`enumerate_physical_devices` failed");
    tndr_assert(!physical_devices.empty(), "");

    core::Array<VulkanContext::Device> devices;

    for (const VkPhysicalDevice physical_device : physical_devices) {
        VkPhysicalDeviceDriverProperties device_driver_properties {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES,
        };

        VkPhysicalDeviceTimelineSemaphoreProperties timeline_semaphore_properties {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_PROPERTIES,
        };

        VkPhysicalDeviceProperties2 physical_device_properties {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
        };

        helpers::chain_structs(
            physical_device_properties,
            timeline_semaphore_properties,
            device_driver_properties);

        m_instance->get_instance().get_physical_device_properties2(
            physical_device, physical_device_properties);

        // Check if device does support Vulkan.
        if (VK_VERSION_MAJOR(physical_device_properties.properties.apiVersion) < 1) {
            tndr_info(
                "Device {} does not support Vulkan API.",
                physical_device_properties.properties.deviceName);
            continue;
        }

        if ((physical_device_properties.properties.deviceType !=
             VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) &&
            (physical_device_properties.properties.deviceType !=
             VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)) {
            continue;
        }

        // Check if device has necessary extensions.
        const core::Array<VkExtensionProperties> device_extensions = vulkan_map_result(
            m_instance->get_instance().enumerate_device_extension_properties(
                physical_device),
            "`enumerate_device_extension_properties` failed");

        if (device_extensions.empty()) {
            tndr_info(
                "Device {} does not have any device extensions.",
                physical_device_properties.properties.deviceName);
            continue;
        }

        const bool supports_swapchain = std::any_of(
            device_extensions.begin(),
            device_extensions.end(),
            [](const VkExtensionProperties& extension) {
                return std::strcmp(
                           extension.extensionName, loader::khr::Swapchain::name()) == 0;
            });

        if (!supports_swapchain) {
            tndr_info(
                "Device {} does not support {} extension.",
                physical_device_properties.properties.deviceName,
                loader::khr::Swapchain::name());
            continue;
        }

        // Check if the device supports all necessary features.
        {
            VkPhysicalDeviceDescriptorIndexingFeatures descriptor_indexing_features {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES,
            };

            VkPhysicalDeviceVulkan13Features physical_device_vulkan13_features {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
            };

            VkPhysicalDeviceFeatures2 physical_device_features {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
            };

            VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT
                physical_device_shader_image_atomic_int64_features {
                    .sType =
                        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_IMAGE_ATOMIC_INT64_FEATURES_EXT,
                };

            helpers::chain_structs(
                physical_device_features,
                physical_device_shader_image_atomic_int64_features,
                descriptor_indexing_features,
                physical_device_vulkan13_features);

            m_instance->get_instance().get_physical_device_features2(
                physical_device, physical_device_features);

            if ((descriptor_indexing_features.runtimeDescriptorArray == false) ||
                (descriptor_indexing_features.descriptorBindingPartiallyBound == false) ||
                (descriptor_indexing_features.shaderSampledImageArrayNonUniformIndexing ==
                 false) ||
                (descriptor_indexing_features.shaderStorageBufferArrayNonUniformIndexing ==
                 false) ||
                (descriptor_indexing_features.shaderStorageImageArrayNonUniformIndexing ==
                 false) ||
                (descriptor_indexing_features
                     .descriptorBindingStorageBufferUpdateAfterBind == false) ||
                (descriptor_indexing_features
                     .descriptorBindingSampledImageUpdateAfterBind == false) ||
                (descriptor_indexing_features
                     .descriptorBindingStorageImageUpdateAfterBind == false) ||
                (physical_device_vulkan13_features.dynamicRendering == false) ||
                (physical_device_shader_image_atomic_int64_features
                     .shaderImageInt64Atomics == false)) {
                tndr_info(
                    "Device {} does not support necessary features.",
                    physical_device_properties.properties.deviceName);
                continue;
            }
        }

        // Check if there are all necessary queues.
        const core::Array<VkQueueFamilyProperties> queue_families =
            m_instance->get_instance().get_physical_device_queue_family_properties(
                physical_device);

        const auto get_queue_family_index =
            [&](const VkQueueFlags queue_family_flags) -> core::Option<u32> {
            if ((queue_family_flags & VK_QUEUE_COMPUTE_BIT) == VK_QUEUE_COMPUTE_BIT) {
                // Dedicated queue for compute.
                // Try to find a queue family index that supports compute but not graphics.
                u32 index = 0;
                for (const VkQueueFamilyProperties& properties : queue_families) {
                    if (((properties.queueFlags & queue_family_flags) ==
                         queue_family_flags) &&
                        ((properties.queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0)) {
                        return index;
                    }
                    index += 1;
                }
            } else if ((queue_family_flags & VK_QUEUE_TRANSFER_BIT) == VK_QUEUE_TRANSFER_BIT) {
                // Dedicated queue for transfer.
                // Try to find a queue family index that supports transfer,
                // but not graphics and compute.
                u32 index = 0;
                for (const VkQueueFamilyProperties& properties : queue_families) {
                    if (((properties.queueFlags & queue_family_flags) ==
                         queue_family_flags) &&
                        ((properties.queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0) &&
                        ((properties.queueFlags & VK_QUEUE_COMPUTE_BIT) == 0)) {
                        return index;
                    }
                    index += 1;
                }
            }

            // For other queue types or if no separate compute queue is present,
            // return the first one to support the requested flags.
            u32 index = 0;
            for (const VkQueueFamilyProperties& properties : queue_families) {
                if ((properties.queueFlags & queue_family_flags) == queue_family_flags) {
                    return index;
                }
                index += 1;
            }

            return std::nullopt;
        };

        const core::Option<u32> graphics_queue = get_queue_family_index(
            VK_QUEUE_GRAPHICS_BIT);
        const core::Option<u32> compute_queue = get_queue_family_index(
            VK_QUEUE_COMPUTE_BIT);
        const core::Option<u32> transfer_queue = get_queue_family_index(
            VK_QUEUE_TRANSFER_BIT);

        if (!graphics_queue || !compute_queue || !transfer_queue) {
            tndr_info(
                "Device {} does not support necessary queues.",
                physical_device_properties.properties.deviceName);
            continue;
        }

        devices.push_back(VulkanContext::Device {
            .physical_device = physical_device,
            .device_type = physical_device_properties.properties.deviceType,
            .queues =
                VulkanContext::Device::Queues {
                    .graphics = *graphics_queue,
                    .compute = *compute_queue,
                    .transfer = *transfer_queue,
                },
        });
    }

    std::sort(
        devices.begin(),
        devices.end(),
        [&](const VulkanContext::Device& a, const VulkanContext::Device& b) {
            const u32 type_a = a.device_type == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU ? 0
                                                                                     : 1;
            const u32 type_b = b.device_type == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU ? 0
                                                                                     : 1;
            return type_a < type_b;
        });

    return devices;
}

const core::SharedPtr<VulkanDevice>& VulkanContext::get_device() const noexcept
{
    return m_devices[0];
}

} // namespace tundra::vulkan_rhi
