#ifndef __SHADER_SHADOW__
#define __SHADER_SHADOW__


#include "Common.hlsl"
#include "LightingUtil.hlsl"
#include "SDFShared.hlsl"

#define MAX_SHADOW_MAP_2D_NUM  10
Texture2D ShadowMaps[MAX_SHADOW_MAP_2D_NUM]; 

#define MAX_SHADOW_MAP_CUBE_NUM  5
TextureCube ShadowMapCubes[MAX_SHADOW_MAP_CUBE_NUM];

StructuredBuffer<LightParameters> Lights; 


#define BLOCKER_SEARCH_SAMPLE_COUNT    10
#define BLOCKER_SEARCH_PIXEL_RADIUS    5.0f 

#define PCF_SAMPLE_COUNT           30
#define PCF_SAMPLE_PIXLE_RADIUS    5



// Ref:https://developer.amd.com/wordpress/media/2012/10/Isidoro-ShadowMapping.pdf
float2 ComputeDepthDerivative(float3 projCoords)
{
	float2 ddist_duv = 0.0f;
    
	// Packing derivatives of u,v, and distance to light source w.r.t. screen space x, and y
	float3 duvdist_dx = ddx(projCoords);
	float3 duvdist_dy = ddy(projCoords);
	
	// Invert texture Jacobian and use chain rule to compute ddist/du and ddist/dv
	// |ddist/du| = |du/dx du/dy|-T * |ddist/dx|
	// |ddist/dv| |dv/dx dv/dy| |ddist/dy|

	// Multiply ddist/dx and ddist/dy by inverse transpose of Jacobian
	float invDet = 1 / ((duvdist_dx.x * duvdist_dy.y) - (duvdist_dx.y * duvdist_dy.x));

	// Top row of 2x2
	ddist_duv.x = duvdist_dy.y * duvdist_dx.z; 
	ddist_duv.x -= duvdist_dx.y * duvdist_dy.z; 

	// Bottom row of 2x2
	ddist_duv.y = duvdist_dx.x * duvdist_dy.z; 
	ddist_duv.y -= duvdist_dy.x * duvdist_dx.z; 
	ddist_duv *= invDet;
    
	return ddist_duv;
}

float BlockerSearch(float3 ReceiverPos, float dx, uint ShadowMapIdx, float2 ddist_duv)
{
    float AverageBlockerDepth = 0.0f;
    int BlockerCount = 0;
	
	const int SampleCount = BLOCKER_SEARCH_SAMPLE_COUNT;
	const float SearchWidth = BLOCKER_SEARCH_PIXEL_RADIUS * dx;
	for(int i = 0; i < SampleCount; i++)
	{
		float2 UVOffset = (Hammersley(i, SampleCount) * 2.0f - 1.0f) * SearchWidth;
			
		float ReceiverDepthBias = ReceiverPos.z + dot(ddist_duv, UVOffset);
            
		float2 SampleUV = ReceiverPos.xy + UVOffset;
		float BlockerDepth = ShadowMaps[ShadowMapIdx].SampleLevel(gsamPointClamp, SampleUV, 0).r; // Important: don't use anisotropic sampler!
                     
		if (BlockerDepth < ReceiverDepthBias)
		{
			AverageBlockerDepth += BlockerDepth;
			BlockerCount++;			
		}
	}
	
	if (BlockerCount > 0)
	{
		AverageBlockerDepth /= BlockerCount;
	}
	else
	{
		AverageBlockerDepth = -1.0f;
	}

	return AverageBlockerDepth;
}

float PCF(float3 ReceiverPos, float UVRadius, float2 ddist_duv, uint ShadowMapIdx)
{
	const int SampleCount = PCF_SAMPLE_COUNT;
    float Visibility = 0.0f;
    for (int i = 0; i < SampleCount; i++)
    {
		float2 UVOffset = (Hammersley(i, SampleCount) * 2.0f - 1.0f) * UVRadius;
		float2 SampleUV = ReceiverPos.xy + UVOffset;
		
		const float FixedBias = 0.003f;
		float ReceiverDepthBias = ReceiverPos.z + dot(ddist_duv, UVOffset) - FixedBias;
		
		Visibility += ShadowMaps[ShadowMapIdx].SampleCmpLevelZero(gsamShadow, SampleUV, ReceiverDepthBias).r;
	}
    
	return Visibility / SampleCount;
}

float PCSS(float3 ReceiverPos, float dx, float2 ddist_duv, uint ShadowMapIdx, float4x4 LightProj, bool bPerspectiveView, float LightPixelWidth)
{	
	float ReceiverDepth = ReceiverPos.z;
	
	// Blocker search
	float BlockerDepth = BlockerSearch(ReceiverPos, dx, ShadowMapIdx, ddist_duv);
	
	if (BlockerDepth < 0.0f)
	{
		return 1.0f;
	}

    // Penumbra estimation
	const float LightWidth = LightPixelWidth * dx;
	
	if(bPerspectiveView)
	{
		ReceiverDepth = NDCDepthToViewDepth(ReceiverDepth, LightProj);
		BlockerDepth = NDCDepthToViewDepth(BlockerDepth, LightProj);		
	}

	float PenumbraWidth = (ReceiverDepth - BlockerDepth) * LightWidth / BlockerDepth;
	
    // Percentage closer filtering
	return PCF(ReceiverPos, PenumbraWidth, ddist_duv, ShadowMapIdx);
}

float VSM(float3 ReceiverPos, uint ShadowMapIdx)
{
	float2 SampleUV = ReceiverPos.xy;	
	float2 SampleValue = ShadowMaps[ShadowMapIdx].SampleLevel(gsamLinearClamp, SampleUV, 0).xy;
	float Mean = SampleValue.x;
		
	float ReceiverDepth = ReceiverPos.z;
	if (ReceiverDepth - 0.001 <= Mean)
	{
		return 1.0f;
	}
	
	float Variance = SampleValue.y - Mean * Mean;	
	float Diff = ReceiverDepth - Mean;	
	float Result = Variance / (Variance + Diff * Diff);
	
	return clamp(Result, 0.0f, 1.0f);
}

float SDF(float3 ReceiverPosW, float3 LightPosW, float TanLightAngle)
{
	float MinConeVisibility = 1.0f;
	
	for (uint i = 0; i < gObjectCount; i++)
	{
	    int SDFIndex = ObjectSDFDescriptors[i].SDFIndex;
		
		float3 LocalRayStart = mul(float4(ReceiverPosW, 1.0f), ObjectSDFDescriptors[i].ObjInvWorld).xyz;
		float3 LocalRayEnd = mul(float4(LightPosW, 1.0f), ObjectSDFDescriptors[i].ObjInvWorld).xyz;
		float3 LocalRayDir = LocalRayEnd - LocalRayStart;
		LocalRayDir = normalize(LocalRayDir);
		float LocalRayLength = length(LocalRayEnd - LocalRayStart);			
		
		float Extent = MeshSDFDescriptors[SDFIndex].Extent;
		int Resolution = MeshSDFDescriptors[SDFIndex].Resolution;
		float RcpExtent = rcp(Extent);
		
		float3 LocalExtent = float3(Extent, Extent, Extent);
		float2 IntersectionTimes = LineBoxIntersect(LocalRayStart, LocalRayEnd, -LocalExtent, LocalExtent);
		
		[branch]
		if (IntersectionTimes.x < IntersectionTimes.y && IntersectionTimes.x < 1)
		{	
			float SampleRayTime = IntersectionTimes.x * LocalRayLength;
			
			int Step = 0;			
			
			[loop]
			for (; Step < MAX_SDF_STEP; Step++)
			{
				float3 SampleLocalPosition = LocalRayStart + SampleRayTime * LocalRayDir;
				float3 ClampedSamplePosition = clamp(SampleLocalPosition, -LocalExtent, LocalExtent);
				float3 VolumeUV = (ClampedSamplePosition * RcpExtent) * 0.5f + 0.5f;
				float SDFWidth = Extent * 2.0f;
				float DistanceField = SampleMeshDistanceField(SDFIndex, VolumeUV, SDFWidth);

				// Don't allow occlusion within an object's self shadow distance
				float SelfShadowScale = 100.0f;
				float SelfShadowVisibility = 1 - saturate(SampleRayTime * SelfShadowScale);
				
				float SphereRadius = TanLightAngle * SampleRayTime;
				float StepConeVisibility = max(saturate(DistanceField / SphereRadius), SelfShadowVisibility);			
				MinConeVisibility = min(MinConeVisibility, StepConeVisibility);							
				
				float MinStepSize = 1.0f / (4 * MAX_SDF_STEP);
				float StepDistance = max(DistanceField, MinStepSize);
				SampleRayTime += StepDistance;
				
				// Terminate the trace if we reached a negative area or went past the end of the ray
				if (DistanceField < 0 || SampleRayTime > IntersectionTimes.y * LocalRayLength)
				{
					break;
				}
			}
		}
	}
	
	if(MinConeVisibility > 0.99f)
	{
		return 1.0f;
	}
	
	return MinConeVisibility;
}

float CalcVisibility(float4 ShadowPosH, uint ShadowMapIdx, float4x4 LightProj, bool bPerspectiveView, float LightPixelWidth, float3 ReceiverPosW, float3 LightPosW, float TanLightAngle)
{
#if !USE_SDF
	if(ShadowMapIdx == -1)
	{
		return 1.0f;
	}
#endif
	
    // Complete projection by doing division by w.
    ShadowPosH.xyz /= ShadowPosH.w;

    // NDC space.
	float3 ReceiverPos = ShadowPosH.xyz;	
	if (ReceiverPos.x < 0.0f || ReceiverPos.x > 1.0f || ReceiverPos.y < 0.0f || ReceiverPos.y > 1.0f)
	{
		return 0.0f;
	}

    uint Width = 0, Height = 0, NumMips = 0;
    ShadowMaps[ShadowMapIdx].GetDimensions(0, Width, Height, NumMips);

    // Texel size.
    float dx = 1.0f / (float)Width;
    
	// ddist_duv
	float2 ddist_duv = ComputeDepthDerivative(ReceiverPos);
	
#if USE_PCSS
	return PCSS(ReceiverPos, dx, ddist_duv, ShadowMapIdx, LightProj, bPerspectiveView, LightPixelWidth);
#elif USE_VSM
	return VSM(ReceiverPos, ShadowMapIdx);
#elif USE_SDF
	return SDF(ReceiverPosW, LightPosW, TanLightAngle);
#else
	return PCF(ReceiverPos, PCF_SAMPLE_PIXLE_RADIUS * dx, ddist_duv, ShadowMapIdx);
#endif
	
	
}


float CalcVisibilityOmni(float3 LightToPoint, uint ShadowMapIdx, float FarZ)
{
	if(ShadowMapIdx == -1)
	{
		return 1.0f;
	}
	
	// Sample shadow cube map
	float ClosestDepth = ShadowMapCubes[ShadowMapIdx].SampleLevel(gsamPointClamp, normalize(LightToPoint), 0).r;
	
	// It is currently in linear range between [0,1]. Re-transform back to original value
	ClosestDepth *= FarZ;
	
	// Depth of shader point
	float CurrentDepth = length(LightToPoint);
	
	// Compare depths
	float Bias = 0.05f;
	float Visibility = (CurrentDepth -  Bias > ClosestDepth ? 0.0 : 1.0);
	
	return Visibility;
}

#endif //__SHADER_SHADOW__
