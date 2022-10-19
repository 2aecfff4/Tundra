#include "resources/vulkan_compute_pipeline.h"
#include "core/profiler.h"
#include "managers/managers.h"
#include "managers/vulkan_pipeline_cache_manager.h"
#include "managers/vulkan_pipeline_layout_manager.h"
#include "vulkan_device.h"
#include "vulkan_helpers.h"
#include "vulkan_shader.h"

namespace tundra::vulkan_rhi {

VulkanComputePipeline::VulkanComputePipeline(
    core::SharedPtr<VulkanRawDevice> raw_device,
    const Managers& managers,
    const rhi::ComputePipelineCreateInfo& create_info) noexcept
    : m_raw_device(raw_device)
{
    TNDR_PROFILER_TRACE("VulkanComputePipeline::VulkanComputePipeline");

    constexpr const char* shader_name = "main";

    const VkPipelineShaderStageCreateInfo shader_stage_create_info =
        managers.shader_manager
            ->with(
                create_info.compute_shader.get_handle(),
                [](const VulkanShader& shader) {
                    return VkPipelineShaderStageCreateInfo {
                        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                        .stage = helpers::map_shader_stage(shader.get_shader_stage()),
                        .module = shader.get_shader_module(),
                        .pName = shader_name,
                    };
                })
            .value_or_else([](const auto&) -> VkPipelineShaderStageCreateInfo {
                core::panic("`ComputePipelineCreateInfo::compute_shader` is not alive.");
            });

    const VkComputePipelineCreateInfo pipeline_create_info {
        .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
        .stage = shader_stage_create_info,
        .layout = managers.pipeline_layout_manager->get_pipeline_layout().pipeline_layout,
    };

    m_pipeline = vulkan_map_result(
        m_raw_device->get_device().create_compute_pipelines(
            managers.pipeline_cache_manager->get_pipeline_cache(),
            core::as_span(core::as_const(pipeline_create_info)),
            nullptr),
        "`create_compute_pipelines` failed")[0];

    if (!create_info.name.empty()) {
        helpers::set_object_name(
            m_raw_device,
            reinterpret_cast<u64>(m_pipeline),
            VK_OBJECT_TYPE_PIPELINE,
            create_info.name.c_str());
    }
}

VulkanComputePipeline::~VulkanComputePipeline() noexcept
{
    TNDR_PROFILER_TRACE("VulkanComputePipeline::VulkanComputePipeline");

    if (m_pipeline != VK_NULL_HANDLE) {
        m_raw_device->get_device().destroy_pipeline(m_pipeline, nullptr);
        m_pipeline = VK_NULL_HANDLE;
    }
}

VulkanComputePipeline::VulkanComputePipeline(VulkanComputePipeline&& rhs) noexcept
    : m_raw_device(core::move(rhs.m_raw_device))
    , m_pipeline(core::exchange(rhs.m_pipeline, VK_NULL_HANDLE))
{
}

VulkanComputePipeline& VulkanComputePipeline::operator=(
    VulkanComputePipeline&& rhs) noexcept
{
    if (&rhs != this) {
        m_raw_device = core::move(rhs.m_raw_device);
        m_pipeline = core::exchange(rhs.m_pipeline, VK_NULL_HANDLE);
    }

    return *this;
}

VkPipeline VulkanComputePipeline::get_pipeline() const noexcept
{
    return m_pipeline;
}

} // namespace tundra::vulkan_rhi
