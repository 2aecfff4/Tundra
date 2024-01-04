#include "renderer/hardware/hardware_rasterizer.h"
#include "core/std/shared_ptr.h"
#include "pipelines.h"
#include "renderer/common/culling/instance_culling_and_lod.h"
#include "renderer/common/culling/meshlet_culling.h"
#include "renderer/config.h"
#include "renderer/frame_graph/frame_graph.h"
#include "renderer/frame_graph/registry.h"
#include "renderer/frame_graph/resources/handle.h"
#include "renderer/hardware/culling/index_buffer_generator_init.h"
#include "renderer/helpers.h"
#include "renderer/render_input_output.h"
#include "renderer/ubo.h"
#include "rhi/commands/barrier.h"
#include "rhi/commands/command_encoder.h"
#include "rhi/commands/dispatch_indirect.h"
#include "rhi/commands/draw_indirect.h"
#include "rhi/resources/access_flags.h"
#include "rhi/rhi_context.h"
#include <array>

namespace tundra::renderer::hardware {

namespace ubo {

///
struct GenerateIndexBufferGeneratorDispatchArgsUbo {
    struct {
        u32 meshlet_offset_srv = config::INVALID_SHADER_HANDLE;
        u32 num_visible_meshlets_srv = config::INVALID_SHADER_HANDLE;
    } in_;

    struct {
        u32 index_generator_dispatch_args_uav = config::INVALID_SHADER_HANDLE;
    } out_;
};

///
struct IndexBufferGeneratorClearUbo {
    u32 visible_indices_count_uav = config::INVALID_SHADER_HANDLE;
};

///
struct IndexBufferGeneratorUBO {
    math::Mat4 world_to_clip;
    math::UVec2 view_size;

    u32 max_num_indices = 0;

    struct {
        u32 index = 0;
        u32 meshlet_offsets_srv = config::INVALID_SHADER_HANDLE;
        u32 visible_meshlets_count_srv = config::INVALID_SHADER_HANDLE;

        u32 mesh_instance_transforms_srv = config::INVALID_SHADER_HANDLE;
        u32 mesh_descriptors_srv = config::INVALID_SHADER_HANDLE;
        u32 visible_meshlets_srv = config::INVALID_SHADER_HANDLE;
        u32 meshlets_offset_srv = config::INVALID_SHADER_HANDLE;
    } in_;

    struct {
        u32 index_buffer_uav = config::INVALID_SHADER_HANDLE;
        u32 visible_indices_count_uav = config::INVALID_SHADER_HANDLE;
    } out_;
};

///
struct GenerateIndirectCommandsUbo {
    struct {
        u32 visible_indices_count_srv = config::INVALID_SHADER_HANDLE;

        // u32 dispatch_indirect_args_srv = config::INVALID_SHADER_HANDLE;
    } in_;

    struct {
        u32 draw_meshlets_draw_args_uav = config::INVALID_SHADER_HANDLE;
        u32 draw_count_uav = config::INVALID_SHADER_HANDLE;

        // u32 meshlet_offset_uav = config::INVALID_SHADER_HANDLE;
    } out_;
};

///
struct UpdateMeshletOffsetUbo {
    struct {
        u32 dispatch_indirect_args_srv = config::INVALID_SHADER_HANDLE;
    } in_;

    struct {
        u32 meshlet_offset_uav = config::INVALID_SHADER_HANDLE;
    } out_;
};

///
struct DrawMeshletsUbo {
    math::Mat4 world_to_view = math::Mat4 {};
    math::Mat4 view_to_clip = math::Mat4 {};

    struct {
        u32 visible_meshlets_srv = config::INVALID_SHADER_HANDLE;
        u32 mesh_descriptors_srv = config::INVALID_SHADER_HANDLE;
        u32 mesh_instance_transforms_srv = config::INVALID_SHADER_HANDLE;
    } in_;
};

} // namespace ubo

///
struct RenderMeshletsInput {
public:
    core::SharedPtr<UboBuffer> ubo_buffer;

public:
    u32 num_iterations;

public:
    math::Mat4 world_to_view = math::Mat4 {};
    math::Mat4 view_to_clip = math::Mat4 {};
    math::Mat4 world_to_clip = math::Mat4 {};
    math::UVec2 view_size = math::UVec2 {};
    u32 max_num_indices = 0;

public:
    rhi::BufferHandle gpu_mesh_descriptors;
    rhi::BufferHandle gpu_mesh_instance_transforms;

public:
    frame_graph::BufferHandle visible_meshlets_count;
    frame_graph::BufferHandle meshlet_offsets;
    frame_graph::BufferHandle index_buffer;
    frame_graph::BufferHandle visible_indices_count;

    frame_graph::BufferHandle draw_meshlets_draw_args;
    frame_graph::BufferHandle draw_count;
    frame_graph::BufferHandle index_generator_dispatch_indirect_commands;

    frame_graph::BufferHandle num_visible_meshlets;

    frame_graph::BufferHandle visible_meshlets;

public:
    const ComputePipelinesMap& compute_pipelines;
    const GraphicsPipelinesMap& graphics_pipelines;
};

///
struct RenderMeshletsOutput {
    frame_graph::TextureHandle visibility_buffer;
    frame_graph::TextureHandle depth_buffer;
};

//
RenderMeshletsOutput render_meshlets(
    frame_graph::FrameGraph& fg, const RenderMeshletsInput& input)
{
    struct Data {
        core::SharedPtr<UboBuffer> ubo_buffer;

        frame_graph::TextureHandle visibility_buffer;
        frame_graph::TextureHandle depth_buffer;

    public:
        frame_graph::BufferHandle visible_meshlets_count;
        frame_graph::BufferHandle meshlet_offsets;
        frame_graph::BufferHandle index_buffer;
        frame_graph::BufferHandle visible_indices_count;

        frame_graph::BufferHandle draw_meshlets_draw_args;
        frame_graph::BufferHandle draw_count;
        frame_graph::BufferHandle index_generator_dispatch_indirect_commands;

        frame_graph::BufferHandle num_visible_meshlets;

        frame_graph::BufferHandle visible_meshlets;
    };

    const Data data = fg.add_render_pass(
        frame_graph::QueueType::Graphics,
        "render_meshlets",
        [&](frame_graph::Builder& builder, frame_graph::RenderPass& render_pass) {
            Data data {};
            data.ubo_buffer = input.ubo_buffer;

            data.visibility_buffer = builder.write(
                builder.create_texture(
                    "visibility_buffer",
                    frame_graph::TextureCreateInfo {
                        .kind =
                            frame_graph::TextureKind::Texture2D {
                                .width = input.view_size.x,
                                .height = input.view_size.y,
                            },
                        .memory_type = frame_graph::MemoryType::GPU,
                        .format = frame_graph::TextureFormat::R8_G8_B8_A8_UNORM,
                        .usage = frame_graph::TextureUsageFlags::COLOR_ATTACHMENT |
                                 frame_graph::TextureUsageFlags::TRANSFER_SOURCE |
                                 frame_graph::TextureUsageFlags::PRESENT,
                        .tiling = frame_graph::TextureTiling::Optimal,
                    }),
                frame_graph::TextureResourceUsage::COLOR_ATTACHMENT);

            data.depth_buffer = builder.write(
                builder.create_texture(
                    "depth_buffer",
                    frame_graph::TextureCreateInfo {
                        .kind =
                            frame_graph::TextureKind::Texture2D {
                                .width = input.view_size.x,
                                .height = input.view_size.y,
                            },
                        .memory_type = frame_graph::MemoryType::GPU,
                        .format = frame_graph::TextureFormat::D32_FLOAT_S8_UINT,
                        .usage = frame_graph::TextureUsageFlags::DEPTH_ATTACHMENT,
                        .tiling = frame_graph::TextureTiling::Optimal,
                    }),
                frame_graph::TextureResourceUsage::DEPTH_STENCIL_ATTACHMENT);

            render_pass.color_attachments.push_back(frame_graph::ColorAttachment {
                .ops = frame_graph::AttachmentOps::INIT,
                .texture = data.visibility_buffer,
                .clear_value = math::Vec4 { 0, 0, 0, 0 },
            });
            render_pass.depth_stencil_attachment = frame_graph::DepthStencilAttachment {
                .ops = frame_graph::AttachmentOps::INIT,
                .texture = data.depth_buffer,
                .clear_value =
                    rhi::ClearDepthStencil {
                        .depth = 0.0f,
                        .stencil = 0,
                    },
            };

            data.visible_meshlets_count = builder.read(
                input.visible_meshlets_count,
                frame_graph::BufferResourceUsage::COMPUTE_STORAGE_BUFFER);

            data.meshlet_offsets = builder.read(
                input.meshlet_offsets,
                frame_graph::BufferResourceUsage::COMPUTE_STORAGE_BUFFER);
            data.meshlet_offsets = builder.write(
                input.meshlet_offsets,
                frame_graph::BufferResourceUsage::COMPUTE_STORAGE_BUFFER);

            data.num_visible_meshlets = builder.read(
                input.num_visible_meshlets,
                frame_graph::BufferResourceUsage::COMPUTE_STORAGE_BUFFER);

            data.index_generator_dispatch_indirect_commands = builder.write(
                input.index_generator_dispatch_indirect_commands,
                frame_graph::BufferResourceUsage::COMPUTE_STORAGE_BUFFER);

            data.visible_meshlets = builder.read(
                input.visible_meshlets,
                frame_graph::BufferResourceUsage::COMPUTE_STORAGE_BUFFER);

            data.index_buffer = builder.write(
                input.index_buffer,
                frame_graph::BufferResourceUsage::COMPUTE_STORAGE_BUFFER);
            data.visible_indices_count = builder.write(
                input.visible_indices_count,
                frame_graph::BufferResourceUsage::COMPUTE_STORAGE_BUFFER);

            data.draw_meshlets_draw_args = builder.write(
                input.draw_meshlets_draw_args,
                frame_graph::BufferResourceUsage::COMPUTE_STORAGE_BUFFER);

            data.draw_count = builder.write(
                input.draw_count,
                frame_graph::BufferResourceUsage::COMPUTE_STORAGE_BUFFER);

            return data;
        },
        [=](rhi::IRHIContext* rhi,
            const frame_graph::Registry& registry,
            rhi::CommandEncoder& encoder,
            const Data& data,
            const rhi::RenderPass& render_pass) {
            //
            const auto index_buffer_generator = [&](const u32 index) {
                encoder.begin_region("index_buffer_generator", { 0.2f, 0.5f, 0.2f, 1.f });

                const auto ubo_buffer = registry.get_buffer(data.ubo_buffer->buffer());

                const auto index_generator_dispatch_args = registry.get_buffer(
                    data.index_generator_dispatch_indirect_commands);

                const auto visible_meshlets = registry.get_buffer(data.visible_meshlets);

                const auto index_buffer = registry.get_buffer(data.index_buffer);
                const auto visible_indices_count //
                    = registry.get_buffer(data.visible_indices_count);

                const auto meshlet_offsets = registry.get_buffer(data.meshlet_offsets);
                const auto visible_meshlets_count = registry.get_buffer(
                    data.visible_meshlets_count);

                {
                    const ubo::IndexBufferGeneratorClearUbo ubo {
                        .visible_indices_count_uav = visible_indices_count.get_uav(),
                    };

                    const auto ubo_ref =
                        data.ubo_buffer->allocate<ubo::IndexBufferGeneratorClearUbo>();

                    rhi->update_buffer(
                        ubo_buffer,
                        {
                            rhi::BufferUpdateRegion {
                                .src = core::as_byte_span(ubo),
                                .dst_offset = ubo_ref.offset,
                            },
                        });

                    encoder.push_constants(ubo_buffer, ubo_ref.offset);
                    encoder.dispatch(
                        helpers::get_pipeline(
                            pipelines::hardware::culling::INDEX_BUFFER_GENERATOR_CLEAR_NAME,
                            input.compute_pipelines),
                        1,
                        1,
                        1);
                }

                encoder.global_barrier(rhi::GlobalBarrier {
                    .previous_access = rhi::GlobalAccessFlags::ALL,
                    .next_access = rhi::GlobalAccessFlags::ALL,
                });

                {
                    const ubo::IndexBufferGeneratorUBO ubo { 
                        .world_to_clip = input.world_to_clip,
                        .view_size = input.view_size,
                        .max_num_indices = input.max_num_indices,
                        .in_ = {
                            .index = index,
                            .meshlet_offsets_srv = meshlet_offsets.get_srv(),
                            .visible_meshlets_count_srv = visible_meshlets_count.get_srv(),
                            .mesh_instance_transforms_srv = input.gpu_mesh_instance_transforms.get_srv(),
                            .mesh_descriptors_srv = input.gpu_mesh_descriptors.get_srv(),
                            .visible_meshlets_srv = visible_meshlets.get_srv(),
                            .meshlets_offset_srv = meshlet_offsets.get_srv(),
                        },
                        .out_ = {
                            .index_buffer_uav = index_buffer.get_uav(),
                            .visible_indices_count_uav = visible_indices_count.get_uav(),
                        }                      
                    };

                    const auto ubo_ref = data.ubo_buffer
                                             ->allocate<ubo::IndexBufferGeneratorUBO>();

                    rhi->update_buffer(
                        ubo_buffer,
                        {
                            rhi::BufferUpdateRegion {
                                .src = core::as_byte_span(ubo),
                                .dst_offset = ubo_ref.offset,
                            },
                        });

                    encoder.push_constants(ubo_buffer, ubo_ref.offset);
                    encoder.dispatch_indirect(
                        helpers::get_pipeline(
                            pipelines::hardware::culling::INDEX_BUFFER_GENERATOR_NAME,
                            input.compute_pipelines),
                        index_generator_dispatch_args,
                        sizeof(rhi::DispatchIndirectCommand) * index);
                }
                encoder.end_region();
            };

            //
            const auto generate_draw_indirect_commands = [&] {
                encoder.begin_region(
                    "generate_draw_indirect_commands", { 0.9f, 0.0f, 0.6f, 1.f });

                const rhi::BufferHandle ubo_buffer = registry.get_buffer(
                    data.ubo_buffer->buffer());
                const rhi::BufferHandle visible_indices_count = registry.get_buffer(
                    data.visible_indices_count);
                const rhi::BufferHandle draw_meshlets_draw_args = registry.get_buffer(
                    data.draw_meshlets_draw_args);
                const rhi::BufferHandle draw_count = registry.get_buffer(data.draw_count);

                const ubo::GenerateIndirectCommandsUbo ubo {
                    .in_ {
                        .visible_indices_count_srv = visible_indices_count.get_srv(),

                        //.dispatch_indirect_args_srv = 0,
                    },
                    .out_ {
                        .draw_meshlets_draw_args_uav = draw_meshlets_draw_args.get_uav(),
                        .draw_count_uav = draw_count.get_uav(),
                    }
                };

                const auto ubo_ref = data.ubo_buffer
                                         ->allocate<ubo::GenerateIndirectCommandsUbo>();

                rhi->update_buffer(
                    ubo_buffer,
                    {
                        rhi::BufferUpdateRegion {
                            .src = core::as_byte_span(ubo),
                            .dst_offset = ubo_ref.offset,
                        },
                    });

                encoder.push_constants(ubo_buffer, ubo_ref.offset);
                encoder.dispatch(
                    helpers::get_pipeline(
                        pipelines::hardware::culling::GENERATE_DRAW_INDIRECT_COMMANDS_NAME,
                        input.compute_pipelines),
                    1,
                    1,
                    1);
                encoder.end_region();
            };

            auto render_pass_copy = render_pass;
            render_pass_copy.color_attachments[0]
                .ops = frame_graph::AttachmentOps::PRESERVE;
            // rhi::ColorAttachment {
            //     .ops = frame_graph::AttachmentOps::PRESERVE,
            //     .texture = registry.get_texture(data.visibility_buffer).get_handle(),
            // };
            render_pass_copy.depth_stencil_attachment
                ->ops = frame_graph::AttachmentOps::PRESERVE;
            // render_pass_copy.depth_stencil_attachment = rhi::DepthStencilAttachment {
            //     .ops = frame_graph::AttachmentOps::PRESERVE,
            //     .texture = registry.get_texture(data.depth_buffer).get_handle(),
            // };

            //
            const auto draw_meshlets = [&, render_pass = render_pass_copy]() {
                encoder.begin_region("draw_meshlets", { 1.f, 0.1f, 0.6f, 1.f });
                //
                const auto ubo_buffer = registry.get_buffer(data.ubo_buffer->buffer());

                const auto visible_meshlets = registry.get_buffer(data.visible_meshlets);

                const auto draw_meshlets_draw_args = registry.get_buffer(
                    data.draw_meshlets_draw_args);
                const auto draw_count = registry.get_buffer(data.draw_count);
                const auto index_buffer = registry.get_buffer(data.index_buffer);

                const ubo::DrawMeshletsUbo
                    ubo { .world_to_view = input.world_to_view,
                          .view_to_clip = input.view_to_clip,
                          .in_ = {
                              .visible_meshlets_srv = visible_meshlets.get_srv(),
                              .mesh_descriptors_srv = input.gpu_mesh_descriptors.get_srv(),
                              .mesh_instance_transforms_srv =
                                  input.gpu_mesh_instance_transforms.get_srv(),
                          } };

                const auto ubo_ref = data.ubo_buffer->allocate<ubo::DrawMeshletsUbo>();

                rhi->update_buffer(
                    ubo_buffer,
                    {
                        rhi::BufferUpdateRegion {
                            .src = core::as_byte_span(ubo),
                            .dst_offset = ubo_ref.offset,
                        },
                    });

                encoder.push_constants(ubo_buffer, ubo_ref.offset);

                encoder.set_viewport(rhi::Viewport {
                    .rect =
                        rhi::Rect {
                            .offset = { 0, 0 },
                            .extent = input.view_size,
                        },
                });

                encoder.set_scissor(rhi::Scissor {
                    .offset = { 0, 0 },
                    .extent = input.view_size,
                });

                encoder.set_culling_mode(rhi::CullingMode::None);
                encoder.begin_render_pass(
                    rhi::Rect {
                        .offset = { 0, 0 },
                        .extent = input.view_size,
                    },
                    render_pass);

                encoder.bind_graphics_pipeline(helpers::get_pipeline(
                    pipelines::hardware::passes::VISIBILITY_BUFFER_PASS,
                    input.graphics_pipelines));
                encoder.bind_index_buffer(index_buffer, 0, rhi::IndexType::U32);

                encoder.draw_indexed_indirect_count(
                    draw_meshlets_draw_args,
                    0,
                    draw_count,
                    0,
                    4,
                    sizeof(rhi::DrawIndexedIndirectCommand));

                encoder.end_render_pass();
                encoder.end_region();
            };

            const auto global_barrier = [&] {
                encoder.global_barrier(rhi::GlobalBarrier {
                    .previous_access = rhi::GlobalAccessFlags::ALL,
                    .next_access = rhi::GlobalAccessFlags::ALL,
                });
            };
            global_barrier();

            ///////////////////////////////////////////////////////////////
            // Clear color texture
            encoder.set_viewport(rhi::Viewport {
                .rect =
                    rhi::Rect {
                        .offset = { 0, 0 },
                        .extent = input.view_size,
                    },
            });

            encoder.set_scissor(rhi::Scissor {
                .offset = { 0, 0 },
                .extent = input.view_size,
            });

            encoder.set_culling_mode(rhi::CullingMode::Back);

            encoder.begin_render_pass(
                rhi::Rect {
                    .offset = { 0, 0 },
                    .extent = input.view_size,
                },
                render_pass);
            encoder.end_render_pass();

            // #TODO: Remove global barriers
            for (u32 i = 0; i < input.num_iterations; ++i) {
                {
                    global_barrier();
                    encoder.global_barrier(rhi::GlobalBarrier {
                        .previous_access = rhi::GlobalAccessFlags::COMPUTE_STORAGE_BUFFER |
                                           rhi::GlobalAccessFlags::INDIRECT_BUFFER |
                                           rhi::GlobalAccessFlags::INDEX_BUFFER,
                        .next_access = rhi::GlobalAccessFlags::COMPUTE_STORAGE_BUFFER |
                                       rhi::GlobalAccessFlags::INDIRECT_BUFFER,
                    });
                    index_buffer_generator(i);
                    encoder.global_barrier(rhi::GlobalBarrier {
                        .previous_access = rhi::GlobalAccessFlags::COMPUTE_STORAGE_BUFFER |
                                           rhi::GlobalAccessFlags::INDIRECT_BUFFER,
                        .next_access = rhi::GlobalAccessFlags::COMPUTE_STORAGE_BUFFER |
                                       rhi::GlobalAccessFlags::INDIRECT_BUFFER,
                    });
                    global_barrier();
                }

                {
                    global_barrier();
                    generate_draw_indirect_commands();
                    encoder.global_barrier(rhi::GlobalBarrier {
                        .previous_access = rhi::GlobalAccessFlags::COMPUTE_STORAGE_BUFFER |
                                           rhi::GlobalAccessFlags::INDIRECT_BUFFER |
                                           rhi::GlobalAccessFlags::ALL,
                        .next_access = rhi::GlobalAccessFlags::COMPUTE_STORAGE_BUFFER |
                                       rhi::GlobalAccessFlags::INDIRECT_BUFFER |
                                       rhi::GlobalAccessFlags::INDEX_BUFFER,
                    });
                    global_barrier();
                }

                {
                    global_barrier();
                    draw_meshlets();
                    global_barrier();
                }
            }

            global_barrier();
        });
    return RenderMeshletsOutput {
        .visibility_buffer = data.visibility_buffer,
        .depth_buffer = data.depth_buffer,
    };
}

///
RenderOutput hardware_rasterizer(
    frame_graph::FrameGraph& fg, const RenderInput& input) noexcept
{
    const usize instance_count = input.mesh_instances.size();

    const u32 max_meshlet_count = [&] {
        u32 count = 0;
        for (const MeshInstance& mesh_instance : input.mesh_instances) {
            count += input.mesh_descriptors[mesh_instance.mesh_descriptor_index]
                         .meshlet_count;
        }

        return count;
    }();

    const math::Mat4 frustum = input.view_to_clip * input.world_to_view;
    const math::Mat4 frustum_t = math::transpose(frustum);

    // Fast Extraction of Viewing Frustum Planes from the World-View-Projection Matrix - Gil Gribb and Klaus Hartmann
    const std::array<math::Vec4, 6> frustum_planes {
        math::normalize(-frustum_t[3] - frustum_t[0]), //
        math::normalize(-frustum_t[3] + frustum_t[0]), //
        math::normalize(-frustum_t[3] - frustum_t[1]), //
        math::normalize(-frustum_t[3] + frustum_t[1]), //
        math::normalize(-frustum_t[3] - frustum_t[2]), //
        math::normalize(-frustum_t[3] + frustum_t[2]),
    };

    struct UboData {
        // #TODO: This should be provided by frame graph
        core::SharedPtr<UboBuffer> ubo_buffer;
    };

    UboData ubo_data = fg.add_pass(
        frame_graph::QueueType::Graphics,
        "create_ubo",
        [&](frame_graph::Builder& builder) {
            constexpr u64 ubo_size = 1024ull * 1024;

            const auto ubo_buffer = builder.create_buffer(
                "ubo_buffer",
                frame_graph::BufferCreateInfo {
                    .usage = frame_graph::BufferUsageFlags::STORAGE_BUFFER,
                    .memory_type = frame_graph::MemoryType::Dynamic,
                    .size = ubo_size,
                });

            builder.write(
                ubo_buffer,
                frame_graph::BufferResourceUsage::COMPUTE_STORAGE_BUFFER |
                    frame_graph::BufferResourceUsage::GRAPHICS_STORAGE_BUFFER);

            return UboData {
                .ubo_buffer = core::make_shared<UboBuffer>(ubo_buffer, ubo_size),
            };
        },
        [=](rhi::IRHIContext*,
            const frame_graph::Registry&,
            rhi::CommandEncoder&,
            const UboData&) {});

    const common::culling::InstanceCullingOutput instance_culling =
        common::culling::instance_culling_and_lod(
            fg,
            common::culling::InstanceCullingInput {
                .ubo_buffer = ubo_data.ubo_buffer,
                .frustum_planes = frustum_planes,
                .instance_count = static_cast<u32>(instance_count),
                .mesh_descriptors = input.gpu_mesh_descriptors,
                .mesh_instances = input.gpu_mesh_instances,
                .mesh_instance_transforms = input.gpu_mesh_instance_transforms,
                .compute_pipelines = input.compute_pipelines,
            });

    const common::culling::MeshletCullingOutput meshlet_culling =
        common::culling::meshlet_culling(
            fg,
            common::culling::MeshletCullingInput {
                .ubo_buffer = ubo_data.ubo_buffer,
                .frustum_planes = frustum_planes,
                .camera_position = input.camera_position,
                .world_to_view = input.world_to_view,
                .max_meshlet_count = max_meshlet_count,
                .mesh_descriptors = input.gpu_mesh_descriptors,
                .mesh_instance_transforms = input.gpu_mesh_instance_transforms,
                .visible_instances = instance_culling.visible_instances,
                .meshlet_culling_dispatch_args = instance_culling
                                                     .meshlet_culling_dispatch_args,
                .compute_pipelines = input.compute_pipelines,
            });

    static constexpr u32 NUM_MESHLETS_PER_ITERATION =
        config::NUM_MESHLETS_PER_INDEX_BUFFER * config::NUM_INDEX_BUFFERS_IN_FIGHT;
    const u32 num_iterations = (max_meshlet_count + (NUM_MESHLETS_PER_ITERATION - 1)) /
                               NUM_MESHLETS_PER_ITERATION;

    const hardware::culling::IndexBufferGeneratorInitOutput index_buffer_generator_init =
        hardware::culling::index_buffer_generator_init(
            fg,
            hardware::culling::IndexBufferGeneratorInitInput {
                .ubo_buffer = ubo_data.ubo_buffer,
                .num_loops = num_iterations,
                .num_visible_meshlets = meshlet_culling.visible_meshlets_count,
                .compute_pipelines = input.compute_pipelines,
            });

    const RenderMeshletsOutput render_meshlets_out = render_meshlets(
        fg,
        RenderMeshletsInput {
            .ubo_buffer = ubo_data.ubo_buffer,
            .num_iterations = num_iterations,
            .world_to_view = input.world_to_view,
            .view_to_clip = input.view_to_clip,
            .world_to_clip = frustum,
            .view_size = input.view_size,
            .max_num_indices = config::INDEX_BUFFER_BATCH_SIZE *
                               config::NUM_INDEX_BUFFERS_IN_FIGHT,
            .gpu_mesh_descriptors = input.gpu_mesh_descriptors,
            .gpu_mesh_instance_transforms = input.gpu_mesh_instance_transforms,
            .visible_meshlets_count = meshlet_culling.visible_meshlets_count,
            .meshlet_offsets = index_buffer_generator_init.meshlet_offsets,
            .index_buffer = index_buffer_generator_init.index_buffer,
            .visible_indices_count = index_buffer_generator_init.visible_indices_count,
            .draw_meshlets_draw_args = index_buffer_generator_init.draw_meshlets_draw_args,
            .draw_count = index_buffer_generator_init.draw_count,
            .index_generator_dispatch_indirect_commands =
                index_buffer_generator_init.index_generator_dispatch_indirect_commands,
            .num_visible_meshlets = meshlet_culling.visible_meshlets_count,
            .visible_meshlets = meshlet_culling.visible_meshlets,
            .compute_pipelines = input.compute_pipelines,
            .graphics_pipelines = input.graphics_pipelines,
        });

    return RenderOutput {
        .color_output = render_meshlets_out.visibility_buffer,
    };
}

} // namespace tundra::renderer::hardware
