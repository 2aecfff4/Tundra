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
struct TaskShaderUbo {
    float4x4 view_to_clip;
    float4x4 world_to_view;
    float4 frustum_planes[6];
    float3 camera_position;

    struct {
        uint mesh_descriptors_srv;
        uint mesh_instances_srv;
        uint mesh_instance_transforms_srv;

        uint command_count_srv;
        uint command_buffer_srv;
    }
    in_;
};

///
groupshared Payload g_payload;

///
[numthreads(64, 1, 1)]
void main(const uint group_thread_id: SV_GroupThreadID, const uint3 group_id: SV_GroupID)
{
    const TaskShaderUbo ubo = tundra::load_ubo<TaskShaderUbo>();

    const uint group_index = group_id.x * 64 + group_id.y;
    const uint command_count //
        = tundra::buffer_load<false, uint>(ubo.in_.command_count_srv, 0, 0);
    if (group_index >= command_count) {
        return;
    }

    const MeshTaskCommand command //
        = tundra::buffer_load<false, MeshTaskCommand>(
            ubo.in_.command_buffer_srv, 0, group_index);

    const uint meshlet_index = group_thread_id + command.meshlet_offset;

    const MeshInstance mesh_instance //
        = tundra::buffer_load<false, MeshInstance>(
            ubo.in_.mesh_instances_srv, 0, command.instance_id);
    const InstanceTransform instance_transform //
        = tundra::buffer_load<false, InstanceTransform>(
            ubo.in_.mesh_instance_transforms_srv, 0, command.instance_id);
    const MeshDescriptor mesh_descriptor //
        = tundra::buffer_load<false, MeshDescriptor>(
            ubo.in_.mesh_descriptors_srv, 0, mesh_instance.mesh_descriptor_index);

    bool is_visible = group_thread_id < command.num_meshlets;
    if (is_visible) {
        const Meshlet meshlet = mesh_descriptor.get_meshlet(meshlet_index);

        const float3 center = mul(ubo.world_to_view,
                                  float4(
                                      ((quat_rotate_vector(
                                            instance_transform.quat, meshlet.center) *
                                        instance_transform.scale) +
                                       instance_transform.position),
                                      1))
                                  .xyz;

        const float radius = meshlet.radius;

        // #TODO: Frustum culling is a little bit broken.
        // As you move forward, the frustum gets smaller and culls too much.
        is_visible = frustum_culling(ubo.frustum_planes, center, radius);

        {
            float3 cone_axis = float3(
                (int(((meshlet.cone_axis_and_cutoff & 0xFF000000u) >> 24u)) - 127) /
                    127.0,
                (int(((meshlet.cone_axis_and_cutoff & 0x00FF0000u) >> 16u)) - 127) /
                    127.0,
                (int(((meshlet.cone_axis_and_cutoff & 0x0000FF00u) >> 8u)) - 127) /
                    127.0);

            cone_axis = quat_rotate_vector(instance_transform.quat, cone_axis);

            const float cone_cutoff = (int((meshlet.cone_axis_and_cutoff & 0x000000FF)) -
                                       127) /
                                      127.0;

            const float3 direction = center - ubo.camera_position;
            const float3 normalized_direction = normalize(direction);
            const float distance = length(direction);

            const bool cone_cull = dot(normalized_direction, cone_axis) >=
                                   ((cone_cutoff * distance) + radius);

            is_visible = is_visible && !cone_cull;
        }
    }

    if (is_visible) {
        const uint index = WavePrefixCountBits(is_visible);
        g_payload.meshlet_indices[index] = meshlet_index;
    }
    g_payload.instance_id = command.instance_id;

    const uint visible_meshlet_count = WaveActiveCountBits(is_visible);
    DispatchMesh(visible_meshlet_count, 1, 1, g_payload);
}
