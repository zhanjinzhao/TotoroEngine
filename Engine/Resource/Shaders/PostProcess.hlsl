#include "Common.hlsl"

Texture2D ColorTexture;

struct VertexIn
{
    float3 PosL    : POSITION;
    float2 TexC    : TEXCOORD;
};

struct VertexOut
{
	float4 PosH  : SV_POSITION;
    float2 TexC  : TEXCOORD;
};

VertexOut VS(VertexIn vin)
{
    VertexOut Out = (VertexOut)0.0f;

	// Already in homogeneous clip space.
	Out.PosH = float4(vin.PosL, 1.0f);

    Out.TexC = vin.TexC;

    return Out;
}

float3 ACESToneMapping(float3 Color, float AdaptedLum)
{
	const float A = 2.51f;
	const float B = 0.03f;
	const float C = 2.43f;
	const float D = 0.59f;
	const float E = 0.14f;

	Color *= AdaptedLum;
	return (Color * (A * Color + B)) / (Color * (C * Color + D) + E);
}

float4 PS(VertexOut pin) : SV_TARGET
{	
	float4 Output;
	
	float3 Color = ColorTexture.SampleLevel(gsamPointClamp, pin.TexC, 0).rgb;
	
	// ToneMapping
	Color = ACESToneMapping(Color, 1.0f); 
	
	// Gamma correction
	float Gamma = 2.2;
    float3 CorrectColor = pow(Color, 1.0 / Gamma);	
	
	Output = float4(CorrectColor, 1.0f);
	
	return Output;
}