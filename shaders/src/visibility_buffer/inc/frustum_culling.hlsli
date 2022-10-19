#ifndef TNDR_MESHLET_RENDERER_INC_FRUSTUM_CULLING_H
#define TNDR_MESHLET_RENDERER_INC_FRUSTUM_CULLING_H
#include "defines.hlsli"

///
bool frustum_culling(const float4 planes[6], const float3 center, const float radius)
{
    bool is_visible = true;
    UNROLL
    for (uint i = 0; i < 6; ++i) {
        const float dot = planes[i].x * center.x + //
                          planes[i].y * center.y + //
                          planes[i].z * center.z + //
                          planes[i].w - radius;
        is_visible &= dot < 0.f;
    }

    return is_visible;
}

#endif // TNDR_MESHLET_RENDERER_INC_FRUSTUM_CULLING_H
