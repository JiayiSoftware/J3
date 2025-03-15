struct PS_INPUT
{
    float4 inPos : SV_POSITION;
    float4 inColor : COLOR;
    float2 inTexCoord : TEXCOORD;
};

Texture2D tex : TEXTURE : register(t0);
SamplerState sam : SAMPLER : register(s0);

float4 main(PS_INPUT input) : SV_TARGET
{
    // TODO: use constant data to use either color or texture
    float3 color = tex.Sample(sam, input.inTexCoord).xyz;
    // float4 color = input.inColor;
    return float4(color, 1.0f);
}