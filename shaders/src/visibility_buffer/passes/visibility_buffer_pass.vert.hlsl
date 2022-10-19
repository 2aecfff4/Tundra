#include "bindings.hlsli"
#include "defines.hlsli"
#include "math/quat.hlsli"
#include "templates.hlsli"
#include "visibility_buffer/inc/frustum_culling.hlsli"
#include "visibility_buffer/inc/instance_transform.hlsli"
#include "visibility_buffer/inc/mesh_descriptor.hlsli"
#include "visibility_buffer/inc/mesh_instance.hlsli"
#include "visibility_buffer/inc/unpacked_index.hlsli"
#include "visibility_buffer/inc/visible_mesh_instance.hlsli"
#include "visibility_buffer/inc/visible_meshlet.hlsli"

///
struct VisibilityBufferVertInput {
    uint vertex_id : SV_VertexID;
};

///
struct VisibilityBufferVertOutput {
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
    uint visible_meshlet_index : TEXCOORD1;
    uint meshlet_index : TEXCOORD2;
};

///
struct VisibilityBufferPassUBO {
    /// View
    float4x4 world_to_view;
    /// Projection
    float4x4 view_to_clip;

    struct {
        uint visible_meshlets_srv;
        uint mesh_descriptors_srv;
        uint mesh_instance_transforms_srv;
    } in_;
};

///
float3 load_vertex_from_meshlet(
    const MeshDescriptor mesh_descriptor,
    const Meshlet meshlet,
    const UnpackedIndex unpacked_index)
{
    // vertices[meshlet_vertices[meshlets[meshlet_id].vertex_offset + vertex_id]]
    const uint vertex_index_buffer_index = meshlet.vertex_offset +
                                           unpacked_index.vertex_id;

    const uint vertex_buffer_index = tundra::buffer_load<true, uint>(
        mesh_descriptor.mesh_data_buffer_srv,
        mesh_descriptor.meshlet_vertices_offset,
        vertex_index_buffer_index);

    return tundra::buffer_load<true, float3>(
        mesh_descriptor.mesh_data_buffer_srv,
        mesh_descriptor.vertex_buffer_offset,
        vertex_buffer_index);
}

///
VisibilityBufferVertOutput main(VisibilityBufferVertInput input)
{
    const VisibilityBufferPassUBO ubo = tundra::load_ubo<VisibilityBufferPassUBO>();

    const UnpackedIndex unpacked_index = UnpackedIndex::create(input.vertex_id);
    const VisibleMeshlet visible_meshlet = tundra::buffer_load<true, VisibleMeshlet>(
        ubo.in_.visible_meshlets_srv, 0, unpacked_index.meshlet_id);
    const InstanceTransform instance_transform =
        tundra::buffer_load<true, InstanceTransform>(
            ubo.in_.mesh_instance_transforms_srv,
            0,
            visible_meshlet.instance_transform_index);
    const MeshDescriptor mesh_descriptor = tundra::buffer_load<true, MeshDescriptor>(
        ubo.in_.mesh_descriptors_srv, 0, visible_meshlet.mesh_descriptor_index);
    const Meshlet meshlet = mesh_descriptor.get_meshlet(visible_meshlet.meshlet_index);

    // const float3 vertex = load_vertex_from_meshlet(
    //     mesh_descriptor, meshlet, unpacked_index);
    const float3 vertex = mesh_descriptor.get_vertex(meshlet, unpacked_index.vertex_id);

    const float3 world_space_vertex = (quat_rotate_vector(
                                           instance_transform.quat, vertex) *
                                       instance_transform.scale) +
                                      instance_transform.position;

    VisibilityBufferVertOutput output;
    float4 pos = float4(world_space_vertex, 1.f);
    pos = mul(ubo.world_to_view, pos);
    pos = mul(ubo.view_to_clip, pos);

    output.position = pos;
    output.uv = mesh_descriptor.get_uv(meshlet, unpacked_index.vertex_id, 0);
    output.visible_meshlet_index = unpacked_index.meshlet_id;
    output.meshlet_index = visible_meshlet.meshlet_index;

    return output;
}