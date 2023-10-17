#include "hash.hlsli"

struct VisibilityBufferFragInput {
    uint visible_meshlet_index : TEXCOORD0;
    uint triangle_index : SV_PrimitiveID;
};

struct VisibilityBufferFragOutput {
    float4 color : SV_Target;
};

VisibilityBufferFragOutput main(VisibilityBufferFragInput input)
{
    VisibilityBufferFragOutput output;

    const uint hash = hash_uint(input.triangle_index);
    const float3 color = float3(
                             float(hash & 0xff),
                             float((hash >> 8) & 0xff),
                             float((hash >> 16) & 0xff)) /
                         255.0f;

    output.color = float4(color, 1.f);
    return output;
}
