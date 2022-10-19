
struct FullscreenClearOutput {
    float4 output : SV_Target;
};

FullscreenClearOutput main()
{
    FullscreenClearOutput output;
    output.output = float4(0.25, 0.25, 0.25, 1.0);
    return output;
}
