#include "Utlis.hlsl"

Texture2D<float> ShadowMap;
RWTexture2D<float2> VSM;

#define N 16

[numthreads(N, N, 1)]
void CS(int3 dispatchThreadID : SV_DispatchThreadID)
{
	float Depth = ShadowMap[dispatchThreadID.xy];	
	VSM[dispatchThreadID.xy] = float2(Depth, Depth * Depth);
}
