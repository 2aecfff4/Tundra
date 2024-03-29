#include "renderer/common/culling/meshlet_culling.h"
#include "pipelines.h"
#include "renderer/helpers.h"
#include "rhi/rhi_context.h"

namespace tundra::renderer::common::culling {

namespace ubo {

///
struct MeshletCullingInitUbo {
    u32 visible_meshlets_count_uav = config::INVALID_SHADER_HANDLE;
};

///
struct MeshletCullingUBO {
    math::Mat4 projection = math::Mat4 {};
    math::Mat4 world_to_view = math::Mat4 {};
    std::array<math::Vec4, config::NUM_PLANES> frustum_planes = {};
    math::Vec3 camera_position = {};
    float z_near = 0;
    u32 max_meshlet_count = 0;

    struct {
        u32 visible_mesh_instances_srv = config::INVALID_SHADER_HANDLE;
        u32 mesh_descriptors_srv = config::INVALID_SHADER_HANDLE;
        u32 mesh_instance_transforms_srv = config::INVALID_SHADER_HANDLE;
        u32 previous_frame_depth_texture_srv = config::INVALID_SHADER_HANDLE;
        u32 depth_texture_sampler = config::INVALID_SHADER_HANDLE;
    } in_;

    struct {
        u32 visible_meshlets_uav = config::INVALID_SHADER_HANDLE;
        u32 visible_meshlets_count_uav = config::INVALID_SHADER_HANDLE;
    } out_;
};

} // namespace ubo

namespace config {

inline constexpr u32 MAX_VISIBLE_MESHLETS_COUNT = (1u << 20u) * 4u;

} // namespace config

///
MeshletCullingOutput meshlet_culling(
    frame_graph::FrameGraph& fg, const MeshletCullingInput& input) noexcept
{
    tndr_assert(
        input.max_meshlet_count < config::MAX_VISIBLE_MESHLETS_COUNT,
        "Too many meshlets.");

    struct Data {
        core::SharedPtr<UboBuffer> ubo_buffer;

        frame_graph::BufferHandle visible_instances;
        frame_graph::BufferHandle meshlet_culling_dispatch_args;

        frame_graph::BufferHandle visible_meshlets;
        frame_graph::BufferHandle visible_meshlets_count;
    };

    const Data data = fg.add_pass(
        frame_graph::QueueType::Graphics,
        "meshlet_culling",
        [&](frame_graph::Builder& builder) {
            Data data {};

            data.ubo_buffer = input.ubo_buffer;

            data.visible_instances = builder.read(
                input.visible_instances,
                frame_graph::BufferResourceUsage::COMPUTE_STORAGE_BUFFER);

            data.meshlet_culling_dispatch_args = builder.read(
                input.meshlet_culling_dispatch_args,
                frame_graph::BufferResourceUsage::INDIRECT_BUFFER);

            data.visible_meshlets = builder.create_buffer(
                "meshlet_culling.visible_meshlets",
                frame_graph::BufferCreateInfo {
                    .usage = frame_graph::BufferUsageFlags::STORAGE_BUFFER,
                    .memory_type = frame_graph::MemoryType::GPU,
                    .size = sizeof(shader::VisibleMeshlet) * input.max_meshlet_count,
                });
            builder.write(
                data.visible_meshlets,
                frame_graph::BufferResourceUsage::COMPUTE_STORAGE_BUFFER);

            data.visible_meshlets_count = builder.create_buffer(
                "meshlet_culling.visible_meshlets_count",
                frame_graph::BufferCreateInfo {
                    .usage = frame_graph::BufferUsageFlags::STORAGE_BUFFER,
                    .memory_type = frame_graph::MemoryType::GPU,
                    .size = sizeof(u32),
                });
            builder.write(
                data.visible_meshlets_count,
                frame_graph::BufferResourceUsage::COMPUTE_STORAGE_BUFFER);

            return data;
        },
        [=](rhi::IRHIContext* rhi,
            const frame_graph::Registry& registry,
            rhi::CommandEncoder& encoder,
            const Data& data) {
            //
            const rhi::BufferHandle ubo_buffer = registry.get_buffer(
                data.ubo_buffer->buffer());
            const rhi::BufferHandle visible_instances = registry.get_buffer(
                data.visible_instances);
            const rhi::BufferHandle meshlet_culling_dispatch_args = registry.get_buffer(
                data.meshlet_culling_dispatch_args);
            const rhi::BufferHandle visible_meshlets = registry.get_buffer(
                data.visible_meshlets);
            const rhi::BufferHandle visible_meshlets_count = registry.get_buffer(
                data.visible_meshlets_count);

            {
                const ubo::MeshletCullingInitUbo ubo {
                    .visible_meshlets_count_uav = visible_meshlets_count.get_uav(),
                };

                const auto ubo_ref = data.ubo_buffer
                                         ->allocate<ubo::MeshletCullingInitUbo>();

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
                        pipelines::common::culling::MESHLET_CULLING_INIT_NAME,
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
                const ubo::MeshletCullingUBO ubo {
                .world_to_view = input.world_to_view,
                .frustum_planes = input.frustum_planes,
                .camera_position = input.camera_position,
                .max_meshlet_count = input.max_meshlet_count,
                .in_ = {
                    .visible_mesh_instances_srv = visible_instances.get_srv(),
                    .mesh_descriptors_srv = input.mesh_descriptors.get_srv(),
                    .mesh_instance_transforms_srv = input.mesh_instance_transforms.get_srv(),
                },
                .out_ = {
                    .visible_meshlets_uav = visible_meshlets.get_uav(),
                    .visible_meshlets_count_uav = visible_meshlets_count.get_uav(),            
                },
            };

                const auto ubo_ref = data.ubo_buffer->allocate<ubo::MeshletCullingUBO>();

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
                        pipelines::common::culling::MESHLET_CULLING_NAME,
                        input.compute_pipelines),
                    meshlet_culling_dispatch_args,
                    0);
            }
        });

    return MeshletCullingOutput {
        .visible_meshlets = data.visible_meshlets,
        .visible_meshlets_count = data.visible_meshlets_count,
    };
}

} // namespace tundra::renderer::common::culling
