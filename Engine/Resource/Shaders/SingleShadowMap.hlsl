#include "Common.hlsl"

struct VertexIn
{
	float3 PosL    : POSITION;
};

struct VertexOut
{
	float4 PosH    : SV_POSITION;
};

VertexOut VS(VertexIn vin)
{
	VertexOut Out = (VertexOut)0.0f;
	
    // Transform to world space.
    float4 PosW = mul(float4(vin.PosL, 1.0f), gWorld);

    // Transform to homogeneous clip space.
    Out.PosH = mul(PosW, gViewProj);
	
    return Out;
}


void PS(VertexOut pin) 
{
	// Nothing
}


