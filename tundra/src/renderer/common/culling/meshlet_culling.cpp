#include "renderer/common/culling/meshlet_culling.h"
#include "rhi/commands/dispatch_indirect.h"
#include "rhi/rhi_context.h"

namespace tundra::renderer::common::culling {

namespace ubo {

///
struct MeshletCullingUBO {
    math::Mat4 projection;
    math::Mat4 world_to_view;
    std::array<math::Vec4, 6> frustum_planes;
    float z_near;
    u32 max_meshlet_count;

    struct {
        u32 visible_mesh_instances_srv;
        u32 mesh_descriptors_srv;
        u32 mesh_instance_transforms_srv;
        u32 previous_frame_depth_texture_srv = 0xFF'FF'FF'FF;
        u32 depth_texture_sampler = 0xFF'FF'FF'FF;
    } in_;

    struct {
        u32 visible_meshlets_uav;
        u32 visible_meshlets_count_uav;
    } out_;
};

} // namespace ubo

namespace config {

inline constexpr u32 MAX_VISIBLE_MESHLETS_COUNT = (1u << 20u) * 4u;

} // namespace config

///
[[nodiscard]] MeshletCullingOutput meshlet_culling(
    frame_graph::FrameGraph& fg, const MeshletCullingInput& input) noexcept
{
    tndr_assert(
        input.max_meshlet_count < config::MAX_VISIBLE_MESHLETS_COUNT,
        "Too many meshlets.");

    const u64 ubo_buffer_offset = input.ubo_buffer_offset +
                                  sizeof(ubo::MeshletCullingUBO);

    struct Data {
        frame_graph::BufferHandle ubo_buffer;
        u64 ubo_buffer_offset = 0;

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
            data.ubo_buffer_offset = input.ubo_buffer_offset;

            data.visible_instances = builder.read(
                input.visible_instances, frame_graph::ResourceUsage::SHADER_COMPUTE);

            data.meshlet_culling_dispatch_args = builder.read(
                input.meshlet_culling_dispatch_args,
                frame_graph::ResourceUsage::INDIRECT_BUFFER);

            data.visible_meshlets = builder.create_buffer(
                "meshlet_culling.visible_meshlets",
                frame_graph::BufferCreateInfo {
                    .usage = frame_graph::BufferUsageFlags::SRV |
                             frame_graph::BufferUsageFlags::UAV,
                    .memory_type = frame_graph::MemoryType::GPU,
                    .size = sizeof(shader::VisibleMeshlet) * input.max_meshlet_count,
                });
            builder.write(
                data.visible_meshlets, frame_graph::ResourceUsage::SHADER_COMPUTE);

            data.visible_meshlets_count = builder.create_buffer(
                "meshlet_culling.visible_meshlets_count",
                frame_graph::BufferCreateInfo {
                    .usage = frame_graph::BufferUsageFlags::SRV |
                             frame_graph::BufferUsageFlags::UAV,
                    .memory_type = frame_graph::MemoryType::GPU,
                    .size = sizeof(u32),
                });
            builder.write(
                data.visible_meshlets_count, frame_graph::ResourceUsage::SHADER_COMPUTE);

            return data;
        },
        [=](rhi::IRHIContext* rhi,
            const frame_graph::Registry& registry,
            rhi::CommandEncoder& encoder,
            const Data& data) {
            //
            const rhi::BufferHandle ubo_buffer = registry.get_buffer(data.ubo_buffer);
            const rhi::BufferHandle visible_instances = registry.get_buffer(
                data.visible_instances);
            const rhi::BufferHandle meshlet_culling_dispatch_args = registry.get_buffer(
                data.meshlet_culling_dispatch_args);
            const rhi::BufferHandle visible_meshlets = registry.get_buffer(
                data.visible_meshlets);
            const rhi::BufferHandle visible_meshlets_count = registry.get_buffer(
                data.visible_meshlets_count);

            const ubo::MeshletCullingUBO ubo {
                .world_to_view = input.world_to_view,
                .frustum_planes = input.frustum_planes,
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

            rhi->update_buffer(
                ubo_buffer,
                {
                    rhi::BufferUpdateRegion {
                        .src = core::as_byte_span(ubo),
                        .dst_offset = data.ubo_buffer_offset,
                    },
                });

            encoder.push_constants(ubo_buffer, data.ubo_buffer_offset);
            encoder.dispatch_indirect(
                get_pipeline(pipelines::MESHLET_CULLING_NAME, input.compute_pipelines),
                meshlet_culling_dispatch_args,
                0);
        });

    return MeshletCullingOutput {
        .ubo_buffer = data.ubo_buffer,
        .ubo_buffer_offset = ubo_buffer_offset,
        .visible_meshlets = data.visible_meshlets,
        .visible_meshlets_count = data.visible_meshlets_count,
    };
}

} // namespace tundra::renderer::common::culling
