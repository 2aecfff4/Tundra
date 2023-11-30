#include "bindings.hlsli"
#include "defines.hlsli"
#include "hash.hlsli"
#include "math/quat.hlsli"
#include "templates.hlsli"
#include "utils/thread_group_tiling.hlsli"
#include "visibility_buffer/inc/commands.hlsli"
#include "visibility_buffer/inc/frustum_culling.hlsli"
#include "visibility_buffer/inc/instance_transform.hlsli"
#include "visibility_buffer/inc/mesh_descriptor.hlsli"
#include "visibility_buffer/inc/mesh_instance.hlsli"
#include "visibility_buffer/inc/unpacked_index.hlsli"
#include "visibility_buffer/inc/visible_mesh_instance.hlsli"
#include "visibility_buffer/inc/visible_meshlet.hlsli"

float3 agxDefaultContrastApprox(const float3 x)
{
    const float3 x2 = x * x;
    const float3 x4 = x2 * x2;

    return +15.5 * x4 * x2  //
           - 40.14 * x4 * x //
           + 31.96 * x4     //
           - 6.868 * x2 * x //
           + 0.4298 * x2    //
           + 0.1191 * x     //
           - 0.00232;
}

float3 agx(float3 val)
{
    const float3x3 agx_mat = float3x3(
        0.842479062253094,
        0.0423282422610123,
        0.0423756549057051,
        0.0784335999999992,
        0.878468636469772,
        0.0784336,
        0.0792237451477643,
        0.0791661274605434,
        0.879142973793104);

    const float min_ev = -12.47393f;
    const float max_ev = 4.026069f;

    // Input transform
    // val = agx_mat * val;
    val = mul(agx_mat, val);

    // Log2 space encoding
    val = clamp(log2(val), min_ev, max_ev);
    val = (val - min_ev) / (max_ev - min_ev);

    // Apply sigmoid function approximation
    val = agxDefaultContrastApprox(val);

    return val;
}

float3 agxEotf(float3 val)
{
    const float3x3 agx_mat_inv = float3x3(
        1.19687900512017,
        -0.0528968517574562,
        -0.0529716355144438,
        -0.0980208811401368,
        1.15190312990417,
        -0.0980434501171241,
        -0.0990297440797205,
        -0.0989611768448433,
        1.15107367264116);

    // Undo input transform
    // val = agx_mat_inv * val;
    val = mul(agx_mat_inv, val);

    // sRGB IEC 61966-2-1 2.2 Exponent Reference EOTF Display
    //val = pow(val, float3(2.2));

    return val;
}

#define AGX_LOOK 2

float3 agxLook(float3 val)
{
    const float3 lw = float3(0.2126, 0.7152, 0.0722);
    float luma = dot(val, lw);

    // Default
    float3 offset = (float3)(0.0);
    float3 slope = (float3)(1.0);
    float3 power = (float3)(1.0);
    float sat = 1.0;

#if AGX_LOOK == 1
    // Golden
    slope = float3(1.0, 0.9, 0.5);
    power = (float3)(0.8);
    sat = 0.8;
#elif AGX_LOOK == 2
    // Punchy
    slope = (float3)(1.0);
    power = float3(1.35, 1.35, 1.35);
    sat = 1.4;
#endif

    // ASC CDL
    val = pow(val * slope + offset, power);
    return luma + sat * (val - luma);
}

///
struct MaterialUbo {
    float4x4 world_to_clip;
    float4 camera_position;
    float4 light_position;
    float4 diffuse_color;
    float4 light_color;

    uint2 view_size;

    struct {
        uint mesh_descriptors_srv;
        uint mesh_instance_transforms_srv;
        uint visible_meshlets_srv;
        uint visibility_buffer_srv;
    } in_;

    struct {
        uint out_color_uav;
    } out_;
};

///
struct Input {
    uint2 thread_id : SV_DispatchThreadID;
    uint2 group_thread_id : SV_GroupThreadID;
    uint2 group_id : SV_GroupID;
};

inline float3 get_column(float3x3 M, const uint i)
{
    return float3(M[0][i], M[1][i], M[2][i]);
}

struct BarycentricDeriv {
    float3 m_lambda;
    float3 m_ddx;
    float3 m_ddy;

    static BarycentricDeriv from(
        const float4 p0,
        const float4 p1,
        const float4 p2,
        const float2 pixel_ndc,
        const float2 view_size)
    {
        BarycentricDeriv ret = (BarycentricDeriv)0;

        const float3 inv_w = rcp(float3(p0.w, p1.w, p2.w));

        const float2 ndc0 = p0.xy * inv_w.x;
        const float2 ndc1 = p1.xy * inv_w.y;
        const float2 ndc2 = p2.xy * inv_w.z;

        const float inv_det = rcp(determinant(float2x2(ndc2 - ndc1, ndc0 - ndc1)));
        ret.m_ddx = float3(ndc1.y - ndc2.y, ndc2.y - ndc0.y, ndc0.y - ndc1.y) * inv_det *
                    inv_w;
        ret.m_ddy = float3(ndc2.x - ndc1.x, ndc0.x - ndc2.x, ndc1.x - ndc0.x) * inv_det *
                    inv_w;
        float ddx_sum = dot(ret.m_ddx, float3(1, 1, 1));
        float ddy_sum = dot(ret.m_ddy, float3(1, 1, 1));

        const float2 delta_vec = pixel_ndc - ndc0;
        const float interp_inv_w = inv_w.x + delta_vec.x * ddx_sum +
                                   delta_vec.y * ddy_sum;
        float interp_w = rcp(interp_inv_w);

        ret.m_lambda.x = interp_w * (inv_w[0] + delta_vec.x * ret.m_ddx.x +
                                     delta_vec.y * ret.m_ddy.x);
        ret.m_lambda.y = interp_w *
                         (0.0f + delta_vec.x * ret.m_ddx.y + delta_vec.y * ret.m_ddy.y);
        ret.m_lambda.z = interp_w *
                         (0.0f + delta_vec.x * ret.m_ddx.z + delta_vec.y * ret.m_ddy.z);

        ret.m_ddx *= (2.0f / view_size.x);
        ret.m_ddy *= (2.0f / view_size.y);
        ddx_sum *= (2.0f / view_size.x);
        ddy_sum *= (2.0f / view_size.y);

        // ret.m_ddy *= -1.0f;
        // ddy_sum *= -1.0f;

        const float interp_w_ddx = 1.0f / (interp_inv_w + ddx_sum);
        const float interp_w_ddy = 1.0f / (interp_inv_w + ddy_sum);

        ret.m_ddx = interp_w_ddx * (ret.m_lambda * interp_inv_w + ret.m_ddx) -
                    ret.m_lambda;
        ret.m_ddy = interp_w_ddy * (ret.m_lambda * interp_inv_w + ret.m_ddy) -
                    ret.m_lambda;

        return ret;
    }

    float3 interpolate(const float3x3 attributes)
    {
        const float3 attr0 = get_column(attributes, 0);
        const float3 attr1 = get_column(attributes, 1);
        const float3 attr2 = get_column(attributes, 2);

        return float3(
            dot(attr0, this.m_lambda),
            dot(attr1, this.m_lambda),
            dot(attr2, this.m_lambda));
    }
};

float v_smith_ggx_correlated(
    const float NoV, //
    const float NoL,
    const float roughness)
{
    const float a2 = roughness * roughness;
    const float lambda_l = NoV * sqrt((-NoL * a2 + NoL) * NoL + a2);
    const float lambda_v = NoL * sqrt((-NoV * a2 + NoV) * NoV + a2);
    return 0.5 / (lambda_v + lambda_l);
}

#define PI (3.14159265359)

/// Normal distribution function
float d_ggx(const float NoH, const float roughness)
{
    const float a2 = roughness * roughness;
    const float f = (NoH * a2 - NoH) * NoH + 1.0;
    return a2 / (PI * f * f);
}

float pow5(const float x)
{
    const float x2 = x * x;
    return x2 * x2 * x;
}

/// Fresnel, specular F
/// Schlick 1994, "An Inexpensive BRDF Model for Physically-Based Rendering"
float3 f_schlick(const float3 f0, const float f90, const float VoH)
{
    return f0 + (f90 - f0) * pow5(1.0 - VoH);
}

float3 brdf(
    const float3 normal,
    const float3 view_direction,
    const float3 light_direction,
    const float3 light_color,
    const float3 diffuse_color,
    const float metallic,
    const float roughness)
{
    const float3 half_vector = normalize(view_direction + light_direction);
    const float NoL = clamp(dot(normal, light_direction), 0.0, 1.0);
    const float NoH = clamp(dot(normal, half_vector), 0.0, 1.0);
    const float LoH = clamp(dot(light_direction, half_vector), 0.0, 1.0);
    const float NoV = abs(dot(normal, view_direction)) + 1e-5;

    const float roughness2 = roughness * roughness;
    const float3 f0 = lerp(float3(0.04, 0.04, 0.04), diffuse_color, metallic);
    const float f90 = saturate(dot(f0, (float3)(50.0 * 0.33)));

    const float D = d_ggx(NoH, roughness);
    const float3 F = f_schlick(f0, f90, LoH);
    const float V = v_smith_ggx_correlated(NoV, NoL, roughness);

    // specular BRDF
    const float3 Fr = (D * V) * F;

    // diffuse BRDF
    const float3 Fd = diffuse_color;

    float3 color = Fr + Fd;
    return (color * light_color) * NoL;
}

///
[numthreads(8, 8, 1)] void main(const Input input) {
    const MaterialUbo ubo = tundra::load_ubo<MaterialUbo>();

    const uint2 pixel_pos = input.thread_id;

    if (any(pixel_pos >= ubo.view_size)) {
        return;
    }
    const uint64_t v = g_rw_textures2D_uint64[ubo.in_.visibility_buffer_srv][pixel_pos];
    const uint meshlet_triangle = (uint)(v & 0xFFFFFFFF);

    if (meshlet_triangle == 0xFFFFFFFF) {
        RWTexture2D<float4> output = g_rw_textures2D_float4[ubo.out_.out_color_uav];
        output[pixel_pos] = float4(0, 0, 0, 1);
        return;
    }

    const UnpackedIndex unpacked_index = UnpackedIndex::create(meshlet_triangle);

    const VisibleMeshlet visible_meshlet = tundra::buffer_load<false, VisibleMeshlet>(
        ubo.in_.visible_meshlets_srv, 0, unpacked_index.meshlet_id);
    const InstanceTransform instance_transform =
        tundra::buffer_load<false, InstanceTransform>(
            ubo.in_.mesh_instance_transforms_srv,
            0,
            visible_meshlet.instance_transform_index);
    const MeshDescriptor mesh_descriptor = tundra::buffer_load<false, MeshDescriptor>(
        ubo.in_.mesh_descriptors_srv, 0, visible_meshlet.mesh_descriptor_index);
    const Meshlet meshlet = mesh_descriptor.get_meshlet(visible_meshlet.meshlet_index);

    const uint3 triangle_indices = tundra::buffer_load<true, uint3>(
        mesh_descriptor.mesh_data_buffer_srv,
        mesh_descriptor.meshlet_triangles_offset +
            (meshlet.triangle_offset * sizeof(uint)),
        unpacked_index.vertex_id);

    float3 vertices[3] = {
        mesh_descriptor.get_vertex(meshlet, triangle_indices[0]),
        mesh_descriptor.get_vertex(meshlet, triangle_indices[1]),
        mesh_descriptor.get_vertex(meshlet, triangle_indices[2]),
    };
    float4 transformed_vertices[3];

    for (uint i = 0; i < 3; ++i) {
        vertices[i] = (quat_rotate_vector(instance_transform.quat, vertices[i]) *
                       instance_transform.scale) +
                      instance_transform.position;
        transformed_vertices[i] = mul(ubo.world_to_clip, float4(vertices[i], 1.f));
        transformed_vertices[i].y *= -1.f;
    }

    const float3 one_over_w = 1.0f / float3(
                                         transformed_vertices[0].w,
                                         transformed_vertices[1].w,
                                         transformed_vertices[2].w);

    const BarycentricDeriv barycentric = BarycentricDeriv::from(
        transformed_vertices[0] * one_over_w[0], //
        transformed_vertices[1] * one_over_w[1],
        transformed_vertices[2] * one_over_w[2],
        (((float2)pixel_pos / (float2)(ubo.view_size - 1)) - 0.5f) * 2.f,
        ubo.view_size);

    const float3 normal0 = mesh_descriptor.get_normal(meshlet, triangle_indices[0]);
    const float3 normal1 = mesh_descriptor.get_normal(meshlet, triangle_indices[1]);
    const float3 normal2 = mesh_descriptor.get_normal(meshlet, triangle_indices[2]);
    const float3x3 normals = {
        normal0 * one_over_w[0],
        normal1 * one_over_w[1],
        normal2 * one_over_w[2],
    };

    float3 normal = normalize(barycentric.interpolate(normals));

    const float3 view_direction = normalize(
        ubo.camera_position.xyz - instance_transform.position);
    const float3 light_direction = normalize(
        ubo.light_position.xyz - instance_transform.position);

    float3 color = brdf(
        normal, //
        view_direction,
        light_direction,
        ubo.light_color.xyz,
        ubo.diffuse_color.xyz,
        1.f,
        0.1f);

    RWTexture2D<float4> output = g_rw_textures2D_float4[ubo.out_.out_color_uav];
    if (true) {
        color = agx(color);
        color = agxLook(color);
        color = agxEotf(color);

        output[pixel_pos] = float4(color, 1);
    } else {
        output[pixel_pos] = float4(normal, 1);
    }
}
