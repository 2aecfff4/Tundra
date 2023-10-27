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
                frame_graph::ResourceUsage::COLOR_ATTACHMENT);

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
                frame_graph::ResourceUsage::DEPTH_STENCIL_ATTACHMENT);

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
                input.visible_meshlets_count, frame_graph::ResourceUsage::SHADER_COMPUTE);

            data.meshlet_offsets = builder.read(
                input.meshlet_offsets, frame_graph::ResourceUsage::SHADER_COMPUTE);
            data.meshlet_offsets = builder.write(
                input.meshlet_offsets, frame_graph::ResourceUsage::SHADER_COMPUTE);

            data.num_visible_meshlets = builder.read(
                input.num_visible_meshlets, frame_graph::ResourceUsage::SHADER_COMPUTE);

            data.index_generator_dispatch_indirect_commands = builder.write(
                input.index_generator_dispatch_indirect_commands,
                frame_graph::ResourceUsage::SHADER_COMPUTE);

            data.visible_meshlets = builder.read(
                input.visible_meshlets, frame_graph::ResourceUsage::SHADER_COMPUTE);

            data.index_buffer = builder.write(
                input.index_buffer, frame_graph::ResourceUsage::SHADER_COMPUTE);
            data.visible_indices_count = builder.write(
                input.visible_indices_count, frame_graph::ResourceUsage::SHADER_COMPUTE);

            data.draw_meshlets_draw_args = builder.write(
                input.draw_meshlets_draw_args,
                frame_graph::ResourceUsage::SHADER_COMPUTE);

            data.draw_count = builder.write(
                input.draw_count, frame_graph::ResourceUsage::SHADER_COMPUTE);

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
                    .previous_access = rhi::AccessFlags::GENERAL,
                    .next_access = rhi::AccessFlags::GENERAL,
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
                        .previous_access = rhi::AccessFlags::UAV_COMPUTE |
                                           rhi::AccessFlags::SRV_COMPUTE |
                                           rhi::AccessFlags::INDIRECT_BUFFER |
                                           rhi::AccessFlags::INDEX_BUFFER,
                        .next_access = rhi::AccessFlags::UAV_COMPUTE |
                                       rhi::AccessFlags::SRV_COMPUTE |
                                       rhi::AccessFlags::INDIRECT_BUFFER,
                    });
                    index_buffer_generator(i);
                    encoder.global_barrier(rhi::GlobalBarrier {
                        .previous_access = rhi::AccessFlags::UAV_COMPUTE |
                                           rhi::AccessFlags::SRV_COMPUTE |
                                           rhi::AccessFlags::INDIRECT_BUFFER,
                        .next_access = rhi::AccessFlags::UAV_COMPUTE |
                                       rhi::AccessFlags::SRV_COMPUTE |
                                       rhi::AccessFlags::INDIRECT_BUFFER,
                    });
                    global_barrier();
                }

                {
                    global_barrier();
                    generate_draw_indirect_commands();
                    encoder.global_barrier(rhi::GlobalBarrier {
                        .previous_access = rhi::AccessFlags::UAV_COMPUTE |
                                           rhi::AccessFlags::SRV_COMPUTE |
                                           rhi::AccessFlags::INDIRECT_BUFFER |
                                           rhi::AccessFlags::GENERAL,
                        .next_access = rhi::AccessFlags::UAV_COMPUTE |
                                       rhi::AccessFlags::SRV_COMPUTE |
                                       rhi::AccessFlags::INDIRECT_BUFFER |
                                       rhi::AccessFlags::INDEX_BUFFER,
                    });
                    global_barrier();
                }

                {
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
                    .usage = frame_graph::BufferUsageFlags::SRV,
                    .memory_type = frame_graph::MemoryType::Dynamic,
                    .size = ubo_size,
                });

            builder.write(
                ubo_buffer,
                frame_graph::ResourceUsage::SHADER_COMPUTE |
                    frame_graph::ResourceUsage::SHADER_GRAPHICS);

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

/*

namespace tundra::renderer::hardware {

/////////////////////////////////////////////////////////////////////////////////////////
// Data

///
template <typename BufferType, typename TextureType>
struct Data {
    using IndexBufferArray = std::array<BufferType, config::NUM_INDEX_BUFFERS_IN_FIGHT>;

    BufferType ubo_buffer;

    struct {
        BufferType visible_instances;
        BufferType meshlet_culling_dispatch_args;
    } instance_culling;

    struct {
        BufferType visible_meshlets;
        BufferType visible_meshlets_count;
    } meshlet_culling;

    struct {
        BufferType index_buffer_generator_dispatch_args;
        BufferType index_buffer_generator_meshlet_offsets;
        BufferType draw_meshlets_draw_counts;
        BufferType meshlet_offset;
    } dispatch_args_generator;

    struct {
        IndexBufferArray index_buffers;
        BufferType draw_meshlets_draw_args;
    } index_buffer_generator;

    struct {
        TextureType color_texture;
        TextureType depth_buffer;
    } draw_meshlets;

    [[nodiscard]] Data<rhi::BufferHandle, rhi::TextureHandle> to_rhi(
        const frame_graph::Registry& registry) const noexcept
    {
        Data<rhi::BufferHandle, rhi::TextureHandle>::IndexBufferArray index_buffers;
        for (u32 i = 0; i < config::NUM_INDEX_BUFFERS_IN_FIGHT; ++i) {
            index_buffers[i] = registry.get_buffer(
                index_buffer_generator.index_buffers[i]);
        }

        return Data<rhi::BufferHandle, rhi::TextureHandle> {
            .ubo_buffer = registry.get_buffer(ubo_buffer),
            .instance_culling = {
                .visible_instances = registry.get_buffer(
                    instance_culling.visible_instances),
                .meshlet_culling_dispatch_args = registry.get_buffer(
                    instance_culling.meshlet_culling_dispatch_args),
            },
            .meshlet_culling = {
                .visible_meshlets = registry.get_buffer(
                    meshlet_culling.visible_meshlets),
                .visible_meshlets_count = registry.get_buffer(
                    meshlet_culling.visible_meshlets_count),
            },
            .dispatch_args_generator = {
                .index_buffer_generator_dispatch_args = registry.get_buffer(
                    dispatch_args_generator.index_buffer_generator_dispatch_args),
                .index_buffer_generator_meshlet_offsets = registry.get_buffer(
                    dispatch_args_generator
                        .index_buffer_generator_meshlet_offsets),
                .draw_meshlets_draw_counts = registry.get_buffer(
                    dispatch_args_generator.draw_meshlets_draw_counts),
                .meshlet_offset = registry.get_buffer(
                    dispatch_args_generator.meshlet_offset),
            },
            .index_buffer_generator = {
                .index_buffers = index_buffers,
                .draw_meshlets_draw_args = registry.get_buffer(
                    index_buffer_generator.draw_meshlets_draw_args),
            },
            .draw_meshlets = {
                .color_texture = registry.get_texture(draw_meshlets.color_texture),
                .depth_buffer = registry.get_texture(draw_meshlets.depth_buffer),
            },
        };
    }
};

using FrameGraphData = Data<frame_graph::BufferHandle, frame_graph::TextureHandle>;
using RHIData = Data<rhi::BufferHandle, rhi::TextureHandle>;

/////////////////////////////////////////////////////////////////////////////////////////
// helpers

namespace helpers {

/// 1
void instance_culling_and_lod(
    rhi::IRHIContext* rhi,
    rhi::CommandEncoder& encoder,
    const std::array<math::Vec4, 6>& planes,
    const u32 num_instances,
    const RHIData& rhi_data,
    const rhi::BufferHandle mesh_descriptors,
    const rhi::BufferHandle mesh_instances,
    const rhi::BufferHandle mesh_instance_transforms,
    const ComputePipelinesMap& compute_pipelines) noexcept;

/// 2
void meshlet_culling(
    rhi::IRHIContext* rhi,
    rhi::CommandEncoder& encoder,
    const math::Mat4& world_to_view,
    const std::array<math::Vec4, 6>& planes,
    const math::Vec3& camera_position,
    const u32 max_num_meshlets,
    const RHIData& rhi_data,
    const rhi::BufferHandle mesh_descriptors,
    const rhi::BufferHandle mesh_instance_transforms,
    const ComputePipelinesMap& compute_pipelines) noexcept;

///
void clear_index_buffer_generator_dispatch_args(
    rhi::IRHIContext* rhi,
    rhi::CommandEncoder& encoder,
    const RHIData& rhi_data,
    const ComputePipelinesMap& compute_pipelines) noexcept;

/// 3
void generate_index_buffer_generator_dispatch_args(
    rhi::IRHIContext* rhi,
    rhi::CommandEncoder& encoder,
    const u32 num_index_buffers_in_fight,
    const u32 num_meshlets_per_index_buffer,
    const RHIData& rhi_data,
    const ComputePipelinesMap& compute_pipelines) noexcept;

/// 4
void index_buffer_generator(
    rhi::IRHIContext* rhi,
    rhi::CommandEncoder& encoder,
    const math::Mat4& world_to_clip,
    const RHIData& rhi_data,
    const rhi::BufferHandle mesh_descriptors,
    const rhi::BufferHandle mesh_instance_transforms,
    const ComputePipelinesMap& compute_pipelines) noexcept;

/// 5
void draw_meshlets(
    rhi::IRHIContext* rhi,
    rhi::CommandEncoder& encoder,
    const math::UVec2& view_size,
    const math::Mat4& world_to_view,
    const math::Mat4& view_to_clip,
    const rhi::RenderPass& render_pass,
    const RHIData& rhi_data,
    const rhi::BufferHandle gpu_mesh_instance_transforms,
    const GraphicsPipelinesMap& graphics_pipelines) noexcept;

} // namespace helpers

/////////////////////////////////////////////////////////////////////////////////////////
// render

///
[[nodiscard]] RenderOutput render(
    frame_graph::FrameGraph& fg, const RenderInput& input) noexcept
{
    const usize num_instances = input.mesh_instances.size();
    tndr_assert(num_instances < config::MAX_INSTANCE_COUNT, "Too many instances");

    const u32 max_num_meshlets = [&] {
        u32 count = 0;
        for (const MeshInstance& mesh_instance : input.mesh_instances) {
            count += input.mesh_descriptors[mesh_instance.mesh_descriptor_index]
                         .meshlet_count;
        }

        return count;
    }();
    tndr_assert(
        max_num_meshlets <= config::MAX_VISIBLE_MESHLETS_COUNT, "Too many meshlets");

    const math::Mat4 frustum = input.view_to_clip * input.world_to_view;
    const math::Mat4 frustum_t = math::transpose(frustum);

    const std::array<math::Vec4, 6> planes {
        math::normalize(-frustum_t[3] - frustum_t[0]), //
        math::normalize(-frustum_t[3] + frustum_t[0]), //
        math::normalize(-frustum_t[3] - frustum_t[1]), //
        math::normalize(-frustum_t[3] + frustum_t[1]), //
        math::normalize(-frustum_t[3] - frustum_t[2]), //
        math::normalize(-frustum_t[3] + frustum_t[2]),
    };

    const FrameGraphData data = fg.add_render_pass(
        frame_graph::QueueType::Graphics,
        "render",
        [&](frame_graph::Builder& builder, frame_graph::RenderPass& render_pass) {
            FrameGraphData data {};

            data.ubo_buffer = builder.create_buffer(
                "ubo_buffer",
                frame_graph::BufferCreateInfo {
                    .usage = frame_graph::BufferUsageFlags::SRV,
                    .memory_type = frame_graph::MemoryType::Dynamic,
                    .size = ubo::TOTAL_UBO_SIZE,
                });

            /////////////////////////////////////////////////////////////////////////////////////////
            // instance_culling

            data.instance_culling.visible_instances = builder.create_buffer(
                "instance_culling.visible_instances",
                frame_graph::BufferCreateInfo {
                    .usage = frame_graph::BufferUsageFlags::SRV |
                             frame_graph::BufferUsageFlags::UAV,
                    .memory_type = frame_graph::MemoryType::GPU,
                    .size = sizeof(shader::VisibleMeshInstance) * num_instances,
                });
            builder.write(
                data.instance_culling.visible_instances,
                frame_graph::ResourceUsage::SHADER_COMPUTE);

            data.instance_culling.meshlet_culling_dispatch_args = builder.create_buffer(
                "instance_culling.meshlet_culling_dispatch_args",
                frame_graph::BufferCreateInfo {
                    .usage = frame_graph::BufferUsageFlags::SRV |
                             frame_graph::BufferUsageFlags::UAV |
                             frame_graph::BufferUsageFlags::INDIRECT_BUFFER,
                    .memory_type = frame_graph::MemoryType::GPU,
                    .size = sizeof(rhi::DispatchIndirectCommand),
                });
            builder.write(
                data.instance_culling.meshlet_culling_dispatch_args,
                frame_graph::ResourceUsage::SHADER_COMPUTE);

            /////////////////////////////////////////////////////////////////////////////////////////
            // meshlet_culling

            data.meshlet_culling.visible_meshlets = builder.create_buffer(
                "meshlet_culling.visible_meshlets",
                frame_graph::BufferCreateInfo {
                    .usage = frame_graph::BufferUsageFlags::SRV |
                             frame_graph::BufferUsageFlags::UAV,
                    .memory_type = frame_graph::MemoryType::GPU,
                    .size = sizeof(shader::VisibleMeshlet) * max_num_meshlets,
                });
            builder.write(
                data.meshlet_culling.visible_meshlets,
                frame_graph::ResourceUsage::SHADER_COMPUTE);

            data.meshlet_culling.visible_meshlets_count = builder.create_buffer(
                "meshlet_culling.num_visible_meshlets",
                frame_graph::BufferCreateInfo {
                    .usage = frame_graph::BufferUsageFlags::SRV |
                             frame_graph::BufferUsageFlags::UAV,
                    .memory_type = frame_graph::MemoryType::GPU,
                    .size = sizeof(u32),
                });
            builder.write(
                data.meshlet_culling.visible_meshlets_count,
                frame_graph::ResourceUsage::SHADER_COMPUTE);

            /////////////////////////////////////////////////////////////////////////////////////////
            // dispatch_args_generator

            data.dispatch_args_generator
                .index_buffer_generator_dispatch_args = builder.create_buffer(
                "dispatch_args_generator.index_buffer_generator_dispatch_args",
                frame_graph::BufferCreateInfo {
                    .usage = frame_graph::BufferUsageFlags::SRV |
                             frame_graph::BufferUsageFlags::UAV |
                             frame_graph::BufferUsageFlags::INDIRECT_BUFFER,
                    .memory_type = frame_graph::MemoryType::GPU,
                    .size = sizeof(rhi::DispatchIndirectCommand) *
                            config::NUM_INDEX_BUFFERS_IN_FIGHT,
                });
            builder.write(
                data.dispatch_args_generator.index_buffer_generator_dispatch_args,
                frame_graph::ResourceUsage::SHADER_COMPUTE);

            data.dispatch_args_generator
                .index_buffer_generator_meshlet_offsets = builder.create_buffer(
                "dispatch_args_generator.index_buffer_generator_meshlet_offsets_uav",
                frame_graph::BufferCreateInfo {
                    .usage = frame_graph::BufferUsageFlags::SRV |
                             frame_graph::BufferUsageFlags::UAV |
                             frame_graph::BufferUsageFlags::INDIRECT_BUFFER,
                    .memory_type = frame_graph::MemoryType::GPU,
                    .size = sizeof(u32) * config::NUM_INDEX_BUFFERS_IN_FIGHT,
                });
            builder.write(
                data.dispatch_args_generator.index_buffer_generator_meshlet_offsets,
                frame_graph::ResourceUsage::SHADER_COMPUTE);

            data.dispatch_args_generator.draw_meshlets_draw_counts = builder.create_buffer(
                "dispatch_args_generator.draw_meshlets_draw_counts",
                frame_graph::BufferCreateInfo {
                    .usage = frame_graph::BufferUsageFlags::SRV |
                             frame_graph::BufferUsageFlags::UAV |
                             frame_graph::BufferUsageFlags::INDIRECT_BUFFER,
                    .memory_type = frame_graph::MemoryType::GPU,
                    .size = sizeof(u32) * config::NUM_INDEX_BUFFERS_IN_FIGHT,
                });
            builder.write(
                data.dispatch_args_generator.draw_meshlets_draw_counts,
                frame_graph::ResourceUsage::SHADER_COMPUTE);

            data.dispatch_args_generator.meshlet_offset = builder.create_buffer(
                "dispatch_args_generator.meshlet_offset",
                frame_graph::BufferCreateInfo {
                    .usage = frame_graph::BufferUsageFlags::SRV |
                             frame_graph::BufferUsageFlags::UAV,
                    .memory_type = frame_graph::MemoryType::GPU,
                    .size = sizeof(u32),
                });
            builder.write(
                data.dispatch_args_generator.meshlet_offset,
                frame_graph::ResourceUsage::SHADER_COMPUTE);

            /////////////////////////////////////////////////////////////////////////////////////////
            // index_buffer_generator

            for (usize i = 0; i < config::NUM_INDEX_BUFFERS_IN_FIGHT; ++i) {
                frame_graph::BufferHandle& index_buffer = data.index_buffer_generator
                                                              .index_buffers[i];
                index_buffer = builder.create_buffer(
                    fmt::format("index_buffer_generator.index_buffers {}", i),
                    frame_graph::BufferCreateInfo {
                        .usage = frame_graph::BufferUsageFlags::SRV |
                                 frame_graph::BufferUsageFlags::UAV |
                                 frame_graph::BufferUsageFlags::INDEX_BUFFER,
                        .memory_type = frame_graph::MemoryType::GPU,
                        .size = sizeof(u32) * config::INDEX_BUFFER_BATCH_SIZE,
                    });
                builder.write(index_buffer, frame_graph::ResourceUsage::SHADER_COMPUTE);
            }

            data.index_buffer_generator.draw_meshlets_draw_args = builder.create_buffer(
                "index_buffer_generator.draw_meshlets_draw_args",
                frame_graph::BufferCreateInfo {
                    .usage = frame_graph::BufferUsageFlags::SRV |
                             frame_graph::BufferUsageFlags::UAV |
                             frame_graph::BufferUsageFlags::INDIRECT_BUFFER,
                    .memory_type = frame_graph::MemoryType::GPU,
                    .size = sizeof(rhi::DrawIndexedIndirectCommand) *
                            config::NUM_INDEX_BUFFERS_IN_FIGHT,
                });
            builder.write(
                data.index_buffer_generator.draw_meshlets_draw_args,
                frame_graph::ResourceUsage::SHADER_COMPUTE);

            /////////////////////////////////////////////////////////////////////////////////////////
            // draw_meshlets

            data.draw_meshlets.color_texture = builder.write(
                builder.create_texture(
                    "draw_meshlets.color_texture",
                    frame_graph::TextureCreateInfo {
                        .kind =
                            frame_graph::TextureKind::Texture2D {
                                .width = input.view_size.x,
                                .height = input.view_size.y,
                            },
                        .memory_type = frame_graph::MemoryType::GPU,
                        .format = frame_graph::TextureFormat::R8_G8_B8_A8_UNORM,
                        .usage = frame_graph::TextureUsageFlags::COLOR_ATTACHMENT |
                                 frame_graph::TextureUsageFlags::PRESENT,
                        .tiling = frame_graph::TextureTiling::Optimal,
                    }),
                frame_graph::ResourceUsage::COLOR_ATTACHMENT);

            data.draw_meshlets.depth_buffer = builder.write(
                builder.create_texture(
                    "draw_meshlets.depth_buffer",
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
                frame_graph::ResourceUsage::DEPTH_STENCIL_ATTACHMENT);

            render_pass.color_attachments.push_back(frame_graph::ColorAttachment {
                .ops = frame_graph::AttachmentOps::INIT,
                .texture = data.draw_meshlets.color_texture,
                .clear_value =
                    math::Vec4 {
                        0.0,
                        0.0,
                        0.0,
                        0.0,
                    },
            });
            render_pass.depth_stencil_attachment = frame_graph::DepthStencilAttachment {
                .ops = frame_graph::AttachmentOps::INIT,
                .texture = data.draw_meshlets.depth_buffer,
                .clear_value =
                    rhi::ClearDepthStencil {
                        .depth = 0.0f,
                        .stencil = 0,
                    },
            };

            return data;
        },
        [=](rhi::IRHIContext* rhi,
            const frame_graph::Registry& registry,
            rhi::CommandEncoder& encoder,
            const FrameGraphData& data,
            const rhi::RenderPass& render_pass) {
            ///
            const RHIData rhi_data = data.to_rhi(registry);

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

            encoder.set_culling_mode(rhi::CullingMode::None);

            encoder.begin_render_pass(
                rhi::Rect {
                    .offset = { 0, 0 },
                    .extent = input.view_size,
                },
                render_pass);
            encoder.end_render_pass();

            ///////////////////////////////////////////////////////////////
            // Dispatch instance culling
            helpers::instance_culling_and_lod(
                rhi,
                encoder,
                planes,
                static_cast<u32>(num_instances),
                rhi_data,
                input.gpu_mesh_descriptors,
                input.gpu_mesh_instances,
                input.gpu_mesh_instance_transforms,
                input.compute_pipelines);

            ///////////////////////////////////////////////////////////////
            // Dispatch meshlet culling
            helpers::meshlet_culling(
                rhi,
                encoder,
                input.world_to_view,
                planes,
                input.camera_position,
                config::MAX_VISIBLE_MESHLETS_COUNT,
                rhi_data,
                input.gpu_mesh_descriptors,
                input.gpu_mesh_instance_transforms,
                input.compute_pipelines);

            ///////////////////////////////////////////////////////////////
            // Clear index buffer generator dispatch args
            helpers::clear_index_buffer_generator_dispatch_args(
                rhi, encoder, rhi_data, input.compute_pipelines);

            static constexpr u32 NUM_MESHLETS_PER_ITERATION =
                config::NUM_MESHLETS_PER_INDEX_BUFFER *
                config::NUM_INDEX_BUFFERS_IN_FIGHT;
            const u32 num_iterations = (max_num_meshlets +
                                        (NUM_MESHLETS_PER_ITERATION - 1)) /
                                       NUM_MESHLETS_PER_ITERATION;

            for (u32 i = 0; i < num_iterations; ++i) {
                ///////////////////////////////////////////////////////////////
                // Generate 4 meshlet buffer ranges for index buffer generator
                helpers::generate_index_buffer_generator_dispatch_args(
                    rhi,
                    encoder,
                    config::NUM_INDEX_BUFFERS_IN_FIGHT,
                    config::NUM_MESHLETS_PER_INDEX_BUFFER,
                    rhi_data,
                    input.compute_pipelines);

                ///////////////////////////////////////////////////////////////
                // Generate index buffers
                helpers::index_buffer_generator(
                    rhi,
                    encoder,
                    frustum,
                    rhi_data,
                    input.gpu_mesh_descriptors,
                    input.gpu_mesh_instance_transforms,
                    input.compute_pipelines);

                ///////////////////////////////////////////////////////////////
                // Draw meshlets
                helpers::draw_meshlets(
                    rhi,
                    encoder,
                    input.view_size,
                    input.world_to_view,
                    input.view_to_clip,
                    render_pass,
                    rhi_data,
                    input.gpu_mesh_instance_transforms,
                    input.graphics_pipelines);
            }
        });

    return RenderOutput {
        .color_output = data.draw_meshlets.color_texture,
    };
}

/////////////////////////////////////////////////////////////////////////////////////////
// helpers

namespace helpers {

/// 1
void instance_culling_and_lod(
    rhi::IRHIContext* rhi,
    rhi::CommandEncoder& encoder,
    const std::array<math::Vec4, 6>& planes,
    const u32 num_instances,
    const RHIData& rhi_data,
    const rhi::BufferHandle mesh_descriptors,
    const rhi::BufferHandle mesh_instances,
    const rhi::BufferHandle mesh_instance_transforms,
    const ComputePipelinesMap& compute_pipelines) noexcept
{
    const ubo::InstanceCullingUBO ubo {
        .frustum_planes = planes,
        .num_instances = num_instances,
        .in_ = {
            .mesh_descriptors_srv = mesh_descriptors.get_srv(),
            .mesh_instances_srv = mesh_instances.get_srv(),
            .mesh_instance_transforms_srv = mesh_instance_transforms.get_srv(),
        },
        .out_ = {
            .visible_mesh_instances_uav = rhi_data.instance_culling.visible_instances.get_uav(),
            .meshlet_culling_dispatch_args_uav = rhi_data.instance_culling.meshlet_culling_dispatch_args.get_uav(),

        },
    };

    rhi->update_buffer(
        rhi_data.ubo_buffer,
        {
            rhi::BufferUpdateRegion {
                .src = core::as_byte_span(ubo),
                .dst_offset = ubo::INSTANCE_CULLING_UBO_OFFSET,
            },
        });

    encoder.begin_region(
        "instance_culling_and_lod",
        math::Vec4 {
            0.25f,
            1.f,
            1.f,
            1.f,
        });

    encoder.push_constants(rhi_data.ubo_buffer, ubo::INSTANCE_CULLING_UBO_OFFSET);
    encoder.dispatch(
        get_pipeline(
            pipelines::INSTANCE_CULLING_AND_LOD_PIPELINE_NAME, compute_pipelines),
        rhi::CommandEncoder::get_group_count(num_instances, 128),
        1,
        1);

    encoder.end_region();
}

/// 2
void meshlet_culling(
    rhi::IRHIContext* rhi,
    rhi::CommandEncoder& encoder,
    const math::Mat4& world_to_view,
    const std::array<math::Vec4, 6>& planes,
    const math::Vec3& camera_position,
    const u32 max_num_meshlets,
    const RHIData& rhi_data,
    const rhi::BufferHandle mesh_descriptors,
    const rhi::BufferHandle mesh_instance_transforms,
    const ComputePipelinesMap& compute_pipelines) noexcept
{
    const ubo::MeshletCullingUBO ubo {
        .world_to_view = world_to_view,
        .frustum_planes = planes,
        .camera_position = camera_position,
        .max_num_meshlets = max_num_meshlets,
        .in_ = {
            .visible_mesh_instances_srv = rhi_data.instance_culling.visible_instances.get_srv(),
            .mesh_descriptors_srv = mesh_descriptors.get_srv(),
            .mesh_instance_transforms_srv = mesh_instance_transforms.get_srv(),
        },
        .out_ = {
            .visible_meshlets_uav = rhi_data.meshlet_culling.visible_meshlets.get_uav(),
            .visible_meshlets_count_uav = rhi_data.meshlet_culling.visible_meshlets_count.get_uav(),            
        },
    };

    rhi->update_buffer(
        rhi_data.ubo_buffer,
        {
            rhi::BufferUpdateRegion {
                .src = core::as_byte_span(ubo),
                .dst_offset = ubo::MESHLET_CULLING_UBO_OFFSET,
            },
        });

    encoder.begin_region(
        "meshlet_culling",
        math::Vec4 {
            0.5f,
            0.5f,
            1.f,
            1.f,
        });

    encoder.global_barrier(rhi::GlobalBarrier {
        .previous_access = rhi::AccessFlags::GENERAL,
        .next_access = rhi::AccessFlags::GENERAL,
        // .previous_access = rhi::AccessFlags::SRV_COMPUTE | //
        //                    rhi::AccessFlags::UAV_COMPUTE,
        // .next_access = rhi::AccessFlags::SRV_COMPUTE | //
        //                rhi::AccessFlags::UAV_COMPUTE | //
        //                rhi::AccessFlags::INDIRECT_BUFFER,
    });

    encoder.push_constants(rhi_data.ubo_buffer, ubo::MESHLET_CULLING_UBO_OFFSET);
    encoder.dispatch_indirect(
        get_pipeline(pipelines::MESHLET_CULLING_NAME, compute_pipelines),
        rhi_data.instance_culling.meshlet_culling_dispatch_args,
        0);

    encoder.end_region();
}

///
void clear_index_buffer_generator_dispatch_args(
    rhi::IRHIContext* rhi,
    rhi::CommandEncoder& encoder,
    const RHIData& rhi_data,
    const ComputePipelinesMap& compute_pipelines) noexcept
{
    const ubo::ClearIndexBufferGeneratorDispatchArgs ubo {
        .out_ = { 
            .meshlet_offset_uav = rhi_data.dispatch_args_generator.meshlet_offset.get_uav(),
        },
    };

    rhi->update_buffer(
        rhi_data.ubo_buffer,
        {
            rhi::BufferUpdateRegion {
                .src = core::as_byte_span(ubo),
                .dst_offset = ubo::CLEAR_INDEX_BUFFER_GENERATOR_DISPATCH_ARGS_OFFSET,
            },
        });

    encoder.begin_region(
        "clear_index_buffer_generator_dispatch_args",
        {
            0.35f,
            0.95f,
            0.1f,
            1.f,
        });
    encoder.global_barrier(rhi::GlobalBarrier {
        .previous_access = rhi::AccessFlags::GENERAL,
        .next_access = rhi::AccessFlags::GENERAL,
        // .previous_access = rhi::AccessFlags::SRV_COMPUTE |     //
        //                    rhi::AccessFlags::UAV_COMPUTE |     //
        //                    rhi::AccessFlags::INDIRECT_BUFFER | //
        //                    rhi::AccessFlags::INDEX_BUFFER,
        // .next_access = rhi::AccessFlags::SRV_COMPUTE | //
        //                rhi::AccessFlags::UAV_COMPUTE | //
        //                rhi::AccessFlags::INDIRECT_BUFFER,
    });

    encoder.push_constants(
        rhi_data.ubo_buffer, ubo::CLEAR_INDEX_BUFFER_GENERATOR_DISPATCH_ARGS_OFFSET);
    encoder.dispatch(
        get_pipeline(
            pipelines::CLEAR_INDEX_BUFFER_GENERATOR_DISPATCH_ARGS_NAME,
            compute_pipelines),
        1,
        1,
        1);

    encoder.end_region();
}

/// 3
void generate_index_buffer_generator_dispatch_args(
    rhi::IRHIContext* rhi,
    rhi::CommandEncoder& encoder,
    const u32 num_index_buffers_in_fight,
    const u32 num_meshlets_per_index_buffer,
    const RHIData& rhi_data,
    const ComputePipelinesMap& compute_pipelines) noexcept
{
    const ubo::GenerateDispatchArgsUBO ubo {
        .num_index_buffers_in_fight = num_index_buffers_in_fight,
        .num_meshlets_per_index_buffer = num_meshlets_per_index_buffer,
        .in_ = { 
            .num_visible_meshlets_srv = rhi_data.meshlet_culling.visible_meshlets_count.get_srv(),
         },
        .out_ = { 
            .index_buffer_generator_dispatch_args_uav = rhi_data.dispatch_args_generator.index_buffer_generator_dispatch_args.get_uav(),
            .index_buffer_generator_meshlet_offsets_uav = rhi_data.dispatch_args_generator.index_buffer_generator_meshlet_offsets.get_uav(), 
            .draw_meshlets_draw_counts_uav = rhi_data.dispatch_args_generator.draw_meshlets_draw_counts.get_uav(),
            .draw_meshlets_draw_args_uav = rhi_data.index_buffer_generator.draw_meshlets_draw_args.get_uav(),
        },
        .inout_ = {
            .meshlet_offset_uav = rhi_data.dispatch_args_generator.meshlet_offset.get_uav(),
        },
    };

    rhi->update_buffer(
        rhi_data.ubo_buffer,
        {
            rhi::BufferUpdateRegion {
                .src = core::as_byte_span(ubo),
                .dst_offset = ubo::GENERATE_DISPATCH_ARGS_UBO_OFFSET,
            },
        });

    encoder.begin_region(
        "generate_index_buffer_generator_dispatch_args",
        {
            0.25f,
            0.75f,
            0.5f,
            1.f,
        });
    encoder.global_barrier(rhi::GlobalBarrier {
        .previous_access = rhi::AccessFlags::GENERAL,
        .next_access = rhi::AccessFlags::GENERAL,
        // .previous_access = rhi::AccessFlags::SRV_COMPUTE |     //
        //                    rhi::AccessFlags::UAV_COMPUTE |     //
        //                    rhi::AccessFlags::INDIRECT_BUFFER | //
        //                    rhi::AccessFlags::INDEX_BUFFER,
        // .next_access = rhi::AccessFlags::SRV_COMPUTE | //
        //                rhi::AccessFlags::UAV_COMPUTE | //
        //                rhi::AccessFlags::INDIRECT_BUFFER,
    });

    encoder.push_constants(rhi_data.ubo_buffer, ubo::GENERATE_DISPATCH_ARGS_UBO_OFFSET);
    encoder.dispatch(
        get_pipeline(
            pipelines::GENERATE_INDEX_BUFFER_GENERATOR_DISPATCH_ARGS_NAME,
            compute_pipelines),
        1,
        1,
        1);

    encoder.end_region();
}

/// 4
void index_buffer_generator(
    rhi::IRHIContext* rhi,
    rhi::CommandEncoder& encoder,
    const math::Mat4& world_to_clip,
    const RHIData& rhi_data,
    const rhi::BufferHandle mesh_descriptors,
    const rhi::BufferHandle mesh_instance_transforms,
    const ComputePipelinesMap& compute_pipelines) noexcept
{
    encoder.begin_region(
        "index_buffer_generator",
        {
            0.75f,
            0.75f,
            0.25f,
            1.f,
        });

    encoder.global_barrier(rhi::GlobalBarrier {
        .previous_access = rhi::AccessFlags::GENERAL,
        .next_access = rhi::AccessFlags::GENERAL,
        // .previous_access = rhi::AccessFlags::SRV_COMPUTE |     //
        //                    rhi::AccessFlags::UAV_COMPUTE |     //
        //                    rhi::AccessFlags::INDIRECT_BUFFER | //
        //                    rhi::AccessFlags::INDEX_BUFFER |
        //                    rhi::AccessFlags::VERTEX_BUFFER,
        // .next_access = rhi::AccessFlags::SRV_COMPUTE | //
        //                rhi::AccessFlags::UAV_COMPUTE | //
        //                rhi::AccessFlags::INDIRECT_BUFFER,
    });

    for (u32 i = 0; i < config::NUM_INDEX_BUFFERS_IN_FIGHT; ++i) {
        // for (u32 i = 1; i < 2; ++i) {
        const u64 dst_offset = ubo::INDEX_BUFFER_GENERATOR_UBO_OFFSET +
                               (sizeof(ubo::IndexBufferGeneratorUBO) * i);
        const rhi::BufferHandle index_buffer = rhi_data.index_buffer_generator
                                                   .index_buffers[i];

        const ubo::IndexBufferGeneratorUBO ubo {
            .world_to_clip = world_to_clip,
            .max_num_indices = config::INDEX_BUFFER_BATCH_SIZE,
            .generator_index = i,
            .in_ = {
                .mesh_instance_transforms_srv = mesh_instance_transforms.get_srv(),
                .visible_meshlets_srv = rhi_data.meshlet_culling.visible_meshlets.get_srv(),
                .mesh_descriptors_srv = mesh_descriptors.get_srv(),
                .meshlet_offsets_srv = rhi_data.dispatch_args_generator.index_buffer_generator_meshlet_offsets.get_srv(),
            },
            .out_ = {
                .index_buffer_uav = index_buffer.get_uav(),
                .draw_meshlets_draw_args_uav = rhi_data.index_buffer_generator.draw_meshlets_draw_args.get_uav(),
            },
        };

        rhi->update_buffer(
            rhi_data.ubo_buffer,
            {
                rhi::BufferUpdateRegion {
                    .src = core::as_byte_span(ubo),
                    .dst_offset = dst_offset,
                },
            });

        // encoder.global_barrier(rhi::GlobalBarrier {
        //     .previous_access = rhi::AccessFlags::GENERAL,
        //     .next_access = rhi::AccessFlags::GENERAL,
        // });

        encoder.push_constants(rhi_data.ubo_buffer, static_cast<u32>(dst_offset));
        encoder.dispatch_indirect(
            get_pipeline(pipelines::INDEX_BUFFER_GENERATOR_NAME, compute_pipelines),
            rhi_data.dispatch_args_generator.index_buffer_generator_dispatch_args,
            i * sizeof(rhi::DispatchIndirectCommand));
    }

    encoder.end_region();
}

/// 5
void draw_meshlets(
    rhi::IRHIContext* rhi,
    rhi::CommandEncoder& encoder,
    const math::UVec2& view_size,
    const math::Mat4& world_to_view,
    const math::Mat4& view_to_clip,
    const rhi::RenderPass& render_pass,
    const RHIData& rhi_data,
    const rhi::BufferHandle gpu_mesh_instance_transforms,
    const GraphicsPipelinesMap& graphics_pipelines) noexcept
{
    encoder.begin_region(
        "draw_meshlets",
        {
            1.0f,
            0.5f,
            0.25f,
            1.f,
        });

    encoder.global_barrier(rhi::GlobalBarrier {
        .previous_access = rhi::AccessFlags::GENERAL,
        .next_access = rhi::AccessFlags::GENERAL,
        // .previous_access = rhi::AccessFlags::SRV_COMPUTE | //
        //                    rhi::AccessFlags::UAV_COMPUTE | //
        //                    rhi::AccessFlags::INDIRECT_BUFFER,
        // .next_access = rhi::AccessFlags::INDEX_BUFFER |    //
        //                rhi::AccessFlags::INDIRECT_BUFFER | //
        //                rhi::AccessFlags::INDEX_BUFFER |    //
        //                rhi::AccessFlags::INDIRECT_BUFFER,
    });
    encoder.set_viewport(rhi::Viewport {
        .rect =
            rhi::Rect {
                .offset = { 0, 0 },
                .extent = view_size,
            },
    });

    encoder.set_scissor(rhi::Scissor {
        .offset = { 0, 0 },
        .extent = view_size,
    });

    encoder.set_culling_mode(rhi::CullingMode::Back);
    rhi::RenderPass render_pass_2 = render_pass;
    render_pass_2.color_attachments[0].ops = rhi::AttachmentOps::PRESERVE;
    render_pass_2.depth_stencil_attachment->ops = rhi::AttachmentOps::PRESERVE;
    encoder.begin_render_pass(
        rhi::Rect {
            .offset = { 0, 0 },
            .extent = view_size,
        },
        render_pass_2);

    encoder.bind_graphics_pipeline(
        get_pipeline(pipelines::VISIBILITY_BUFFER_PASS, graphics_pipelines));
    for (u32 i = 0; i < config::NUM_INDEX_BUFFERS_IN_FIGHT; ++i) {
        //for (u32 i = 2; i < 3; ++i) {
        const u64 dst_offset = ubo::VISIBILITY_BUFFER_PASS_UBO_OFFSET +
                               (sizeof(ubo::VisibilityBufferPassUBO) * i);

        const ubo::VisibilityBufferPassUBO ubo {
            .world_to_view = world_to_view,
            .view_to_clip = view_to_clip,
            .in_ = {
                .visible_meshlets_srv = rhi_data.meshlet_culling.visible_meshlets.get_srv(),
                .mesh_instance_transforms_srv = gpu_mesh_instance_transforms.get_srv(),
            },
        };

        rhi->update_buffer(
            rhi_data.ubo_buffer,
            {
                rhi::BufferUpdateRegion {
                    .src = core::as_byte_span(ubo),
                    .dst_offset = dst_offset,
                },
            });
        encoder.push_constants(rhi_data.ubo_buffer, static_cast<u32>(dst_offset));
        encoder.bind_index_buffer(
            rhi_data.index_buffer_generator.index_buffers[i], 0, rhi::IndexType::U32);

        encoder.draw_indexed_indirect_count(
            rhi_data.index_buffer_generator.draw_meshlets_draw_args,
            sizeof(rhi::DrawIndexedIndirectCommand) * i,
            rhi_data.dispatch_args_generator.draw_meshlets_draw_counts,
            sizeof(u32) * i,
            1,
            sizeof(rhi::DrawIndexedIndirectCommand));
    }

    encoder.end_render_pass();
    encoder.end_region();
}

} // namespace helpers


} // namespace tundra::renderer::hardware

*/
