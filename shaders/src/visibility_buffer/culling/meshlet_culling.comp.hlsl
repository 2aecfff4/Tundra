#include "bindings.hlsli"
#include "defines.hlsli"
#include "math/quat.hlsli"
#include "templates.hlsli"
#include "visibility_buffer/inc/frustum_culling.hlsli"
#include "visibility_buffer/inc/instance_transform.hlsli"
#include "visibility_buffer/inc/mesh_descriptor.hlsli"
#include "visibility_buffer/inc/mesh_instance.hlsli"
#include "visibility_buffer/inc/meshlet.hlsli"
#include "visibility_buffer/inc/visible_mesh_instance.hlsli"
#include "visibility_buffer/inc/visible_meshlet.hlsli"

///
#define NUM_THREADS_X 128

///
struct MeshletCullingUBO {
    float4 frustum_planes[6];
    uint max_meshlet_count;

    struct {
        uint visible_mesh_instances_srv;
        uint mesh_descriptors_srv;
        uint mesh_instance_transforms_srv;
    } in_;

    struct {
        uint visible_meshlets_uav;
        uint visible_meshlets_count_uav;
    } out_;
};

///
struct Input {
    uint instance_index : SV_GroupID;
    uint meshlet_index : SV_GroupThreadID;
    uint thread_id : SV_DispatchThreadID;
};

///
bool cull_meshlet(
    const in MeshletCullingUBO ubo,
    const MeshDescriptor mesh_descriptor,
    const InstanceTransform instance_transform,
    const Meshlet meshlet)
{
    const float3 center = (quat_rotate_vector(instance_transform.quat, meshlet.center) *
                           instance_transform.scale) +
                          instance_transform.position;
    const float radius = meshlet.radius;

    return frustum_culling(ubo.frustum_planes, center, radius);
}

///
[numthreads(NUM_THREADS_X, 1, 1)] void main(const Input input) {
    const MeshletCullingUBO ubo = tundra::load_ubo<MeshletCullingUBO>();

    if (input.thread_id == 0) {
        tundra::buffer_store<false>(ubo.out_.visible_meshlets_count_uav, 0, 0, 0);
    }

    DeviceMemoryBarrierWithGroupSync();

    const VisibleMeshInstance visible_mesh_instance =
        tundra::buffer_load<false, VisibleMeshInstance>(
            ubo.in_.visible_mesh_instances_srv, 0, input.instance_index);
    const MeshDescriptor mesh_descriptor = tundra::buffer_load<true, MeshDescriptor>(
        ubo.in_.mesh_descriptors_srv, 0, visible_mesh_instance.mesh_descriptor_index);
    const InstanceTransform instance_transform =
        tundra::buffer_load<false, InstanceTransform>(
            ubo.in_.mesh_instance_transforms_srv,
            0,
            visible_mesh_instance.instance_transform_index);

    const uint num_loops = (mesh_descriptor.meshlet_count + (NUM_THREADS_X - 1)) /
                           NUM_THREADS_X;

    LOOP for (uint i = 0; i < num_loops; ++i)
    {
        const uint meshlet_index = input.meshlet_index + (i * NUM_THREADS_X);

        bool is_visible = false;
        if (meshlet_index < mesh_descriptor.meshlet_count) {
            const Meshlet meshlet = mesh_descriptor.get_meshlet(meshlet_index);
            is_visible = cull_meshlet(ubo, mesh_descriptor, instance_transform, meshlet);
        }

        // Count number of lanes with `is_visible` set to true(lane indices smaller than this lane’s)
        const uint meshlet_offset = WavePrefixCountBits(is_visible);
        // Count number of visible meshlets in this wave.
        const uint visible_meshlet_count = WaveActiveCountBits(is_visible);

        uint global_meshlet_offset = 0;
        if (WaveIsFirstLane()) {
            tundra::buffer_interlocked_add<false>(
                ubo.out_.visible_meshlets_count_uav,
                0,
                visible_meshlet_count,
                global_meshlet_offset);
        }

        // Get `global_meshlet_offset` from the first lane.
        global_meshlet_offset = WaveReadLaneFirst(global_meshlet_offset);

        if (is_visible && (meshlet_index < mesh_descriptor.meshlet_count) &&
            ((global_meshlet_offset + meshlet_offset) < ubo.max_meshlet_count)) {
            tundra::buffer_store<false>(
                ubo.out_.visible_meshlets_uav,
                0,
                global_meshlet_offset + meshlet_offset,
                VisibleMeshlet::create(
                    visible_mesh_instance.mesh_descriptor_index,
                    meshlet_index,
                    visible_mesh_instance.instance_transform_index));
        }
    }
}
