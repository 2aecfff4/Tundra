struct FullscreenClearVertOutput {
    float4 position : SV_Position;
};

FullscreenClearVertOutput main(const uint vertex_id : SV_VertexID)
{
    const float2 uv = float2((vertex_id << 1) & 2, vertex_id & 2);

    FullscreenClearVertOutput output;
    output.position = float4((uv * 2.0f) + -1.0f, 0.0f, 1.0f);
    return output;
}
