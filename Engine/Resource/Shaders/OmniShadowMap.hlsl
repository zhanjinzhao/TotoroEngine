#include "Common.hlsl"

struct VertexIn
{
	float3 PosL    : POSITION;
};

struct VertexOut
{
	float4 PosH    : SV_POSITION;
	float3 PosW    : POSITION;
};

VertexOut VS(VertexIn vin)
{
	VertexOut Out = (VertexOut)0.0f;
	
    // Transform to world space.
    float4 PosW = mul(float4(vin.PosL, 1.0f), gWorld);
	Out.PosW = PosW.xyz;
	
    // Transform to homogeneous clip space.
	Out.PosH = mul(PosW, gViewProj);

    return Out;
}


float PS(VertexOut pin) : SV_Depth
{	
	float3 LightPos = gEyePosW;
	
	// Get distance between the shading point and light source
	float LightDistance = length(pin.PosW - LightPos);
	
	// Map to [0,1] range by dividing by far plane
	LightDistance /= gFarZ;
		
	// Write this as modified depth
	return LightDistance;
}


