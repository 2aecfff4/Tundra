#ifndef TNDR_MESHLET_RENDERER_INC_MESH_DESCRIPTOR_H
#define TNDR_MESHLET_RENDERER_INC_MESH_DESCRIPTOR_H

#include "bindings.hlsli"
#include "meshlet.hlsli"
#include "templates.hlsli"

#define MESH_POSITION_SIZE sizeof(float3)
#define MESH_NORMAL_SIZE sizeof(float3)
#define MESH_TANGENT_SIZE sizeof(float4)
#define MESH_UV_SIZE sizeof(float2)

/// Vertex buffer layout
/// bits:...6 5 4 3 2 1 0
///       | | | | | | | |- positions
///       | | | | | | |--- normals
///       | | | | | |----- tangents
///       | | | | |------- uv0
///       | | | |--------- uv1
///       | | |----------- uv2
///       | |------------- unused
///       |--------------- unused
enum VertexBufferLayout : uint {
    POSITIONS_BIT = 1 << 0,
    NORMALS_BIT = 1 << 1,
    TANGENTS_BIT = 1 << 2,
    UV0_BIT = 1 << 3,
    UV1_BIT = 1 << 4,
    UV2_BIT = 1 << 5,
    MAX_VALUE = 1 << 6,
};

///
struct MeshDescriptor {
    float3 center;
    float radius;

    /// Buffer layout:
    /// - Meshlets
    /// - Meshlet triangles
    /// - Meshlet vertices
    /// - Vertex buffer
    ///   - SOA [position][position][position][normal][normal][normal][uv][uv][uv]
    uint mesh_data_buffer_srv;

    uint meshlet_count;

    uint meshlet_triangles_offset;
    uint meshlet_triangles_count;

    uint meshlet_vertices_offset;
    uint meshlet_vertices_count;

    uint vertex_buffer_offset;
    uint vertex_count;

    /// DXC 1.7.2207 error: Explicit template arguments on intrinsic Load must be a single numeric type
    /// VertexBufferLayout
    uint vertex_buffer_layout;

    ///
    static MeshDescriptor load(
        const uint mesh_descriptor_buffer_srv, const uint mesh_descriptor_offset)
    {
        return tundra::buffer_load<true, MeshDescriptor>(
            mesh_descriptor_buffer_srv, mesh_descriptor_offset, 0);
    }

    ///
    Meshlet get_meshlet(const uint meshlet_index)
    {
        return tundra::buffer_load<true, Meshlet>(mesh_data_buffer_srv, 0, meshlet_index);
    }

    ///
    uint3 get_meshlet_triangle(const Meshlet meshlet, const uint triangle_index)
    {
        const uint index_0 = tundra::buffer_load<true, uint>(
            mesh_data_buffer_srv,
            meshlet_triangles_offset,
            meshlet.triangle_offset + (triangle_index * 3) + 0);
        const uint index_1 = tundra::buffer_load<true, uint>(
            mesh_data_buffer_srv,
            meshlet_triangles_offset,
            meshlet.triangle_offset + (triangle_index * 3) + 1);
        const uint index_2 = tundra::buffer_load<true, uint>(
            mesh_data_buffer_srv,
            meshlet_triangles_offset,
            meshlet.triangle_offset + (triangle_index * 3) + 2);

        return uint3(index_0, index_1, index_2);
    }

    float3 get_vertex(const Meshlet meshlet, const uint vertex_id)
    {
        // vertices[meshlet_vertices[meshlets[meshlet_id].vertex_offset + vertex_id]]
        const uint vertex_index_buffer_index = meshlet.vertex_offset + vertex_id;

        const uint vertex_buffer_index = tundra::buffer_load<true, uint>(
            mesh_data_buffer_srv, meshlet_vertices_offset, vertex_index_buffer_index);

        return tundra::buffer_load<true, float3>(
            mesh_data_buffer_srv, vertex_buffer_offset, vertex_buffer_index);
    }

    float2 get_uv(const Meshlet meshlet, const uint vertex_id, const uint uv_index)
    {
        const uint uv_bit = UV0_BIT << (VertexBufferLayout)uv_index;
        if ((vertex_buffer_layout & uv_bit) == 0) {
            return float2(0.f / 0.f, 0.f / 0.f);
        }

        // vertices[meshlet_vertices[meshlets[meshlet_id].vertex_offset + vertex_id]]
        const uint vertex_index_buffer_index = meshlet.vertex_offset + vertex_id;

        const uint vertex_buffer_index = tundra::buffer_load<true, uint>(
            mesh_data_buffer_srv, meshlet_vertices_offset, vertex_index_buffer_index);

        uint offset = vertex_buffer_offset;
        offset += MESH_POSITION_SIZE * vertex_count;
        offset += ((vertex_buffer_layout & NORMALS_BIT) != 0) *
                  (MESH_NORMAL_SIZE * vertex_count);
        offset += ((vertex_buffer_layout & TANGENTS_BIT) != 0) *
                  (MESH_TANGENT_SIZE * vertex_count);

        for (uint i = 0; i < uv_index; ++i) {
            offset += ((vertex_buffer_layout & (UV0_BIT << (VertexBufferLayout)i)) != 0) *
                      (MESH_UV_SIZE * vertex_count);
        }

        return tundra::buffer_load<true, float2>(
            mesh_data_buffer_srv, offset, vertex_buffer_index);
    }
};

#endif // TNDR_MESHLET_RENDERER_INC_MESH_DESCRIPTOR_H
