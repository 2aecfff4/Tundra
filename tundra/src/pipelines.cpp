#include "pipelines.h"

namespace tundra::pipelines {

///
[[nodiscard]] const core::Array<std::pair<const char*, PipelineInfo>>& get_pipelines()
{

    const core::Array<rhi::ColorBlendDesc> color_blend_attachments {
        rhi::ColorBlendDesc {
            .mask = rhi::ColorWriteMask::RGBA,
            .format = rhi::TextureFormat::R8_G8_B8_A8_UNORM,
        },
    };

    static const core::Array<std::pair<const char*, PipelineInfo>> pipelines =
        core::Array<std::pair<const char*, PipelineInfo>> {
            { common::culling::INSTANCE_CULLING_AND_LOD_PIPELINE_NAME, Compute {} },
            { common::culling::MESHLET_CULLING_NAME, Compute {} },
            { hardware::culling::CLEAR_INDEX_BUFFER_GENERATOR_DISPATCH_ARGS_NAME,
              Compute {} },
            { hardware::culling::GENERATE_INDEX_BUFFER_GENERATOR_DISPATCH_ARGS_NAME,
              Compute {} },
            { hardware::culling::INDEX_BUFFER_GENERATOR_NAME, Compute {} },
            {
                hardware::passes::VISIBILITY_BUFFER_PASS,
                Graphics {
                    .input_assembly =
                        rhi::InputAssemblyState {
                            .primitive_type = rhi::PrimitiveType::Triangle,
                        },
                    .rasterizer_state =
                        rhi::RasterizerState {
                            .polygon_mode = rhi::PolygonMode::Fill,
                            .front_face = rhi::FrontFace::CounterClockwise,
                        },
                    .depth_stencil =
                        rhi::DepthStencilDesc {
                            .depth_test =
                                rhi::DepthTest {
                                    .op = rhi::CompareOp::GreaterOrEqual,
                                    .write = true,
                                },
                            .format = rhi::TextureFormat::D32_FLOAT_S8_UINT,
                        },
                    .color_blend_state =
                        rhi::ColorBlendState {
                            .attachments = color_blend_attachments,
                        },
                },
            },
            { software::passes::GPU_RASTERIZE_INIT_NAME, Compute {} },
            { software::passes::GPU_RASTERIZE_PASS_NAME, Compute {} },
            { software::passes::GPU_RASTERIZE_DEBUG_PASS_NAME, Compute {} },
        };

    return pipelines;
}

} // namespace tundra::pipelines
