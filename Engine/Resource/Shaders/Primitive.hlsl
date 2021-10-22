//***************************************************************************************
// Default.hlsl by Frank Luna (C) 2015 All Rights Reserved.
//
// Default shader, currently supports lighting.
//***************************************************************************************

#include "Common.hlsl"

struct VertexIn
{
    float3 PosW  : POSITION;
    float4 Color : COLOR;
};

struct VertexOut
{
    float4 PosH  : SV_POSITION;
    float4 Color : COLOR;
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
    VertexOut Out = (VertexOut)0.0f;

    // Transform to homogeneous clip space.
    Out.PosH = mul(float4(vin.PosW, 1.0f), gViewProj);
	
	Out.Color = vin.Color;

    return Out;
}

PixelOutput PS(VertexOut pin) //: SV_Target
{
	PixelOutput Out;
    
    // BaseColor
	Out.BaseColor = pin.Color;
	
	// ShadingModel
    float ShadingModel = 1.0f; // Unlit
	Out.WorldPos.a = ShadingModel / (float)0xF;
    
    // Unuse
    Out.Normal = 0.0f;
    Out.OcclusionRoughnessMetallic = 0.0f;
    Out.WorldPos.rbg = 0.0f;
    
    return Out;
}


