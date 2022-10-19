#include "managers/vulkan_pipeline_cache_manager.h"
#include "core/logger.h"
#include "core/profiler.h"
#include "core/project_dirs.h"
#include "core/std/containers/array.h"
#include "core/utils/endianness.h"
#include "vulkan_device.h"
#include "vulkan_utils.h"
#include <fstream>

namespace tundra::vulkan_rhi {

[[nodiscard]] static bool is_pipeline_cache_valid(
    const core::Array<char>& buffer, const DeviceProperties& device_properties) noexcept
{
    // https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkPipelineCacheHeaderVersion.html
    // https://www.khronos.org/registry/vulkan/specs/1.2-extensions/html/vkspec.html#pipelines-cache-header

    if (buffer.size() >= sizeof(VkPipelineCacheHeaderVersionOne)) {
        // "Unlike most structures declared by the Vulkan API,
        // all fields of this structure are written with the least significant byte first,
        // regardless of host byte-order."

        if constexpr (core::is_little_endian()) {
            VkPipelineCacheHeaderVersionOne header;
            std::memcpy(&header, buffer.data(), sizeof(VkPipelineCacheHeaderVersionOne));

            const bool is_header_size_correct = header.headerSize == 32;
            const bool is_header_version_correct = header.headerVersion ==
                                                   VK_PIPELINE_CACHE_HEADER_VERSION_ONE;
            const bool is_vendor_id_correct = header.vendorID ==
                                              device_properties.vendor_id;
            const bool is_device_id_correct = header.deviceID ==
                                              device_properties.device_id;
            const bool is_uuid_correct = std::memcmp(
                                             header.pipelineCacheUUID,
                                             device_properties.pipeline_cache_uuid,
                                             std::size(device_properties
                                                           .pipeline_cache_uuid)) == 0;
            return is_header_size_correct && is_header_version_correct &&
                   is_vendor_id_correct && is_device_id_correct && is_uuid_correct;
        } else {
            VkPipelineCacheHeaderVersionOne header;
            std::memcpy(&header, buffer.data(), sizeof(VkPipelineCacheHeaderVersionOne));

            const bool is_header_size_correct = core::endian_swap(header.headerSize) ==
                                                32;
            const bool is_header_version_correct = core::endian_swap(static_cast<i32>(
                                                       header.headerVersion)) ==
                                                   VK_PIPELINE_CACHE_HEADER_VERSION_ONE;
            const bool is_vendor_id_correct = core::endian_swap(header.vendorID) ==
                                              device_properties.vendor_id;
            const bool is_device_id_correct = core::endian_swap(header.deviceID) ==
                                              device_properties.device_id;
            const bool is_uuid_correct = std::memcmp(
                                             header.pipelineCacheUUID,
                                             device_properties.pipeline_cache_uuid,
                                             std::size(device_properties
                                                           .pipeline_cache_uuid)) == 0;
            return is_header_size_correct && is_header_version_correct &&
                   is_vendor_id_correct && is_device_id_correct && is_uuid_correct;
        }
    }
    return false;
}

VulkanPipelineCacheManager::VulkanPipelineCacheManager(
    core::SharedPtr<VulkanRawDevice> device) noexcept
    : m_raw_device(device)
{
    TNDR_PROFILER_TRACE("VulkanPipelineCacheManager::VulkanPipelineCacheManager");

    const core::ProjectDirs proj_dirs("com", "tundra", "tundra");
    const std::filesystem::path& cache_dir = proj_dirs.get_cache_dir();
    const std::filesystem::path pipeline_cache_path = cache_dir / "pipeline_cache";

    if (!std::filesystem::exists(pipeline_cache_path)) {
        std::filesystem::create_directories(pipeline_cache_path);
    }

    const auto read_file =
        [](const std::filesystem::directory_entry& entry) -> core::Array<char> {
        std::ifstream file(entry.path(), std::ios::binary);
        core::Array<char> buffer(entry.file_size());
        file.read(buffer.data(), entry.file_size());
        file.close();
        return buffer;
    };

    core::Array<char> buffer {};
    const DeviceProperties& device_properties = device->get_device_properties();
    const std::filesystem::path file_path = pipeline_cache_path /
                                            fmt::format(
                                                "{}_{}.bin",
                                                device_properties.vendor_id,
                                                device_properties.device_id);

    if (std::filesystem::exists(file_path)) {
        buffer = read_file(std::filesystem::directory_entry(file_path));

        if (is_pipeline_cache_valid(buffer, device_properties)) {
            tndr_info(
                "Pipeline cache: {} has been loaded.", file_path.filename().string());
        } else {
            tndr_info(
                "Pipeline cache: {} has been removed due to incompatibility.",
                file_path.filename().string());
            std::filesystem::remove(file_path);
            buffer.clear();
        }
    }

    const VkPipelineCacheCreateInfo create_info {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
        .initialDataSize = static_cast<u32>(buffer.size()),
        .pInitialData = buffer.data(),
    };

    m_pipeline_cache = vulkan_map_result(
        m_raw_device->get_device().create_pipeline_cache(create_info, nullptr),
        "`create_pipeline_cache` failed");
}

VulkanPipelineCacheManager::~VulkanPipelineCacheManager() noexcept
{
    TNDR_PROFILER_TRACE("VulkanPipelineCacheManager::~VulkanPipelineCacheManager");

    const core::ProjectDirs proj_dirs("com", "tundra", "tundra");
    const std::filesystem::path& cache_dir = proj_dirs.get_cache_dir();
    const std::filesystem::path pipeline_cache_path = cache_dir / "pipeline_cache";

    const DeviceProperties& device_properties = m_raw_device->get_device_properties();
    const std::filesystem::path file_path = pipeline_cache_path /
                                            fmt::format(
                                                "{}_{}.bin",
                                                device_properties.vendor_id,
                                                device_properties.device_id);

    const core::Expected<core::Array<char>, VkResult> result =
        m_raw_device->get_device().get_pipeline_cache_data(m_pipeline_cache);

    if (result) {
        if (!std::filesystem::exists(pipeline_cache_path)) {
            std::filesystem::create_directories(pipeline_cache_path);
        }

        const core::Array<char>& buffer = *result;
        std::ofstream file(file_path, std::ios::binary);
        file.write(buffer.data(), buffer.size());

        tndr_info("Pipeline cache has been saved to: `{}`.", file_path.string());
    } else {
        // For some reason on AMD drivers this works only when at least one pipeline has been created.
        // In any other case this function returns `VK_ERROR_INITIALIZATION_FAILED`.
        // After looking at radv it looks like it's a driver bug.
        // https://gitlab.freedesktop.org/mesa/mesa/-/blob/c314893988d4b7408383d5c0357319082c347fc6/src/amd/vulkan/radv_pipeline_cache.c#L559
        // RX480, drivers: 20.7.2 and 20.11.2
        // Works on Nvidia GTX 970, drivers: 452.06
        tndr_warn(
            "`get_pipeline_cache_data` failed! Error: `{}`",
            vk_result_to_str(result.error()));
    }

    m_raw_device->get_device().destroy_pipeline_cache(m_pipeline_cache, nullptr);
}

VkPipelineCache VulkanPipelineCacheManager::get_pipeline_cache() const noexcept
{
    return m_pipeline_cache;
}

} // namespace tundra::vulkan_rhi
