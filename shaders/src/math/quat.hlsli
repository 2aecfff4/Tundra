#ifndef TNDR_SHADERS_MATH_QUAT_H
#define TNDR_SHADERS_MATH_QUAT_H

///
float3 quat_rotate_vector(const float4 quat, const float3 vec)
{
    const float3 quat_xyz = quat.xyz;
    const float3 uv = cross(quat_xyz, vec);
    const float3 uuv = cross(quat_xyz, uv);

    return vec + ((uv * quat.w) + uuv) * 2.f;
}

#endif // TNDR_SHADERS_MATH_QUAT_H
