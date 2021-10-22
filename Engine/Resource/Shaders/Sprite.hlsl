#include "Common.hlsl"

cbuffer cbSpritePass
{
    float4x4 gScreenToNDC;
};


Texture2D spriteTexture;


struct VertexIn
{
    float3 PosS  : POSITION;
    float2 TexC  : TEXCOORD;
};

struct VertexOut
{
    float4 PosH  : SV_POSITION;
    float2 TexC  : TEXCOORD;
};

VertexOut VS(VertexIn vin)
{
    VertexOut vout = (VertexOut)0.0f;

    // Transform to homogeneous clip space.
    vout.PosH = mul(float4(vin.PosS, 1.0f), gScreenToNDC);
	
	vout.TexC = vin.TexC;

    return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
    float4 Color = spriteTexture.Sample(gsamAnisotropicWrap, pin.TexC);

    return Color;
}


