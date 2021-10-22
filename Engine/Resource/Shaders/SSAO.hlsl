//#include "Shadows.hlsl"
//#include "PBRLighting.hlsl"

#include "Common.hlsl"

Texture2D NormalGbuffer;
Texture2D DepthGbuffer;

static const int gSampleCount = 16;

cbuffer cbSSAO
{
    float4x4 gProjTex;

    // Coordinates given in view space.
    float    gOcclusionRadius;
    float    gOcclusionFadeStart;
    float    gOcclusionFadeEnd;
    float    gSurfaceEpsilon;
};

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
    VertexOut vout = (VertexOut)0.0f;

	// Already in homogeneous clip space.
	vout.PosH = float4(vin.PosL, 1.0f);

    vout.TexC = vin.TexC;

    return vout;
}

float NdcDepthToViewDepth(float z_ndc)
{
    // z_ndc = A + B/viewZ, where gProj[2,2]=A and gProj[3,2]=B.
    float viewZ = gProj[3][2] / (z_ndc - gProj[2][2]);
    return viewZ;
}

// Determines how much the point R occludes the point P as a function of DistZ.
float OcclusionFunction(float DistZ)
{
	//
	//       1.0     -------------\
	//               |           |  \
	//               |           |    \
	//               |           |      \ 
	//               |           |        \
	//               |           |          \
	//               |           |            \
	//  ------|------|-----------|-------------|---------|--> zv
	//        0     Eps       Start           End       
	//
	
	float Occlusion = 0.0f;
	if(DistZ > gSurfaceEpsilon)
	{
		float FadeLength = gOcclusionFadeEnd - gOcclusionFadeStart;	
		Occlusion = saturate( (gOcclusionFadeEnd - DistZ) / FadeLength );
	}
	
	return Occlusion;	
}

float4 PS(VertexOut pin) : SV_TARGET
{	
	// Get viewspace normal
	float3 NormalW = NormalGbuffer.SampleLevel(gsamPointClamp, pin.TexC, 0).rgb;
	float3 NormalV = mul(NormalW, (float3x3)gView);
	NormalV = normalize(NormalV);
	
	// Get viewspace P
	float Pz = DepthGbuffer.SampleLevel(gsamDepthMap, pin.TexC, 0.0f).r;	
	float3 P = UVToView(pin.TexC, Pz, gInvProj).xyz;
	
	float Occlusion = 0.0f;
	
	// Sample neighboring points about P in the hemisphere oriented by normal.
	const float Phi = PI * (3.0f - sqrt(5.0f));
	for(int i = 0; i < gSampleCount; ++i)
	{
		// Fibonacci lattices.
		float3 Offset;
		Offset.y = 1 - (i / float(gSampleCount - 1)) * 2;  // y goes from 1 to - 1
		float Radius = sqrt(1.0f - Offset.y * Offset.y);  // Radius at y
		float Theta = Phi * i;    // Golden angle increment
		Offset.x = Radius * cos(Theta);
		Offset.z = Radius * sin(Theta);
	
		// Flip offset if it is behind P.
		float Flip = sign(dot(Offset, NormalV));
		
		// Sample point Q
		float3 Q = P + Flip * gOcclusionRadius * Offset;
		
		// Get potential occluder R
		float2 UV = ViewToUV(float4(Q, 1.0f), gProj);
		float Rz = DepthGbuffer.SampleLevel(gsamDepthMap, UV, 0.0f).r;
		float3 R = UVToView(UV, Rz, gInvProj).xyz;	

		// Test whether R occludes P.
		float AngleFactor = max(dot(NormalV, normalize(R - P)), 0.0f);
		float DistFactor = OcclusionFunction(P.z - R.z);	

		Occlusion += AngleFactor * DistFactor;
	}
	
	Occlusion /= gSampleCount;
	
	float AmbientAccess = 1.0f - Occlusion;

	// Sharpen the contrast of the SSAO map to make the SSAO affect more dramatic.
	return saturate(pow(AmbientAccess, 6.0f));
}