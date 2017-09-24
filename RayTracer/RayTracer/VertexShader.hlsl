
struct VSOut
{
    float4 Position : SV_POSITION;
    float3 Color : COLOR;
};


VSOut main( float2 pos : POSITION, float3 Color : COLOR)
{
	// Pass through shader
    VSOut output;
    output.Position.xy = pos.xy;
    output.Position.zw = float2(1, 1);
    output.Color = Color;
    return output;
}