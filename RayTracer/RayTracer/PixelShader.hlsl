
struct PSIn
{
    float4 Position : SV_POSITION;
    float3 Color : COLOR;
};

float4 main(PSIn input) : SV_TARGET
{
    return float4(input.Color, 1.0f);
}