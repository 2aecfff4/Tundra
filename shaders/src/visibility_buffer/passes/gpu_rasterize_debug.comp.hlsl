#include "bindings.hlsli"
#include "defines.hlsli"
#include "hash.hlsli"
#include "math/quat.hlsli"
#include "templates.hlsli"

///
struct GpuRasterizeDebugUBO {
    uint2 size;
    uint input_srv;
    uint output_uav;
};

///
[numthreads(16, 16, 1)] void main(uint3 dispatch_id
                                  : SV_DispatchThreadID) {
    const GpuRasterizeDebugUBO ubo = tundra::load_ubo<GpuRasterizeDebugUBO>();

    if (any(dispatch_id.xy >= ubo.size)) {
        return;
    }

    const Texture2D<uint64_t> input = g_textures2D_uint64[ubo.input_srv];
    RWTexture2D<float4> output = g_rw_textures2D_float4[ubo.output_uav];
    const uint64_t v = input[dispatch_id.xy] >> 32;

    const uint hash = hash_uint((uint)v);
    const float3 color = float3(
                             float(hash & 0xff),
                             float((hash >> 8) & 0xff),
                             float((hash >> 16) & 0xff)) /
                         255.0f;

    output[dispatch_id.xy] = float4(color, 1.f);
}
