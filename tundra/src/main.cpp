#include "app.h"
#include "core/module/module_manager.h"
#include "core/profiler.h"
#include "core/std/containers/hash_map.h"
#include "core/std/defer.h"
#include "core/std/unique_ptr.h"
#include "core/typedefs.h"
#include "globals/globals.h"
#include "math/transform.h"
#include "math/vector3.h"
#include "meshlet_mesh.h"
#include "renderer/frame_graph/frame_graph.h"
#include "renderer/renderer.h"
#include "rhi/config.h"
#include "rhi/rhi_context.h"
#include "rhi/rhi_module.h"
#include "rhi/validation_layers.h"
#include "shader.h"
#include <cxxopts.hpp>
#include <filesystem>
#include <fstream>
#include <ios>
#include <random>
#include <thread>

#include <GLFW/glfw3.h>

namespace tundra {

///
class MeshletApp : public App {
private:
    static constexpr usize NUM_INSTANCES = 1;

private:
    renderer::frame_graph::FrameGraph m_frame_graph;

private:
    rhi::BufferHandle m_mesh_descriptors_buffer;
    rhi::BufferHandle m_mesh_data_buffer;

    core::Array<renderer::MeshInstance> m_mesh_instances;
    core::Array<renderer::MeshDescriptor> m_mesh_descriptors;
    core::Array<math::Transform> m_instances_transforms;

    rhi::BufferHandle m_gpu_instance_transforms_buffer[rhi::config::MAX_FRAMES_IN_FLIGHT];
    rhi::BufferHandle m_mesh_instances_buffer[rhi::config::MAX_FRAMES_IN_FLIGHT];

    core::HashMap<core::String, rhi::ComputePipelineHandle> m_compute_pipelines;
    core::HashMap<core::String, rhi::GraphicsPipelineHandle> m_graphics_pipelines;

    static inline const core::String ASSET_PATH =
        "H:/Programming/Projects/Tundra2/assets/";

private:
    u64 m_frame_index = 0;

public:
    MeshletApp() noexcept
        : App()
        , m_frame_graph(globals::g_rhi_context)
    {
        m_mesh_descriptors_buffer = globals::g_rhi_context->create_buffer(
            rhi::BufferCreateInfo {
                .usage = rhi::BufferUsageFlags::SRV,
                .memory_type = rhi::MemoryType::Dynamic,
                .size = sizeof(shader::MeshDescriptor),
                .name = "mesh_descriptors_buffer",
            });

        for (rhi::BufferHandle& buffer : m_gpu_instance_transforms_buffer) {
            buffer = globals::g_rhi_context->create_buffer(rhi::BufferCreateInfo {
                .usage = rhi::BufferUsageFlags::SRV,
                .memory_type = rhi::MemoryType::Dynamic,
                .size = NUM_INSTANCES * sizeof(shader::InstanceTransform),
                .name = "gpu_instance_transforms",
            });
        }

        for (usize i = 0; i < rhi::config::MAX_FRAMES_IN_FLIGHT; ++i) {
            rhi::BufferHandle& buffer = m_mesh_instances_buffer[i];
            buffer = globals::g_rhi_context->create_buffer(rhi::BufferCreateInfo {
                .usage = rhi::BufferUsageFlags::SRV,
                .memory_type = rhi::MemoryType::Dynamic,
                .size = NUM_INSTANCES * sizeof(shader::MeshInstance),
                .name = "mesh_instances",
            });

            core::Array<shader::MeshInstance> mesh_instances(
                NUM_INSTANCES,
                shader::MeshInstance {
                    .mesh_descriptor_index = 0,
                });

            globals::g_rhi_context->update_buffer(
                buffer,
                {
                    rhi::BufferUpdateRegion {
                        .src = core::as_byte_span(mesh_instances),
                        .dst_offset = 0,
                    },
                });
        }

        std::mt19937 gen { 0 };
        std::uniform_real_distribution<f32> dist { 0, 1 };
        const f32 scene_radius = 10.f;
        for (usize i = 0; i < NUM_INSTANCES; ++i) {
            math::Transform transform = math::Transform::IDENTITY;
            // transform.position = math::Vec3 {
            //     dist(gen) * scene_radius * 2 - scene_radius,
            //     dist(gen) * scene_radius * 2 - scene_radius,
            //     dist(gen) * scene_radius * 2 - scene_radius,
            // };
            transform.scale = 1.f;

            m_instances_transforms.push_back(transform);
            m_mesh_instances.push_back(renderer::MeshInstance {
                .mesh_descriptor_index = 0,
            });
        }

        this->upload_mesh();
        this->create_pipelines();
    }

    ~MeshletApp()
    {
        for (const auto& [_, pipeline] : m_compute_pipelines) {
            globals::g_rhi_context->destroy_compute_pipeline(pipeline);
        }

        for (const auto& [_, pipeline] : m_graphics_pipelines) {
            globals::g_rhi_context->destroy_graphics_pipeline(pipeline);
        }
    }

private:
    void upload_mesh() noexcept
    {
        // MeshletMesh::import(
        //     ASSET_PATH + "level_test.glb", ASSET_PATH + "level_test.meshlet_mesh");
        // MeshletMesh meshlet_mesh = MeshletMesh::load(
        //     ASSET_PATH + "level_test.meshlet_mesh");

        MeshletMesh::import(
            ASSET_PATH + "monkey.glb", ASSET_PATH + "monkey.meshlet_mesh");
        MeshletMesh meshlet_mesh = MeshletMesh::load(ASSET_PATH + "monkey.meshlet_mesh");

        core::Array<u32> meshlet_triangles(meshlet_mesh.meshlet_triangles.size());
        for (usize i = 0; i < meshlet_triangles.size(); ++i) {
            meshlet_triangles[i] = static_cast<u32>(meshlet_mesh.meshlet_triangles[i]);
        }

        core::Array<shader::Meshlet> meshlets;
        meshlets.reserve(meshlet_mesh.meshlets.size());
        for (const MeshletMesh::Meshlet& meshlet : meshlet_mesh.meshlets) {
            meshlets.push_back(shader::Meshlet {
                .center = meshlet.center,
                .radius = meshlet.radius,
                .cone_axis_and_cutoff = meshlet.cone_axis_and_cutoff,
                .triangle_offset = meshlet.triangle_offset,
                .triangle_count = meshlet.triangle_count,
                .vertex_offset = meshlet.vertex_offset,
                .vertex_count = meshlet.vertex_count,
            });
        }

        core::Array<math::Vec2> flattened_uvs;
        for (const core::Array<math::Vec2>& uv : meshlet_mesh.uvs) {
            flattened_uvs.insert(flattened_uvs.end(), uv.begin(), uv.end());
        }

        u64 total_size = 0;

        const u64 meshlets_size_bytes = sizeof(shader::Meshlet) *
                                        meshlet_mesh.meshlets.size();
        total_size += meshlets_size_bytes;

        const u64 meshlet_triangles_offset = total_size;
        const u64 meshlet_triangles_size_bytes = sizeof(u32) * meshlet_triangles.size();
        total_size += meshlet_triangles_size_bytes;

        const u64 meshlet_vertices_offset = total_size;
        const u64 meshlet_vertices_size_bytes = sizeof(u32) *
                                                meshlet_mesh.meshlet_vertices.size();
        total_size += meshlet_vertices_size_bytes;

        const u64 vertices_offset = total_size;
        const u64 vertices_size_bytes = meshlet_mesh.vertices.size() * sizeof(math::Vec3);
        total_size += vertices_size_bytes;

        const u64 normals_size_bytes = meshlet_mesh.normals.size() * sizeof(math::Vec3);
        total_size += normals_size_bytes;

        const u64 tangents_size_bytes = meshlet_mesh.tangents.size() * sizeof(math::Vec4);
        total_size += tangents_size_bytes;

        for (const core::Array<math::Vec2>& uv : meshlet_mesh.uvs) {
            total_size += (uv.size() * sizeof(math::Vec2));
        }

        m_mesh_data_buffer = globals::g_rhi_context->create_buffer(rhi::BufferCreateInfo {
            .usage = rhi::BufferUsageFlags::SRV,
            .memory_type = rhi::MemoryType::Dynamic,
            .size = total_size,
            .name = "mesh_data_buffer",
        });

        core::Array<rhi::BufferUpdateRegion> update_regions;
        update_regions.push_back(rhi::BufferUpdateRegion {
            .src = core::as_byte_span(meshlets),
            .dst_offset = 0,
        });
        update_regions.push_back(rhi::BufferUpdateRegion {
            .src = core::as_byte_span(meshlet_triangles),
            .dst_offset = meshlet_triangles_offset,
        });
        update_regions.push_back(rhi::BufferUpdateRegion {
            .src = core::as_byte_span(meshlet_mesh.meshlet_vertices),
            .dst_offset = meshlet_vertices_offset,
        });
        u64 offset = vertices_offset;
        update_regions.push_back(rhi::BufferUpdateRegion {
            .src = core::as_byte_span(meshlet_mesh.vertices),
            .dst_offset = offset,
        });
        offset += core::as_byte_span(meshlet_mesh.vertices).size();

        update_regions.push_back(rhi::BufferUpdateRegion {
            .src = core::as_byte_span(meshlet_mesh.normals),
            .dst_offset = offset,
        });
        offset += core::as_byte_span(meshlet_mesh.normals).size();

        update_regions.push_back(rhi::BufferUpdateRegion {
            .src = core::as_byte_span(meshlet_mesh.tangents),
            .dst_offset = offset,
        });
        offset += core::as_byte_span(meshlet_mesh.tangents).size();

        update_regions.push_back(rhi::BufferUpdateRegion {
            .src = core::as_byte_span(flattened_uvs),
            .dst_offset = offset,
        });

        globals::g_rhi_context->update_buffer(m_mesh_data_buffer, update_regions);

        const shader::MeshDescriptor mesh_descriptor = [&] {
            shader::VertexBufferLayout layout = shader::VertexBufferLayout::NONE;
            if (!meshlet_mesh.vertices.empty()) {
                layout |= shader::VertexBufferLayout::POSITIONS;
            }
            if (!meshlet_mesh.normals.empty()) {
                layout |= shader::VertexBufferLayout::NORMALS;
            }
            if (!meshlet_mesh.tangents.empty()) {
                layout |= shader::VertexBufferLayout::TANGENTS;
            }

            switch (meshlet_mesh.uvs.size()) {
                case 0: {
                    break;
                }
                case 1: {
                    layout |= shader::VertexBufferLayout::UV0;
                    break;
                }
                case 2: {
                    layout |= shader::VertexBufferLayout::UV0 |
                              shader::VertexBufferLayout::UV1;
                    break;
                }
                case 3: {
                    layout |= shader::VertexBufferLayout::UV0 |
                              shader::VertexBufferLayout::UV1 |
                              shader::VertexBufferLayout::UV2;
                    break;
                }
                default: {
                    core::unreachable();
                    break;
                }
            }

            return shader::MeshDescriptor {
                .center = meshlet_mesh.center,
                .radius = meshlet_mesh.radius,
                .mesh_data_buffer_srv = m_mesh_data_buffer.get_srv(),
                .meshlet_count = static_cast<u32>(meshlet_mesh.meshlets.size()),
                .meshlet_triangles_offset = static_cast<u32>(meshlet_triangles_offset),
                .meshlet_triangles_count = static_cast<u32>(meshlet_triangles.size()),
                .meshlet_vertices_offset = static_cast<u32>(meshlet_vertices_offset),
                .meshlet_vertices_count = static_cast<u32>(
                    meshlet_mesh.meshlet_vertices.size()),
                .vertex_buffer_offset = static_cast<u32>(vertices_offset),
                .vertex_count = static_cast<u32>(meshlet_mesh.vertices.size()),
                .vertex_buffer_layout = layout,
            };
        }();

        globals::g_rhi_context->update_buffer(
            m_mesh_descriptors_buffer,
            {
                rhi::BufferUpdateRegion {
                    .src = core::as_byte_span(mesh_descriptor),
                    .dst_offset = 0,
                },
            });

        m_mesh_descriptors.push_back(renderer::MeshDescriptor {
            .center = meshlet_mesh.center,
            .radius = meshlet_mesh.radius,
            .meshlet_count = static_cast<u32>(meshlet_mesh.meshlets.size()),
            .meshlet_triangles_count = static_cast<u32>(meshlet_triangles.size()),
            .meshlet_vertices_count = static_cast<u32>(
                meshlet_mesh.meshlet_vertices.size()),
            .vertex_count = static_cast<u32>(meshlet_mesh.vertices.size()),
            .vertex_buffer_layout = mesh_descriptor.vertex_buffer_layout,
        });
    }

    void create_pipelines() noexcept
    {
        const auto read_file = [](const core::String& path) -> core::Array<char> {
            std::ifstream file(path, std::ios::binary | std::ios::ate);
            tndr_assert(file.is_open(), "");
            core::Array<char> buffer(static_cast<usize>(file.tellg()));
            file.seekg(0);
            file.read(buffer.data(), buffer.capacity());
            file.close();
            return buffer;
        };

        const auto create_compute_pipeline = [&](const core::String& path) {
            const core::Array<char> shader_buffer = read_file(path);
            const rhi::ShaderHandle shader = globals::g_rhi_context->create_shader(
                rhi::ShaderCreateInfo {
                    .shader_stage = rhi::ShaderStage::ComputeShader,
                    .shader_buffer = core::as_span(shader_buffer),
                });
            tndr_defer {
                globals::g_rhi_context->destroy_shader(shader);
            };

            return globals::g_rhi_context->create_compute_pipeline(
                rhi::ComputePipelineCreateInfo {
                    .compute_shader = shader,
                    .name = pipelines::INSTANCE_CULLING_AND_LOD_PIPELINE_NAME,
                });
        };

        // instance_culling_and_lod
        m_compute_pipelines.insert({
            pipelines::MESHLET_CULLING_NAME,
            create_compute_pipeline(
                ASSET_PATH + "shaders/visibility_buffer/culling/"
                             "instance_culling_and_lod.comp.hlsl.spv"),
        });

        // meshlet_culling
        m_compute_pipelines.insert({
            pipelines::MESHLET_CULLING_NAME,
            create_compute_pipeline(
                ASSET_PATH + "shaders/visibility_buffer/culling/"
                             "meshlet_culling.comp.hlsl.spv"),
        });

        // clear_index_buffer_generator_dispatch_args
        m_compute_pipelines.insert({
            pipelines::CLEAR_INDEX_BUFFER_GENERATOR_DISPATCH_ARGS_NAME,
            create_compute_pipeline(
                ASSET_PATH + "shaders/visibility_buffer/culling/"
                             "clear_index_buffer_generator_dispatch_args.comp.hlsl.spv"),
        });

        // generate_index_buffer_generator_dispatch_args
        m_compute_pipelines.insert({
            pipelines::GENERATE_INDEX_BUFFER_GENERATOR_DISPATCH_ARGS_NAME,
            create_compute_pipeline(
                ASSET_PATH +
                "shaders/visibility_buffer/culling/"
                "generate_index_buffer_generator_dispatch_args.comp.hlsl.spv"),
        });

        // index_buffer_generator
        m_compute_pipelines.insert({
            pipelines::INDEX_BUFFER_GENERATOR_NAME,
            create_compute_pipeline(
                ASSET_PATH + "shaders/visibility_buffer/culling/"
                             "index_buffer_generator.comp.hlsl.spv"),
        });

        // gpu_rasterize
        m_compute_pipelines.insert({
            pipelines::GPU_RASTERIZE_PASS_NAME,
            create_compute_pipeline(
                ASSET_PATH + "shaders/visibility_buffer/passes/"
                             "gpu_rasterize.comp.hlsl.spv"),
        });

        // gpu_rasterize_debug
        m_compute_pipelines.insert({
            pipelines::GPU_RASTERIZE_DEBUG_PASS_NAME,
            create_compute_pipeline(
                ASSET_PATH + "shaders/visibility_buffer/passes/"
                             "gpu_rasterize_debug.comp.hlsl.spv"),
        });

        // visibility_buffer_pass
        {
            core::Array<char> shader_buffer = read_file(
                ASSET_PATH + "shaders/visibility_buffer/passes/"
                             "visibility_buffer_pass.frag.hlsl.spv");
            const rhi::ShaderHandle frag = globals::g_rhi_context->create_shader(
                rhi::ShaderCreateInfo {
                    .shader_stage = rhi::ShaderStage::FragmentShader,
                    .shader_buffer = core::as_span(shader_buffer),
                });
            tndr_defer {
                globals::g_rhi_context->destroy_shader(frag);
            };

            shader_buffer = read_file(
                ASSET_PATH + "shaders/visibility_buffer/passes/"
                             "visibility_buffer_pass.vert.hlsl.spv");
            const rhi::ShaderHandle vert = globals::g_rhi_context->create_shader(
                rhi::ShaderCreateInfo {
                    .shader_stage = rhi::ShaderStage::VertexShader,
                    .shader_buffer = core::as_span(shader_buffer),
                });
            tndr_defer {
                globals::g_rhi_context->destroy_shader(vert);
            };

            const core::Array<rhi::ColorBlendDesc> attachments {
                rhi::ColorBlendDesc {
                    .mask = rhi::ColorWriteMask::RGBA,
                    .format = rhi::TextureFormat::R8_G8_B8_A8_UNORM,
                },
            };

            const rhi::GraphicsPipelineCreateInfo create_info {
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
                        .attachments = attachments,
                    },
                .vertex_shader = vert,
                .fragment_shader = frag,
                .name = pipelines::VISIBILITY_BUFFER_PASS,
            };

            const rhi::GraphicsPipelineHandle pipeline =
                globals::g_rhi_context->create_graphics_pipeline(create_info);

            m_graphics_pipelines.insert({
                pipelines::VISIBILITY_BUFFER_PASS,
                pipeline,
            });
        }
    }

protected:
    virtual void tick(const f32 delta_time) noexcept override
    {
        const u64 frame_index = m_frame_index % rhi::config::MAX_FRAMES_IN_FLIGHT;

        // for (usize i = 0; i < NUM_INSTANCES; ++i) {
        //     math::Transform& transform = m_instances_transforms[i];
        //     transform.rotation *= math::Quat::from_angle(math::Vec3 {
        //         0,
        //         0,
        //         15.f * delta_time,
        //     });
        // }

        globals::g_rhi_context->update_buffer(
            m_gpu_instance_transforms_buffer[frame_index],
            { rhi::BufferUpdateRegion {
                .src = core::as_byte_span(m_instances_transforms),
                .dst_offset = 0,
            } });

        const math::UVec2 view_size = this->get_window_surface_size();
        const f32 z_near = 0.01f;

        const renderer::RenderOutput render_output = renderer::render(
            m_frame_graph,
            renderer::RenderInput {
                .world_to_view = m_camera.view,
                .view_to_clip = m_camera.projection,
                .camera_position = m_camera_transform.position,
                .view_size = view_size,
                .near_plane = z_near,
                .mesh_instances = m_mesh_instances,
                .mesh_descriptors = m_mesh_descriptors,
                .gpu_mesh_descriptors = m_mesh_descriptors_buffer,
                .gpu_mesh_instance_transforms =
                    m_gpu_instance_transforms_buffer[frame_index],
                .gpu_mesh_instances = m_mesh_instances_buffer[frame_index],
                .compute_pipelines = m_compute_pipelines,
                .graphics_pipelines = m_graphics_pipelines,
            });

        m_frame_graph.add_present_pass(this->get_swapchain(), render_output.color_output);
        m_frame_graph.compile();
        m_frame_graph.execute(globals::g_rhi_context);
        m_frame_graph.reset();
    }
};

} // namespace tundra

int main(const int argc, const char* argv[])
{
    using namespace tundra;
    rhi::IRHIModule* rhi_module = nullptr;
    core::UniquePtr<rhi::IRHIContext> rhi_context;

    if (!glfwInit()) {
        std::cerr << "`glfwInit` failed.\n";
        std::terminate();
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    core::ModuleManager::get().load_module("vulkan_rhi");
    rhi_module = core::ModuleManager::get().get_module<rhi::IRHIModule>("vulkan_rhi");

    rhi_context = core::make_unique<rhi::ValidationLayers>(rhi_module->create_rhi());
    globals::g_rhi_context = rhi_context.get();

    {
        tundra::MeshletApp app;
        app.loop();
    }

    globals::g_rhi_context = nullptr;
    rhi_context.reset();
    rhi_module = nullptr;
    core::ModuleManager::get().unload_all_modules();

    glfwTerminate();
}