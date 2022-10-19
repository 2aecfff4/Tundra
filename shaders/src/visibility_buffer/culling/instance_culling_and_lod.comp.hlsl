#include "bindings.hlsli"
#include "defines.hlsli"
#include "math/quat.hlsli"
#include "templates.hlsli"
#include "visibility_buffer/inc/commands.hlsli"
#include "visibility_buffer/inc/frustum_culling.hlsli"
#include "visibility_buffer/inc/instance_transform.hlsli"
#include "visibility_buffer/inc/mesh_descriptor.hlsli"
#include "visibility_buffer/inc/mesh_instance.hlsli"
#include "visibility_buffer/inc/visible_mesh_instance.hlsli"

///
struct InstanceCullingUBO {
    float4 frustum_planes[6];
    uint num_instances;

    struct {
        uint mesh_descriptors_srv;
        uint mesh_instances_srv;
        uint mesh_instance_transforms_srv;
    } in_;

    struct {
        uint visible_mesh_instances_uav;
        uint meshlet_culling_dispatch_args_uav;
    } out_;
};

///
bool cull_frustum_mesh_instance(
    const InstanceCullingUBO ubo,
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
[numthreads(128, 1, 1)] void main(uint thread_id
                                  : SV_DispatchThreadID) {
    const InstanceCullingUBO ubo = tundra::load_ubo<InstanceCullingUBO>();

    if (thread_id == 0) {
        tundra::buffer_store<false>(
            ubo.out_.meshlet_culling_dispatch_args_uav,
            0,
            0,
            DispatchIndirectCommand::create(0, 1, 1));
    }

    DeviceMemoryBarrierWithGroupSync();

    const uint num_instances = ubo.num_instances;
    const uint instance_index = thread_id;

    MeshInstance mesh_instance;
    InstanceTransform instance_transform;
    MeshDescriptor mesh_descriptor;

    bool is_visible = false;
    // uint lod_index = 0;

    if (instance_index < num_instances) {
        mesh_instance = tundra::buffer_load<false, MeshInstance>(
            ubo.in_.mesh_instances_srv, 0, instance_index);
        instance_transform = tundra::buffer_load<false, InstanceTransform>(
            ubo.in_.mesh_instance_transforms_srv, 0, instance_index);
        mesh_descriptor = tundra::buffer_load<false, MeshDescriptor>(
            ubo.in_.mesh_descriptors_srv, 0, mesh_instance.mesh_descriptor_index);

        is_visible = cull_frustum_mesh_instance(ubo, mesh_descriptor, instance_transform);

        /*
        // #TODO: Depth culling
        if (is_visible) {
        }

        // #TODO: Compute lod, but only when instance is visible.
        if (is_visible) {
        }
*/
    }

    // Count number of lanes with `is_visible` set to true(lane indices smaller than this lane’s)
    const uint index_offset = WavePrefixCountBits(is_visible);
    // Count number of visible instances in this wave.
    const uint visible_instance_count = WaveActiveCountBits(is_visible);

    uint global_index_offset = 0;
    if (WaveIsFirstLane()) {
        tundra::buffer_interlocked_add<false>(
            ubo.out_.meshlet_culling_dispatch_args_uav,
            0,
            visible_instance_count,
            global_index_offset);
    }

    // Get `global_index_offset` from the first lane.
    global_index_offset = WaveReadLaneFirst(global_index_offset);

    if (is_visible && (instance_index < num_instances)) {
        tundra::buffer_store<false>(
            ubo.out_.visible_mesh_instances_uav,
            0,
            global_index_offset + index_offset,
            VisibleMeshInstance::create(
                mesh_instance.mesh_descriptor_index, instance_index, 0));
    }
}
