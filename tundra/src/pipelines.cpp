#include "pipelines.h"
#include "fmt/format.h"

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

    static const core::Array<std::pair<const char*, PipelineInfo>> pipelines = core::Array<
        std::pair<const char*, PipelineInfo>> {
        { common::culling::INSTANCE_CULLING_AND_LOD_PIPELINE_INIT_NAME, Compute {} },
        { common::culling::INSTANCE_CULLING_AND_LOD_PIPELINE_NAME, Compute {} },
        { common::culling::MESHLET_CULLING_INIT_NAME, Compute {} },
        { common::culling::MESHLET_CULLING_NAME, Compute {} },
        { hardware::culling::INDEX_BUFFER_GENERATOR_INIT_NAME, Compute {} },
        { hardware::culling::INDEX_BUFFER_GENERATOR_CLEAR_NAME, Compute {} },
        { hardware::culling::INDEX_BUFFER_GENERATOR_NAME, Compute {} },
        { hardware::culling::GENERATE_DRAW_INDIRECT_COMMANDS_NAME, Compute {} },
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
                .shaders =
                    GraphicShaders::VertexShaders {
                        .vertex_shader = fmt::format(
                            "{}.{}", hardware::passes::VISIBILITY_BUFFER_PASS, "vert"),
                        .fragment_shader = fmt::format(
                            "{}.{}", hardware::passes::VISIBILITY_BUFFER_PASS, "frag"),
                    },
            },
        },
        { software::passes::GPU_RASTERIZE_INIT_NAME, Compute {} },
        { software::passes::GPU_RASTERIZE_PASS_NAME, Compute {} },
        { software::passes::GPU_RASTERIZE_DEBUG_PASS_NAME, Compute {} },

        { mesh_shaders::passes::INSTANCE_CULLING_INIT_NAME, Compute {} },
        { mesh_shaders::passes::INSTANCE_CULLING_NAME, Compute {} },
        { mesh_shaders::passes::TASK_DISPATCH_COMMAND_GENERATOR_NAME, Compute {} },
        // {
        //     mesh_shaders::passes::MESH_SHADER_NAME,
        //     Graphics {
        //         .input_assembly =
        //             rhi::InputAssemblyState {
        //                 .primitive_type = rhi::PrimitiveType::Triangle,
        //             },
        //         .rasterizer_state =
        //             rhi::RasterizerState {
        //                 .polygon_mode = rhi::PolygonMode::Fill,
        //                 .front_face = rhi::FrontFace::CounterClockwise,
        //             },
        //         .depth_stencil =
        //             rhi::DepthStencilDesc {
        //                 .depth_test =
        //                     rhi::DepthTest {
        //                         .op = rhi::CompareOp::GreaterOrEqual,
        //                         .write = true,
        //                     },
        //                 .format = rhi::TextureFormat::D32_FLOAT_S8_UINT,
        //             },
        //         .color_blend_state =
        //             rhi::ColorBlendState {
        //                 .attachments = color_blend_attachments,
        //             },
        //         .shaders =
        //             GraphicShaders::MeshShaders {
        //                 .task_shader = fmt::format(
        //                     "{}.{}", mesh_shaders::passes::MESH_SHADER_NAME, "task"),
        //                 .mesh_shader = fmt::format(
        //                     "{}.{}", mesh_shaders::passes::MESH_SHADER_NAME, "mesh"),
        //                 .fragment_shader = fmt::format(
        //                     "{}.{}", mesh_shaders::passes::MESH_SHADER_NAME, "frag"),
        //             },
        //     },
        // },
        { passes::MATERIAL_PASS_NAME, Compute {} },
    };

    return pipelines;
}

} // namespace tundra::pipelines
