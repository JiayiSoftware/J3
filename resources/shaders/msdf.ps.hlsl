cbuffer CONSTANT_DATA : register(b0)
{
    float hasColor; // inherited from cb_pixel
};

struct PS_INPUT
{
    float4 inPos : SV_POSITION;
    float4 inColor : COLOR;
    float2 inTexCoord : TEXCOORD;
};

Texture2D tex : TEXTURE : register(t0);
SamplerState sam : SAMPLER : register(s0);

float median(float x, float y, float z)
{
    return max(min(x, y), min(max(x, y), z));
}

float screenPxRange(float2 texCoord)
{
    float pxRange = 6; // from msdf-atlas-gen command in CMakeLists.txt

    float2 textureSize;
    tex.GetDimensions(textureSize.x, textureSize.y);
	float2 unitRange = float2(pxRange, pxRange) / textureSize;

    float2 screenTexSize = float2(1, 1) / fwidth(texCoord);
    
    return max(0.5 * dot(unitRange, screenTexSize), 1);
}

float4 main(PS_INPUT input) : SV_TARGET
{
    float3 msd = tex.Sample(sam, input.inTexCoord).xyz;
    float sd = median(msd.x, msd.y, msd.z);
	float dist = screenPxRange(input.inTexCoord) * (sd - 0.5);
    float opacity = clamp(dist + 0.5, 0, 1);
	return float4(input.inColor.xyz, opacity);
}