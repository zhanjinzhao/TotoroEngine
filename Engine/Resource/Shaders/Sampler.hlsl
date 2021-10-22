#ifndef __SHADER_SAMPLER__
#define __SHADER_SAMPLER__

#include "Utlis.hlsl"

SamplerState gsamPointWrap        : register(s0);
SamplerState gsamPointClamp       : register(s1);
SamplerState gsamLinearWrap       : register(s2);
SamplerState gsamLinearClamp      : register(s3);
SamplerState gsamAnisotropicWrap  : register(s4);
SamplerState gsamAnisotropicClamp : register(s5);
SamplerComparisonState gsamShadow : register(s6);
SamplerState gsamDepthMap         : register(s7);

float RadicalInverse_VdC(uint bits)
{
	bits = (bits << 16u) | (bits >> 16u);
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
	return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

float2 Hammersley(uint i, uint N)
{
	return float2(float(i) / float(N), RadicalInverse_VdC(i));
}

float4 ImportanceSampleGGX(float2 Xi, float3 N, float Roughness)
{
	float a = Roughness * Roughness;
	float a2 = a * a;

	float Phi = 2.0 * PI * Xi.x;
	float CosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a2 - 1.0) * Xi.y));
	float SinTheta = sqrt(1.0 - CosTheta * CosTheta);

	// From spherical coordinates to cartesian coordinates
	float3 H;
	H.x = cos(Phi) * SinTheta;
	H.y = sin(Phi) * SinTheta;
	H.z = CosTheta;

	// From tangent-space vector to world-space sample vector
	float3 Up = abs(N.z) < 0.999 ? float3(0.0, 0.0, 1.0) : float3(1.0, 0.0, 0.0);
	float3 TangentX = normalize(cross(Up, N));
	float3 TangentY = cross(N, TangentX);

	float3 SampleVec = TangentX * H.x + TangentY * H.y + N * H.z;
	SampleVec = normalize(SampleVec);
	
	// Calculate PDF
	float d = (CosTheta * a2 - CosTheta) * CosTheta + 1;
	float D = a2 / (PI * d * d);
	float PDF = D * CosTheta;
	
	return float4(SampleVec, PDF);
}

#endif //__SHADER_SAMPLER__