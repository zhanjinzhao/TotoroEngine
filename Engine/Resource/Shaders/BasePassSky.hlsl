#include "Common.hlsl"

TextureCube SkyCubeTexture;

struct VertexIn
{
    float3 PosL    : POSITION;
    float3 NormalL : NORMAL;
    float2 TexC    : TEXCOORD;
    float3 TangentU : TANGENT;
};

struct VertexOut
{
	float4 PosH : SV_POSITION;
    float3 PosL : POSITION;
};

struct PixelOutput
{
    float4 BaseColor    : SV_TARGET0;
    float4 Normal		: SV_TARGET1;
    float4 WorldPos		: SV_TARGET2;
    float4 OcclusionRoughnessMetallic	: SV_TARGET3;
};
 
VertexOut VS(VertexIn vin)
{
	VertexOut Out;

	// Use local vertex position as cubemap lookup vector.
	Out.PosL = vin.PosL;	
	
	// Remove translation from the view matrix
	float4x4 View = gView;
	View[3][0] = View[3][1] = View[3][2] = 0.0f;
	
	float4 PosH = mul(mul(float4(vin.PosL, 1.0f), View), gProj);
	
	// Set z = w so that z/w = 1 (i.e., skydome always on far plane).
	Out.PosH = PosH.xyww;
		
	return Out;
}

PixelOutput PS(VertexOut pin) //: SV_Target
{
	PixelOutput Out;
	
	MaterialData MatData = cbMaterialData;
	
	// BaseColor
	Out.BaseColor = SkyCubeTexture.Sample(gsamLinearWrap, pin.PosL);
	
	// ShadingModel
	Out.WorldPos.a = (float)MatData.ShadingModel / (float)0xF;
	
	// Unuse
    Out.Normal = 0.0f;
    Out.OcclusionRoughnessMetallic = 0.0f;
    Out.WorldPos.rbg = 0.0f;

	return Out;
}

