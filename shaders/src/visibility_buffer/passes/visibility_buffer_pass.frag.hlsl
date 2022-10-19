#include "hash.hlsli"
#include "visibility_buffer/inc/unpacked_index.hlsli"

struct VisibilityBufferFragInput {
    float2 uv : TEXCOORD0;
    uint visible_meshlet_index : TEXCOORD1;
    uint meshlet_index : TEXCOORD2;
};

struct VisibilityBufferFragOutput {
    float4 color : SV_Target;
};

VisibilityBufferFragOutput main(VisibilityBufferFragInput input)
{
    VisibilityBufferFragOutput output;

    // const uint hash = hash_uint(input.meshlet_index);
    // const float3 color = float3(
    //                          float(hash & 0xff),
    //                          float((hash >> 8) & 0xff),
    //                          float((hash >> 16) & 0xff)) /
    //                      255.0;
    // output.color = float4(color, 1.f);
    output.color = float4(input.uv, 0.f, 1.f);
    return output;
}
