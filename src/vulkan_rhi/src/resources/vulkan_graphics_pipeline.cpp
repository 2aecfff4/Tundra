#include "resources/vulkan_graphics_pipeline.h"
#include "core/profiler.h"
#include "managers/managers.h"
#include "managers/vulkan_pipeline_cache_manager.h"
#include "managers/vulkan_pipeline_layout_manager.h"
#include "rhi/resources/resource_tracker.h"
#include "vulkan_device.h"
#include "vulkan_helpers.h"
#include "vulkan_shader.h"

namespace tundra::vulkan_rhi {

VulkanGraphicsPipeline::VulkanGraphicsPipeline(
    core::SharedPtr<VulkanRawDevice> raw_device,
    const Managers& managers,
    const rhi::GraphicsPipelineCreateInfo& create_info) noexcept
    : m_raw_device(core::move(raw_device))
{
    TNDR_PROFILER_TRACE("VulkanGraphicsPipeline::VulkanGraphicsPipeline");

    constexpr const char* shader_name = "main";

    core::Array<VkPipelineShaderStageCreateInfo> shader_stage_create_infos;
    shader_stage_create_infos.reserve(
        static_cast<usize>(create_info.vertex_shader.is_valid()) +
        static_cast<usize>(create_info.fragment_shader.is_valid()));

    for (const rhi::ShaderHandle shader :
         { create_info.vertex_shader, create_info.fragment_shader }) {

        const VkPipelineShaderStageCreateInfo shader_stage_create_info =
            managers.shader_manager
                ->with(
                    shader.get_handle(),
                    [](const VulkanShader& shader) {
                        return VkPipelineShaderStageCreateInfo {
                            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                            .stage = helpers::map_shader_stage(shader.get_shader_stage()),
                            .module = shader.get_shader_module(),
                            .pName = shader_name,
                        };
                    })
                .value_or_else([](const auto&) -> VkPipelineShaderStageCreateInfo {
                    core::panic("`GraphicsPipelineCreateInfo::vertex_shader/"
                                "fragment_shader` is not alive.");
                });

        // #TODO: Maybe implement shader cache?
        shader_stage_create_infos.push_back(shader_stage_create_info);
    }

    // We don't use vertex input layouts, but Vulkan specs requires "pVertexInputState" to be a valid pointer.
    const VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
    };

    const VkPipelineInputAssemblyStateCreateInfo input_assembly_state_create_info {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = helpers::map_primitive_type(
            create_info.input_assembly.primitive_type),
    };

    const VkPipelineViewportStateCreateInfo viewport_state_create_info {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .scissorCount = 1,
    };

    const VkPipelineRasterizationStateCreateInfo rasterization_state_create_info {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = create_info.rasterizer_state.depth_clamp,
        .rasterizerDiscardEnable = false, // #TODO: ?
        .polygonMode = helpers::map_polygon_mode(
            create_info.rasterizer_state.polygon_mode),
        // .cullMode VK_DYNAMIC_STATE_CULL_MODE
        .frontFace = helpers::map_front_face(create_info.rasterizer_state.front_face),
        .depthBiasEnable = create_info.rasterizer_state.depth_bias.has_value(),
        .depthBiasConstantFactor = create_info.rasterizer_state.depth_bias.has_value()
                                       ? create_info.rasterizer_state.depth_bias
                                             ->const_factor
                                       : 0.f,
        .depthBiasClamp = create_info.rasterizer_state.depth_bias.has_value()
                              ? create_info.rasterizer_state.depth_bias->clamp
                              : 0.f,
        .depthBiasSlopeFactor = create_info.rasterizer_state.depth_bias.has_value()
                                    ? create_info.rasterizer_state.depth_bias->slope_factor
                                    : 0.f,
        .lineWidth = 1.f,
    };

    const VkPipelineDepthStencilStateCreateInfo depth_stencil_state_create_info {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depthTestEnable = create_info.depth_stencil.depth_test.has_value(),
        .depthWriteEnable = create_info.depth_stencil.depth_test.has_value()
                                ? create_info.depth_stencil.depth_test->write
                                : false,
        .depthCompareOp = create_info.depth_stencil.depth_test.has_value()
                              ? helpers::map_compare_op(
                                    create_info.depth_stencil.depth_test->op)
                              : VK_COMPARE_OP_NEVER,
        .depthBoundsTestEnable = create_info.depth_stencil.depth_bounds,
        .stencilTestEnable = create_info.depth_stencil.stencil_test.has_value(),
        .front = create_info.depth_stencil.stencil_test.has_value()
                     ? helpers::map_stencil_op_desc(
                           create_info.depth_stencil.stencil_test->front)
                     : VkStencilOpState {},
        .back = create_info.depth_stencil.stencil_test.has_value()
                    ? helpers::map_stencil_op_desc(
                          create_info.depth_stencil.stencil_test->back)
                    : VkStencilOpState {},
        .minDepthBounds = 0.f,
        .maxDepthBounds = 1.f,
    };

    core::Array<VkPipelineColorBlendAttachmentState> color_blend_attachments;
    std::transform(
        create_info.color_blend_state.attachments.begin(),
        create_info.color_blend_state.attachments.end(),
        std::back_inserter(color_blend_attachments),
        [&](const rhi::ColorBlendDesc& attachment) {
            if (attachment.blend) {
                const auto [color_blend_op, src_color_blend_factor, dst_color_blend_factor] =
                    helpers::map_blend_op(attachment.blend->color);
                const auto [alpha_blend_op, src_alpha_blend_factor, dst_alpha_blend_factor] =
                    helpers::map_blend_op(attachment.blend->alpha);

                return VkPipelineColorBlendAttachmentState {
                    .blendEnable = true,
                    .srcColorBlendFactor = src_color_blend_factor,
                    .dstColorBlendFactor = dst_color_blend_factor,
                    .colorBlendOp = color_blend_op,
                    .srcAlphaBlendFactor = src_alpha_blend_factor,
                    .dstAlphaBlendFactor = dst_alpha_blend_factor,
                    .alphaBlendOp = alpha_blend_op,
                    .colorWriteMask = helpers::map_color_write_mask(attachment.mask),
                };
            } else {
                return VkPipelineColorBlendAttachmentState {
                    .colorWriteMask = helpers::map_color_write_mask(attachment.mask),
                };
            }
        });

    const VkPipelineColorBlendStateCreateInfo color_blend_state_create_info {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = false,       // #TODO: ?
        .logicOp = VK_LOGIC_OP_CLEAR, // #TODO: ?
        .attachmentCount = static_cast<u32>(color_blend_attachments.size()),
        .pAttachments = color_blend_attachments.data(),
        .blendConstants = { 1.f, 1.f, 1.f, 1.f },
    };

    const VkDynamicState dynamic_states[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
        VK_DYNAMIC_STATE_CULL_MODE,
    };

    const VkPipelineDynamicStateCreateInfo dynamic_state_create_info {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = static_cast<u32>(std::size(dynamic_states)),
        .pDynamicStates = dynamic_states,
    };

    const VkPipelineMultisampleStateCreateInfo multisample_state_create_info = [&] {
        if (create_info.multisampling_state) {
            const auto& [sample_count, sample_shading, alpha_coverage, alpha_to_one] =
                *create_info.multisampling_state;

            return VkPipelineMultisampleStateCreateInfo {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
                .rasterizationSamples = helpers::map_sample_count(sample_count),
                .minSampleShading = sample_shading.value_or(0.0f),
                .alphaToCoverageEnable = alpha_coverage,
                .alphaToOneEnable = alpha_to_one,
            };
        } else {
            return VkPipelineMultisampleStateCreateInfo {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
                .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
            };
        }
    }();

    const core::Array<VkFormat> color_attachment_formats = [&] {
        core::Array<VkFormat> result;
        result.reserve(create_info.color_blend_state.attachments.size());

        for (const rhi::ColorBlendDesc& desc :
             create_info.color_blend_state.attachments) {
            result.push_back(helpers::map_texture_format(desc.format));
        }

        return result;
    }();

    const VkPipelineRenderingCreateInfo pipeline_rendering_create_info {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
        .colorAttachmentCount = static_cast<u32>(color_attachment_formats.size()),
        .pColorAttachmentFormats = color_attachment_formats.data(),
        .depthAttachmentFormat = helpers::map_texture_format(
            create_info.depth_stencil.format),
        .stencilAttachmentFormat = helpers::map_texture_format(
            create_info.depth_stencil.format),
    };

    const VkGraphicsPipelineCreateInfo graphics_pipeline_create_info {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext = &pipeline_rendering_create_info,
        .stageCount = static_cast<u32>(shader_stage_create_infos.size()),
        .pStages = shader_stage_create_infos.data(),
        .pVertexInputState = &vertex_input_state_create_info,
        .pInputAssemblyState = &input_assembly_state_create_info,
        .pTessellationState = nullptr, // We don't use tessellation
        .pViewportState = &viewport_state_create_info,
        .pRasterizationState = &rasterization_state_create_info,
        .pMultisampleState = &multisample_state_create_info,
        .pDepthStencilState = &depth_stencil_state_create_info,
        .pColorBlendState = &color_blend_state_create_info,
        .pDynamicState = &dynamic_state_create_info,
        .layout = managers.pipeline_layout_manager->get_pipeline_layout().pipeline_layout,
        .renderPass = VK_NULL_HANDLE, // #NOTE: We are using dynamic rendering.
    };

    m_pipeline = vulkan_map_result(
        m_raw_device->get_device().create_graphics_pipelines(
            managers.pipeline_cache_manager->get_pipeline_cache(),
            core::as_span(graphics_pipeline_create_info),
            nullptr),
        "`create_graphics_pipelines` failed")[0];
}

VulkanGraphicsPipeline::~VulkanGraphicsPipeline() noexcept
{
    TNDR_PROFILER_TRACE("VulkanGraphicsPipeline::~VulkanGraphicsPipeline");

    if (m_pipeline != VK_NULL_HANDLE) {
        m_raw_device->get_device().destroy_pipeline(m_pipeline, nullptr);
        m_pipeline = VK_NULL_HANDLE;
    }
}

VulkanGraphicsPipeline::VulkanGraphicsPipeline(VulkanGraphicsPipeline&& rhs) noexcept
    : m_raw_device(core::move(rhs.m_raw_device))
    , m_pipeline(core::exchange(rhs.m_pipeline, VK_NULL_HANDLE))
{
}

VulkanGraphicsPipeline& VulkanGraphicsPipeline::operator=(
    VulkanGraphicsPipeline&& rhs) noexcept
{
    if (&rhs != this) {
        m_raw_device = core::move(rhs.m_raw_device);
        m_pipeline = core::exchange(rhs.m_pipeline, VK_NULL_HANDLE);
    }

    return *this;
}

VkPipeline VulkanGraphicsPipeline::get_pipeline() const noexcept
{
    return m_pipeline;
}

} // namespace tundra::vulkan_rhi
