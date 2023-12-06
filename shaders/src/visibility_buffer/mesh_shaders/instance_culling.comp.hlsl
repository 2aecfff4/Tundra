#include "bindings.hlsli"
#include "defines.hlsli"
#include "math/quat.hlsli"
#include "shared.hlsli"
#include "templates.hlsli"
#include "visibility_buffer/inc/commands.hlsli"
#include "visibility_buffer/inc/frustum_culling.hlsli"
#include "visibility_buffer/inc/instance_transform.hlsli"
#include "visibility_buffer/inc/mesh_descriptor.hlsli"
#include "visibility_buffer/inc/mesh_instance.hlsli"

///
struct InstanceCullingUbo {
    float4 frustum_planes[6];
    uint num_instances;

    struct {
        uint mesh_descriptors_srv;
        uint mesh_instances_srv;
        uint mesh_instance_transforms_srv;
    }
    in_;

    struct {
        uint command_count_uav;
        uint command_buffer_uav;
    }
    out_;
};

///
bool cull_frustum_mesh_instance(
    const InstanceCullingUbo ubo,
    const MeshDescriptor mesh_descriptor,
    const InstanceTransform instance_transform)
{
    const float3 center = (quat_rotate_vector(
                               instance_transform.quat, mesh_descriptor.center) *
                           instance_transform.scale) +
                          instance_transform.position;
    const float radius = mesh_descriptor.radius;

    return frustum_culling(ubo.frustum_planes, center, radius);
}

///
[numthreads(128, 1, 1)]
void main(uint thread_id: SV_DispatchThreadID)
{
    const InstanceCullingUbo ubo = tundra::load_ubo<InstanceCullingUbo>();

    const uint num_instances = ubo.num_instances;
    const uint instance_index = thread_id;

    MeshInstance mesh_instance;
    InstanceTransform instance_transform;
    MeshDescriptor mesh_descriptor;

    bool is_visible = false;

    if (instance_index < num_instances) {
        mesh_instance = tundra::buffer_load<false, MeshInstance>(
            ubo.in_.mesh_instances_srv, 0, instance_index);
        instance_transform = tundra::buffer_load<false, InstanceTransform>(
            ubo.in_.mesh_instance_transforms_srv, 0, instance_index);
        mesh_descriptor = tundra::buffer_load<false, MeshDescriptor>(
            ubo.in_.mesh_descriptors_srv, 0, mesh_instance.mesh_descriptor_index);

        is_visible = cull_frustum_mesh_instance(ubo, mesh_descriptor, instance_transform);
    }

    if (is_visible) {
        const uint task_group_count = (mesh_descriptor.meshlet_count +
                                       (TASK_WORKGROUP_SIZE - 1)) /
                                      TASK_WORKGROUP_SIZE;

        const uint wave_task_group_offset = WavePrefixSum(task_group_count);
        const uint wave_task_group_sum = WaveActiveSum(task_group_count);

        uint offset = 0;
        if (WaveIsFirstLane()) {
            tundra::buffer_interlocked_add<false>(
                ubo.out_.command_count_uav, //
                0,
                wave_task_group_sum,
                offset);
        }
        offset = WaveReadLaneFirst(offset);

        const uint command_index = offset + wave_task_group_offset;
        if (command_index < TASK_NUM_WORKGROUP_LIMIT) {
            for (uint i = 0; i < task_group_count; ++i) {
                const uint meshlet_offset = i * TASK_WORKGROUP_SIZE;

                tundra::buffer_store<false>(
                    ubo.out_.command_buffer_uav,
                    0,
                    command_index,
                    MeshTaskCommand::create(
                        instance_index,
                        meshlet_offset,
                        min(mesh_descriptor.meshlet_count - (i * TASK_WORKGROUP_SIZE),
                            TASK_WORKGROUP_SIZE)));
            }
        } else {
            // #TODO: We probably should note that we went over the limit, and how much.
        }
    }
}
