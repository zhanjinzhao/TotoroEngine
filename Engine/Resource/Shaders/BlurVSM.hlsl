//=============================================================================
// Performs a separable Guassian blur with a blur radius up to 5 pixels.
//=============================================================================
#include "Utlis.hlsl"

Texture2D<float2> InputTexture;
RWTexture2D<float2> OutputTexture;

cbuffer cbBlurSettings
{
	int gBlurRadius;
	
	// Support up to 11 blur weights.
	float w0;
	float w1;
	float w2;
	
	float w3;	
	float w4;
	float w5;
	float w6;
	
	float w7;	
	float w8;
	float w9;
	float w10;	
};


#define N 256
#define MAX_BLUR_RADIUS 5
#define CacheSize (N + 2 * MAX_BLUR_RADIUS)
groupshared float2 gCache[CacheSize];

[numthreads(N, 1, 1)]
void HorzBlurCS(int3 groupThreadID : SV_GroupThreadID,
				int3 dispatchThreadID : SV_DispatchThreadID)
{
	// Put in an array for each indexing.
	float Weights[11] = { w0, w1, w2, w3, w4, w5, w6, w7, w8, w9, w10 };

	//
	// Fill local thread storage to reduce bandwidth.  
	// To blur N pixels, we will need to load N + 2*BlurRadius pixels due to the blur radius.
	//
	
	// Sample extra 2 * BlurRadius pixels.
	int ExtraX = 0, CacheIdx = -1;
	if(groupThreadID.x < gBlurRadius)
	{
		// Clamp out of bound samples that occur at image borders.
		ExtraX = max(dispatchThreadID.x - gBlurRadius, 0);
		CacheIdx = groupThreadID.x;
	}
	else if(groupThreadID.x >= N-gBlurRadius)
	{
		// Clamp out of bound samples that occur at image borders.
		ExtraX = min(dispatchThreadID.x + gBlurRadius, InputTexture.Length.x - 1);
		CacheIdx = groupThreadID.x + 2 * gBlurRadius;
	}
	
	if(CacheIdx > 0)
	{
		gCache[CacheIdx] = InputTexture[int2(ExtraX, dispatchThreadID.y)];
	}
	
	// Clamp out of bound samples that occur at image borders.
	CacheIdx = groupThreadID.x + gBlurRadius;
	gCache[CacheIdx] = InputTexture[min(dispatchThreadID.xy, InputTexture.Length.xy - 1)];

	// Wait for all threads to finish.
	GroupMemoryBarrierWithGroupSync();
	
	//
	// Blur each pixel.
	//

	float2 BlurColor = float2(0, 0);
	
	for(int i = -gBlurRadius; i <= gBlurRadius; ++i)
	{
		int k = groupThreadID.x + gBlurRadius + i;
		
		BlurColor += Weights[i + gBlurRadius] * gCache[k];
	}
	
	OutputTexture[dispatchThreadID.xy] = BlurColor;
}

[numthreads(1, N, 1)]
void VertBlurCS(int3 groupThreadID : SV_GroupThreadID,
				int3 dispatchThreadID : SV_DispatchThreadID)
{
	// Put in an array for each indexing.
	float Weights[11] = { w0, w1, w2, w3, w4, w5, w6, w7, w8, w9, w10 };

	//
	// Fill local thread storage to reduce bandwidth.  
	// To blur N pixels, we will need to load N + 2*BlurRadius pixels due to the blur radius.
	//
	
	// Sample extra 2 * BlurRadius pixels.
	int ExtraY = 0, CacheIdx = -1;
	if(groupThreadID.y < gBlurRadius)
	{
		// Clamp out of bound samples that occur at image borders.
		ExtraY = max(dispatchThreadID.y - gBlurRadius, 0);
		CacheIdx = groupThreadID.y;
	}
	if(groupThreadID.y >= N-gBlurRadius)
	{
		// Clamp out of bound samples that occur at image borders.
		ExtraY = min(dispatchThreadID.y + gBlurRadius, InputTexture.Length.y - 1);
		CacheIdx = groupThreadID.y + 2 * gBlurRadius;
	}
	
	if(CacheIdx > 0)
	{
		gCache[CacheIdx] = InputTexture[int2(dispatchThreadID.x, ExtraY)];
	}
	
	
	// Clamp out of bound samples that occur at image borders.
	CacheIdx = groupThreadID.y + gBlurRadius;
	gCache[CacheIdx] =  InputTexture[min(dispatchThreadID.xy, InputTexture.Length.xy - 1)];


	// Wait for all threads to finish.
	GroupMemoryBarrierWithGroupSync();
	
	//
	// Blur each pixel.
	//

	float2 BlurColor = float2(0, 0);
	
	for(int i = -gBlurRadius; i <= gBlurRadius; ++i)
	{
		int k = groupThreadID.y + gBlurRadius + i;
		
		BlurColor += Weights[i + gBlurRadius]*gCache[k];
	}
	
	OutputTexture[dispatchThreadID.xy] = BlurColor;
}