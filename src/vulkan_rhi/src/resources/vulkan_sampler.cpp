#include "resources/vulkan_sampler.h"
#include "core/profiler.h"
#include "core/std/panic.h"
#include "vulkan_device.h"
#include "vulkan_helpers.h"
#include <algorithm>

namespace tundra::vulkan_rhi {

VulkanSampler::VulkanSampler(
    core::SharedPtr<VulkanRawDevice> raw_device,
    const rhi::SamplerCreateInfo& create_info) noexcept
    : m_raw_device(core::move(raw_device))
{
    TNDR_PROFILER_TRACE("VulkanSampler::VulkanSampler");

    const f32 anisotropy_level = std::min(
        create_info.anisotropy_level,
        m_raw_device->get_device_limits().max_sampler_anisotropy);

    const VkSamplerMipmapMode mipmap_mode = [&] {
        switch (create_info.min_filter) {
            case rhi::SamplerFilter::Nearest:
                return VK_SAMPLER_MIPMAP_MODE_NEAREST;
            case rhi::SamplerFilter::Linear:
                return VK_SAMPLER_MIPMAP_MODE_LINEAR;
        }

        core::panic("Invalid enum");
    }();

    const VkSamplerCreateInfo sampler_create_info {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .magFilter = helpers::map_sampler_filer(create_info.mag_filter),
        .minFilter = helpers::map_sampler_filer(create_info.min_filter),
        .mipmapMode = mipmap_mode,
        .addressModeU = helpers::map_sampler_address_mode(create_info.address_mode_u),
        .addressModeV = helpers::map_sampler_address_mode(create_info.address_mode_v),
        .addressModeW = helpers::map_sampler_address_mode(create_info.address_mode_w),
        .mipLodBias = create_info.mip_bias,
        .anisotropyEnable = anisotropy_level > 1.f,
        .maxAnisotropy = anisotropy_level,
        .compareEnable = create_info.sampler_compare_op != rhi::CompareOp::Never,
        .compareOp = helpers::map_compare_op(create_info.sampler_compare_op),
        .minLod = create_info.min_mip_level,
        .maxLod = create_info.max_mip_level,
        .borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,
        .unnormalizedCoordinates = false,
    };

    m_sampler = vulkan_map_result(
        m_raw_device->get_device().create_sampler(sampler_create_info, nullptr),
        "`create_sampler` failed");

    if (!create_info.name.empty()) {
        helpers::set_object_name(
            m_raw_device,
            reinterpret_cast<u64>(m_sampler),
            VK_OBJECT_TYPE_SAMPLER,
            create_info.name.c_str());
    }
}

VulkanSampler::~VulkanSampler() noexcept
{
    TNDR_PROFILER_TRACE("VulkanSampler::~VulkanSampler");

    if (m_sampler != VK_NULL_HANDLE) {
        m_raw_device->get_device().destroy_sampler(m_sampler, nullptr);
        m_sampler = VK_NULL_HANDLE;
    }
}

VulkanSampler::VulkanSampler(VulkanSampler&& rhs) noexcept
    : m_raw_device(core::move(rhs.m_raw_device))
    , m_sampler(core::exchange(rhs.m_sampler, VK_NULL_HANDLE))
{
}

VulkanSampler& VulkanSampler::operator=(VulkanSampler&& rhs) noexcept
{
    if (&rhs != this) {
        m_raw_device = core::move(rhs.m_raw_device);
        m_sampler = core::exchange(rhs.m_sampler, VK_NULL_HANDLE);
    }

    return *this;
}

VkSampler VulkanSampler::get_sampler() const noexcept
{
    return m_sampler;
}

} // namespace tundra::vulkan_rhi
