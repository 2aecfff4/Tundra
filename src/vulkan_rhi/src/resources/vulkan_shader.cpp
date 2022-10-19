#include "resources/vulkan_shader.h"
#include "core/profiler.h"
#include "core/std/assert.h"
#include "vulkan_device.h"
#include "vulkan_helpers.h"
#include "vulkan_instance.h"

namespace tundra::vulkan_rhi {

VulkanShader::VulkanShader(
    core::SharedPtr<VulkanRawDevice> raw_device,
    const rhi::ShaderCreateInfo& create_info) noexcept
    : m_raw_device(core::move(raw_device))
    , m_shader_stage(create_info.shader_stage)
{
    TNDR_PROFILER_TRACE("VulkanShader::VulkanShader");

    tndr_assert(
        (create_info.shader_buffer.size() % 4) == 0,
        "`create_info.buffer.len()` must be a multiple of 4!");

    // #TODO: Validate if a shader has correct descriptor layout.
    // - Set 0 - Buffers
    // - Set 1 - Textures
    // - Set 2 - RWTextures
    // - Set 3 - Samplers
    // - Set 4 - Probably stuff related to ray tracing.
    // - Push constant(8 bytes)

    const VkShaderModuleCreateInfo shader_create_info {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = static_cast<u32>(create_info.shader_buffer.size()),
        .pCode = reinterpret_cast<const u32*>(create_info.shader_buffer.data()),
    };

    m_shader_module = vulkan_map_result(
        m_raw_device->get_device().create_shader_module(shader_create_info, nullptr),
        "`create_shader_module` failed");

    if (!create_info.name.empty()) {
        helpers::set_object_name(
            m_raw_device,
            reinterpret_cast<u64>(m_shader_module),
            VK_OBJECT_TYPE_SHADER_MODULE,
            create_info.name.c_str());
    }
}

VulkanShader::~VulkanShader() noexcept
{
    TNDR_PROFILER_TRACE("VulkanShader::~VulkanShader");

    if (m_shader_module != VK_NULL_HANDLE) {
        m_raw_device->get_device().destroy_shader_module(m_shader_module, nullptr);
        m_shader_module = VK_NULL_HANDLE;
    }
}

VulkanShader::VulkanShader(VulkanShader&& rhs) noexcept
    : m_raw_device(core::move(rhs.m_raw_device))
    , m_shader_stage(rhs.m_shader_stage)
    , m_shader_module(core::exchange(rhs.m_shader_module, VK_NULL_HANDLE))
{
}

VulkanShader& VulkanShader::operator=(VulkanShader&& rhs) noexcept
{
    if (this != &rhs) {
        m_raw_device = core::move(rhs.m_raw_device);
        m_shader_stage = rhs.m_shader_stage;
        m_shader_module = core::exchange(rhs.m_shader_module, VK_NULL_HANDLE);
    }
    return *this;
}

VkShaderModule VulkanShader::get_shader_module() const noexcept
{
    return m_shader_module;
}

tundra::rhi::ShaderStage VulkanShader::get_shader_stage() const noexcept
{
    return m_shader_stage;
}

} // namespace tundra::vulkan_rhi
