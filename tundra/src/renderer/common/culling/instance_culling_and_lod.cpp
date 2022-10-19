#include "renderer/common/culling/instance_culling_and_lod.h"
#include "pipelines.h"
#include "renderer/helpers.h"
#include "rhi/commands/dispatch_indirect.h"
#include "rhi/rhi_context.h"
#include <array>

namespace tundra::renderer::common::culling {

namespace ubo {

///
struct InstanceCullingUBO {
    std::array<math::Vec4, config::NUM_PLANES> frustum_planes = {};
    u32 instance_count = 0;

    struct {
        u32 mesh_descriptors_srv = config::INVALID_SHADER_HANDLE;
        u32 mesh_instances_srv = config::INVALID_SHADER_HANDLE;
        u32 mesh_instance_transforms_srv = config::INVALID_SHADER_HANDLE;
    } in_;

    struct {
        u32 visible_mesh_instances_uav = config::INVALID_SHADER_HANDLE;
        u32 meshlet_culling_dispatch_args_uav = config::INVALID_SHADER_HANDLE;
    } out_;
};

} // namespace ubo

namespace config {

inline constexpr u32 MAX_INSTANCE_COUNT = 1u << 16u;

} // namespace config

///
InstanceCullingOutput instance_culling_and_lod(
    frame_graph::FrameGraph& fg, const InstanceCullingInput& input) noexcept
{
    tndr_assert(input.instance_count < config::MAX_INSTANCE_COUNT, "Too many instances");

    const u64 ubo_buffer_offset = input.ubo_buffer_offset +
                                  sizeof(ubo::InstanceCullingUBO);

    struct Data {
        frame_graph::BufferHandle ubo_buffer;
        u64 ubo_buffer_offset = 0;

        frame_graph::BufferHandle visible_instances;
        frame_graph::BufferHandle meshlet_culling_dispatch_args;
    };

    const Data data = fg.add_pass(
        frame_graph::QueueType::Graphics,
        "instance_culling_and_lod",
        [&](frame_graph::Builder& builder) {
            Data data {};

            data.ubo_buffer = input.ubo_buffer;
            data.ubo_buffer_offset = input.ubo_buffer_offset;

            data.visible_instances = builder.create_buffer(
                "instance_culling_and_lod.visible_instances",
                frame_graph::BufferCreateInfo {
                    .usage = frame_graph::BufferUsageFlags::SRV |
                             frame_graph::BufferUsageFlags::UAV,
                    .memory_type = frame_graph::MemoryType::GPU,
                    .size = sizeof(shader::VisibleMeshInstance) * input.instance_count,
                });

            builder.write(
                data.visible_instances, frame_graph::ResourceUsage::SHADER_COMPUTE);

            data.meshlet_culling_dispatch_args = builder.create_buffer(
                "instance_culling_and_lod.meshlet_culling_dispatch_args",
                frame_graph::BufferCreateInfo {
                    .usage = frame_graph::BufferUsageFlags::UAV |
                             frame_graph::BufferUsageFlags::INDIRECT_BUFFER,
                    .memory_type = frame_graph::MemoryType::GPU,
                    .size = sizeof(rhi::DispatchIndirectCommand),
                });
            builder.write(
                data.meshlet_culling_dispatch_args,
                frame_graph::ResourceUsage::SHADER_COMPUTE);

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

            const ubo::InstanceCullingUBO ubo {
                .frustum_planes = input.frustum_planes,
                .instance_count = input.instance_count,
                .in_ = {
                    .mesh_descriptors_srv = input.mesh_descriptors.get_srv(),
                    .mesh_instances_srv = input.mesh_instances.get_srv(),
                    .mesh_instance_transforms_srv = input.mesh_instance_transforms.get_srv(),
                },
                .out_ = {
                    .visible_mesh_instances_uav = visible_instances.get_uav(),
                    .meshlet_culling_dispatch_args_uav = meshlet_culling_dispatch_args.get_uav(),
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
            encoder.dispatch(
                helpers::get_pipeline(
                    pipelines::common::culling::INSTANCE_CULLING_AND_LOD_PIPELINE_NAME,
                    input.compute_pipelines),
                rhi::CommandEncoder::get_group_count(input.instance_count, 128),
                1,
                1);
        });

    return InstanceCullingOutput {
        .ubo_buffer = data.ubo_buffer,
        .ubo_buffer_offset = ubo_buffer_offset,
        .visible_instances = data.visible_instances,
        .meshlet_culling_dispatch_args = data.meshlet_culling_dispatch_args,
    };
}

} // namespace tundra::renderer::common::culling
