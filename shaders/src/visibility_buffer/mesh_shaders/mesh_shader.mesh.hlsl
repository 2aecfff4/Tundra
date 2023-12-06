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
struct MeshShaderUbo {
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
[outputtopology("triangle")]
[numthreads(128, 1, 1)]
void main(
    in payload Payload payload,
    out vertices VertexOutput vertices[64],
    out indices uint3 triangles[128],
    const uint group_thread_id: SV_GroupThreadID)
{
    const MeshShaderUbo ubo = tundra::load_ubo<MeshShaderUbo>();

    const uint instance_id = payload.instance_id;
    const uint meshlet_index = payload.meshlet_indices[group_thread_id];

    const MeshInstance mesh_instance //
        = tundra::buffer_load<false, MeshInstance>(
            ubo.in_.mesh_instances_srv, 0, instance_id);
    const InstanceTransform instance_transform //
        = tundra::buffer_load<false, InstanceTransform>(
            ubo.in_.mesh_instance_transforms_srv, 0, instance_id);
    const MeshDescriptor mesh_descriptor //
        = tundra::buffer_load<false, MeshDescriptor>(
            ubo.in_.mesh_descriptors_srv, 0, mesh_instance.mesh_descriptor_index);
    const Meshlet meshlet = mesh_descriptor.get_meshlet(meshlet_index);

    SetMeshOutputCounts(meshlet.vertex_count, meshlet.triangle_count);

    if (group_thread_id < meshlet.vertex_count) {
        const uint vertex_id = group_thread_id;
        const float3 vertex = mesh_descriptor.get_vertex(meshlet, vertex_id);
        const float3 world_space_vertex = (quat_rotate_vector(
                                               instance_transform.quat, vertex) *
                                           instance_transform.scale) +
                                          instance_transform.position;

        float4 position = float4(world_space_vertex, 1.f);
        position = mul(ubo.world_to_view, position);
        position = mul(ubo.view_to_clip, position);

        VertexOutput output;
        output.position = position;
        output.instance_meshlet_id = ((uint64_t)instance_id << 32) |
                                     ((uint64_t)meshlet_index << 8);

        vertices[vertex_id] = output;
    }

    if (group_thread_id < meshlet.triangle_count) {
        const uint triangle_id = group_thread_id;
        // #NOTE: Amd doesn't recommend per-triangle culling.
        // SAMPLER FEEDBACK & MESH SHADERS, page 13
        // https://gpuopen.com/wp-content/uploads/slides/AMD_RDNA2_DirectX12_Ultimate_SamplerFeedbackMeshShaders.pdf
        const uint3 t = mesh_descriptor.get_meshlet_triangle(meshlet, triangle_id);
        triangles[group_thread_id] = t;
    }
}
