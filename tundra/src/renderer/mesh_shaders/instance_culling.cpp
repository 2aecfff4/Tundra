#include "renderer/mesh_shaders/instance_culling.h"

#include "pipelines.h"
#include "renderer/common/culling/instance_culling_and_lod.h"
#include "renderer/helpers.h"
#include "rhi/commands/dispatch_indirect.h"
#include "rhi/rhi_context.h"
#include <array>

namespace tundra::renderer::mesh_shaders {

namespace ubo {

///
struct InstanceCullingInitUbo {
    u32 command_count_uav = config::INVALID_SHADER_HANDLE;
};

///
struct InstanceCullingUbo {
    std::array<math::Vec4, config::NUM_PLANES> frustum_planes = {};
    u32 num_instances = 0;

    struct {
        u32 mesh_descriptors_srv = config::INVALID_SHADER_HANDLE;
        u32 mesh_instances_srv = config::INVALID_SHADER_HANDLE;
        u32 mesh_instance_transforms_srv = config::INVALID_SHADER_HANDLE;
    } in_;

    struct {
        u32 command_count_uav = config::INVALID_SHADER_HANDLE;
        u32 command_buffer_uav = config::INVALID_SHADER_HANDLE;
    } out_;
};

///
struct MeshTaskCommand {
    u32 instance_id;
    u32 meshlet_offset;
    /// Number of threads we want to execute inside of a task shader.
    /// This is important, because there may be situation
    /// where the group size is bigger than number of meshlets we want to process.
    u32 num_meshlets;
};

} // namespace ubo

namespace config {

inline constexpr u32 MAX_INSTANCE_COUNT = 1u << 16u;

/// https://docs.vulkan.org/spec/latest/chapters/limits.html#limits-minmax
/// maxMeshWorkGroupTotalCount: minimum 2^22
inline constexpr u32 TASK_NUM_WORKGROUP_LIMIT = 1u << 22u;

} // namespace config

///
InstanceCullingOutput instance_culling(
    frame_graph::FrameGraph& fg, const InstanceCullingInput& input) noexcept
{
    tndr_assert(input.num_instances < config::MAX_INSTANCE_COUNT, "Too many instances");

    struct Data {
        core::SharedPtr<UboBuffer> ubo_buffer;

        frame_graph::BufferHandle command_count;
        frame_graph::BufferHandle command_buffer;
    };

    const Data data = fg.add_pass(
        frame_graph::QueueType::Graphics,
        "instance_culling",
        [&](frame_graph::Builder& builder) {
            Data data {};

            data.ubo_buffer = input.ubo_buffer;

            data.command_count = builder.create_buffer(
                "instance_culling.command_count",
                frame_graph::BufferCreateInfo {
                    .usage = frame_graph::BufferUsageFlags::STORAGE_BUFFER,
                    .memory_type = frame_graph::MemoryType::GPU,
                    .size = sizeof(u32),
                });

            builder.write(
                data.command_count,
                frame_graph::BufferResourceUsage::COMPUTE_STORAGE_BUFFER);

            data.command_buffer = builder.create_buffer(
                "instance_culling.command_buffer",
                frame_graph::BufferCreateInfo {
                    .usage = frame_graph::BufferUsageFlags::STORAGE_BUFFER,
                    .memory_type = frame_graph::MemoryType::GPU,
                    .size = sizeof(ubo::MeshTaskCommand) *
                            config::TASK_NUM_WORKGROUP_LIMIT,
                });
            builder.write(
                data.command_buffer,
                frame_graph::BufferResourceUsage::COMPUTE_STORAGE_BUFFER);

            return data;
        },
        [=](rhi::IRHIContext* rhi,
            const frame_graph::Registry& registry,
            rhi::CommandEncoder& encoder,
            const Data& data) {
            //
            const rhi::BufferHandle ubo_buffer //
                = registry.get_buffer(data.ubo_buffer->buffer());
            const rhi::BufferHandle command_count //
                = registry.get_buffer(data.command_count);
            const rhi::BufferHandle command_buffer //
                = registry.get_buffer(data.command_buffer);

            const ubo::InstanceCullingInitUbo init_ubo {
                .command_count_uav = command_count.get_uav(),
            };

            const ubo::InstanceCullingUbo culling_ubo {
                .frustum_planes = input.frustum_planes,
                .num_instances = input.num_instances,
                .in_ = {
                    .mesh_descriptors_srv = input.mesh_descriptors.get_srv(),
                    .mesh_instances_srv = input.mesh_instances.get_srv(),
                    .mesh_instance_transforms_srv = input.mesh_instance_transforms.get_srv(),
                },
                .out_ = {
                    .command_count_uav = command_count.get_uav(),
                    .command_buffer_uav = command_buffer.get_uav(),
                },
            };

            auto ubo_ref = data.ubo_buffer->allocate<ubo::InstanceCullingInitUbo>();
            rhi->update_buffer(
                ubo_buffer,
                {
                    rhi::BufferUpdateRegion {
                        .src = core::as_byte_span(init_ubo),
                        .dst_offset = ubo_ref.offset,
                    },
                });
            encoder.push_constants(ubo_buffer, static_cast<u32>(ubo_ref.offset));
            encoder.dispatch(
                helpers::get_pipeline(
                    pipelines::mesh_shaders::passes::INSTANCE_CULLING_INIT_NAME,
                    input.compute_pipelines),
                1,
                1,
                1);

            encoder.global_barrier(rhi::GlobalBarrier {
                .previous_access = rhi::GlobalAccessFlags::ALL,
                .next_access = rhi::GlobalAccessFlags::ALL,
            });

            ubo_ref = data.ubo_buffer->allocate<ubo::InstanceCullingUbo>();
            rhi->update_buffer(
                ubo_buffer,
                {
                    rhi::BufferUpdateRegion {
                        .src = core::as_byte_span(culling_ubo),
                        .dst_offset = ubo_ref.offset,
                    },
                });
            encoder.push_constants(ubo_buffer, static_cast<u32>(ubo_ref.offset));
            encoder.dispatch(
                helpers::get_pipeline(
                    pipelines::mesh_shaders::passes::INSTANCE_CULLING_NAME,
                    input.compute_pipelines),
                rhi::CommandEncoder::get_group_count(input.num_instances, 128),
                1,
                1);
        });

    return InstanceCullingOutput {
        .command_count = data.command_count,
        .command_buffer = data.command_buffer,
    };
}

} // namespace tundra::renderer::mesh_shaders
