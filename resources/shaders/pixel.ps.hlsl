cbuffer CONSTANT_DATA : register(b0)
{
    float hasColor; // set to 1 for full color, 0 for texture
};

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
    if (hasColor > 0.5f)
    {
        return input.inColor;
    }

    float3 color = tex.Sample(sam, input.inTexCoord);
    return float4(color, input.inColor.w);
}