#pragma once
#include "core/core.h"
#include "core/std/shared_ptr.h"
#include "rhi/resources/shader.h"
#include "vulkan_utils.h"

namespace tundra::vulkan_rhi {

class VulkanRawDevice;

///
class VulkanShader {
private:
    core::SharedPtr<VulkanRawDevice> m_raw_device;
    rhi::ShaderStage m_shader_stage;
    VkShaderModule m_shader_module;

public:
    VulkanShader(
        core::SharedPtr<VulkanRawDevice> raw_device,
        const rhi::ShaderCreateInfo& create_info) noexcept;
    ~VulkanShader() noexcept;

    VulkanShader(VulkanShader&& rhs) noexcept;
    VulkanShader& operator=(VulkanShader&& rhs) noexcept;
    VulkanShader(const VulkanShader&) noexcept = delete;
    VulkanShader& operator=(const VulkanShader&) noexcept = delete;

public:
    [[nodiscard]] VkShaderModule get_shader_module() const noexcept;
    [[nodiscard]] rhi::ShaderStage get_shader_stage() const noexcept;
};

} // namespace tundra::vulkan_rhi
