// This shader convert the equirectangular map to cube map

#include "Common.hlsl"

Texture2D EquirectangularMap;

struct VertexIn
{
	float3 PosL    : POSITION;
	float3 NormalL : NORMAL;
	float2 TexC    : TEXCOORD;
	float3 TangentU : TANGENT;
};

struct VertexOut
{
	float4 PosH		: SV_POSITION;
	float3 PosL		: POSITION;
};


VertexOut VS(VertexIn vin)
{
	VertexOut Out;

	// Use local vertex position as cubemap lookup vector.
	Out.PosL = vin.PosL;
	
	// Remove translation from the view matrix
	float4x4 View = gView;
	View[3][0] = View[3][1] = View[3][2] = 0.0f;
	
	Out.PosH = mul(mul(float4(vin.PosL, 1.0f), View), gProj);
		
	return Out;
}


float2 SampleSphericalMap(float3 v)
{
	const float2 InvAtan = float2(0.1591, 0.3183);
	
    float2 UV = float2(atan2(v.z, v.x), asin(v.y));
    UV *= InvAtan;
    UV += 0.5;
    return UV;
}

float4 PS(VertexOut pin) : SV_TARGET
{
	float2 UV = SampleSphericalMap(normalize(pin.PosL));
	
	float3 Color = EquirectangularMap.SampleLevel(gsamLinearClamp, UV, 0).rgb;

	return float4(Color, 1.0f);
}