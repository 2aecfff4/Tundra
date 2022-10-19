#include "bindings.hlsli"
#include "defines.hlsli"
#include "math/quat.hlsli"
#include "templates.hlsli"
#include "visibility_buffer/inc/commands.hlsli"
#include "visibility_buffer/inc/frustum_culling.hlsli"
#include "visibility_buffer/inc/instance_transform.hlsli"
#include "visibility_buffer/inc/mesh_descriptor.hlsli"
#include "visibility_buffer/inc/mesh_instance.hlsli"
#include "visibility_buffer/inc/unpacked_index.hlsli"
#include "visibility_buffer/inc/visible_mesh_instance.hlsli"
#include "visibility_buffer/inc/visible_meshlet.hlsli"

// https://microsoft.github.io/DirectX-Specs/d3d/archive/D3D11_3_FunctionalSpec.htm#CoordinateSnapping
#define SUBPIXEL_SAMPLES 256
#define SMALL_NUMBER 1.e-4f

///
struct GPURasterizeUBO {
    float4x4 world_to_clip;
    uint2 view_size;

    struct {
        uint mesh_instance_transforms_srv;
        uint mesh_descriptors_srv;
        uint visible_meshlets_srv;
    } in_;

    struct {
        uint output_texture_uav;
    } out_;
};

///
struct Input {
    uint meshlet_index : SV_GroupID;
    uint triangle_index : SV_GroupThreadID;
    uint thread_id : SV_DispatchThreadID;
    uint group_thread_id : SV_GroupThreadID;
};

///
void write_pixel(
    RWTexture2D<uint64_t> texture,
    const uint meshlet_index,
    const uint triangle_index,
    const uint2 pos,
    const float z)
{
    const uint meshlet_triangle = (meshlet_index << VERTEX_ID_BITS) | triangle_index;
    const uint depth_uint = asuint(z);
    const uint64_t value = (((uint64_t)depth_uint << (uint64_t)32)) | meshlet_triangle;
    InterlockedMax(texture[pos], value);
}

///
void rasterize(
    const GPURasterizeUBO ubo,
    const uint meshlet_index,
    const uint triangle_index,
    const float3 p0,
    const float3 p1,
    const float3 p2)
{
    if ((p0.z < 0.0f) || (p1.z < 0.0f) || (p2.z < 0.0f)) {
        return;
    }

    /// Backface culling
    const float3 p10 = p1 - p0;
    const float3 p20 = p2 - p0;
    const float det = p20.x * p10.y - p20.y * p10.x;
    if (det <= 0.0f) {
        return;
    }

    // Pixel boundaries
    float2 min_p = floor(min(min(p0.xy, p1.xy), p2.xy));
    float2 max_p = ceil(max(max(p0.xy, p1.xy), p2.xy));
    if ((max_p.x < 0.0f) || (max_p.y < 0.0f) || (min_p.x >= ubo.view_size.x) ||
        (min_p.y >= ubo.view_size.y)) {
        return;
    }

    min_p = clamp(min_p, float2(0, 0), ubo.view_size);
    max_p = clamp(max_p, float2(0, 0), ubo.view_size - 1);

    if (any(min_p > max_p)) {
        // Pixel is not visible
        return;
    }

    // #TODO: "Nanite - A Deep Dive" Brian Karis, Rune Stubbe, Graham Wihlidal
    // Page 90
    // http://advances.realtimerendering.com/s2021/Karis_Nanite_SIGGRAPH_Advances_2021_final.pdf
    for (float y = min_p.y; y <= max_p.y; y += 1.f) {
        for (float x = min_p.x; x <= max_p.x; x += 1.f) {
            const float2 p = float2(x, y);
            const float2 pa = float2(p0.x, p0.y) - p;
            const float3 uv1 = cross(
                float3(p20.x, p10.x, pa.x), float3(p20.y, p10.y, pa.y));
            const float3 bary = (1.0 / uv1.z) *
                                float3(uv1.z - (uv1.x + uv1.y), uv1.y, uv1.x);

            if ((bary.x >= 0.f) && (bary.y >= 0.f) && (bary.z >= 0.f)) {
                const float z = p0.z * bary.x + p1.z * bary.y + p2.z * bary.z;

                write_pixel(
                    g_rw_textures2D_uint64[ubo.out_.output_texture_uav],
                    meshlet_index,
                    triangle_index,
                    uint2(x, y),
                    z);
            }
        }
    }
}

///
groupshared uint3 g_indices[128];
groupshared float3 g_vertices[64];

///
[numthreads(128, 1, 1)] void main(const Input input) {
    const GPURasterizeUBO ubo = tundra::load_ubo<GPURasterizeUBO>();

    const VisibleMeshlet visible_meshlet = tundra::buffer_load<false, VisibleMeshlet>(
        ubo.in_.visible_meshlets_srv, 0, input.meshlet_index);
    const InstanceTransform instance_transform =
        tundra::buffer_load<false, InstanceTransform>(
            ubo.in_.mesh_instance_transforms_srv,
            0,
            visible_meshlet.instance_transform_index);
    const MeshDescriptor mesh_descriptor = tundra::buffer_load<false, MeshDescriptor>(
        ubo.in_.mesh_descriptors_srv, 0, visible_meshlet.mesh_descriptor_index);
    const Meshlet meshlet = mesh_descriptor.get_meshlet(visible_meshlet.meshlet_index);

    /////////////////////////////////////////////////////////////////////////////////////
    // Load indices
    {
        const uint triangle_index = input.triangle_index;
        if (triangle_index < meshlet.triangle_count) {
            g_indices[triangle_index] = tundra::buffer_load<true, uint3>(
                mesh_descriptor.mesh_data_buffer_srv,
                mesh_descriptor.meshlet_triangles_offset +
                    (meshlet.triangle_offset * sizeof(uint)),
                triangle_index);
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////
    // Load and transform vertices
    {
        const uint vertex_index = input.triangle_index;
        if (vertex_index < meshlet.vertex_count) {
            float3 vertex = mesh_descriptor.get_vertex(meshlet, vertex_index);

            vertex = (quat_rotate_vector(instance_transform.quat, vertex) *
                      instance_transform.scale) +
                     instance_transform.position;
            float4 transformed_vertex = mul(ubo.world_to_clip, float4(vertex, 1.f));
            transformed_vertex.y *= -1.f;

            const float3 subpixel = transformed_vertex.xyz / transformed_vertex.w;
            const float2 xy = round(
                                  ((subpixel.xy * 0.5f) + 0.5f) * (ubo.view_size) *
                                  float(SUBPIXEL_SAMPLES)) /
                                  float(SUBPIXEL_SAMPLES) -
                              0.5f;

            g_vertices[vertex_index] = float3(xy, subpixel.z);
        }
    }

    DeviceMemoryBarrierWithGroupSync();

    /////////////////////////////////////////////////////////////////////////////////////
    // Rasterize
    if (input.triangle_index < meshlet.triangle_count) {
        const uint3 indices = g_indices[input.triangle_index];
        rasterize(
            ubo,
            input.meshlet_index,
            input.triangle_index,
            g_vertices[indices[0]],
            g_vertices[indices[1]],
            g_vertices[indices[2]]);
    }
}
