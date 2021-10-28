#include "Common.hlsl"
#include "PBRLighting.hlsl"

Texture2D BaseColorGbuffer;
Texture2D NormalGbuffer;
Texture2D OrmGbuffer;
Texture2D DepthGbuffer;
Texture2D ColorTexture;
Texture2D BlueNoiseTexture;
Texture2D BackDepthTexture;

static const int gTraceStepCount = 100;
static const float gRayLength = 10.0f;
static const float gScreenEdgeFadeLength = 0.2f;

#define USE_SCREEN_EDGE_FADE  1
#define MIRROR_ONLY_SSR   0

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

bool Intersect(float TraceDepth, float2 UV)
{
	float FrontDepth = DepthGbuffer.SampleLevel(gsamPointClamp, UV, 0).r;
	
	float BackDepth = BackDepthTexture.SampleLevel(gsamPointClamp, UV, 0).r;
	
	return TraceDepth > FrontDepth && TraceDepth < BackDepth;
}


bool TraceRay(float3 RayStart, float3 RayDir, out float2 OutUV)
{
	[loop]
	for(int i = 0; i < gTraceStepCount; i++)
	{
		float3 TracePos = RayStart + float(i) / gTraceStepCount * gRayLength * RayDir;	
		float TraceDepth = ViewDepthToNDCDepth(TracePos.z, gProj);
		//float TraceDepth = TracePos.z;
		
		float2 UV = ViewToUV(float4(TracePos, 1.0f), gProj);				
		if(UV.x < 0.0f || UV.x > 1.0f || UV.y < 0.0f || UV.y > 1.0f)
		{
			return false;
		}
	
		if (Intersect(TraceDepth, UV))
		{
			OutUV = UV;
			
			return true;
		}	
	}
	
	return false;
}

float CalculateAlpha(float2 UV)
{	
#if USE_SCREEN_EDGE_FADE	
	// Alpha
	// 1.0             -------------
	//                /|           |\
	//               / |           | \
	//              /  |           |  \
	//             /   |           |   \ 
	//            /    |           |    \
	//           /     |           |     \
	//          /      |           |      \
	// 0.0 ----|-------|-----------|-------|------> UV
	//       0.0<--t-->             <--t-->1.0       
	//
	// t = gScreenEdgeFadeLength
		
	float BorderOffsetX = min(UV.x, 1.0f - UV.x);
	float BorderOffsetY = min(UV.y, 1.0f - UV.y);
	float MinBorderOffset = min(BorderOffsetX, BorderOffsetY);
	
	return saturate(MinBorderOffset / gScreenEdgeFadeLength);	
#else	
	return 1.0f
#endif
}

#define BLUE_NOISE_SIZE 1024
#define BRDF_BIAS 0.7f

float4 PS(VertexOut pin) : SV_TARGET
{	
	float4 Output;
	
	// Get Gbuffer data
	float3 BaseColor = BaseColorGbuffer.SampleLevel(gsamPointClamp, pin.TexC, 0).rgb;
	float3 NormalW = NormalGbuffer.SampleLevel(gsamPointClamp, pin.TexC, 0).rgb;
	float Roughness = OrmGbuffer.SampleLevel(gsamPointClamp, pin.TexC, 0).g;
	float Metallic = OrmGbuffer.SampleLevel(gsamPointClamp, pin.TexC, 0).b;
	float Depth = DepthGbuffer.SampleLevel(gsamPointClamp, pin.TexC, 0).r;	
	
	// Get viewspace normal
	float3 NormalV = mul(NormalW, (float3x3)gView);
	NormalV = normalize(NormalV);
	
	// Get viewspace shading point postion
	float3 PosV = UVToView(pin.TexC, Depth, gInvProj).xyz;
	
#if MIRROR_ONLY_SSR				
	float4 TraceColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
		
	// Get viewspace reflect direction
	float3 ReflectDir = reflect(normalize(PosV), NormalV);
	
	float2 OutUV;
	if (TraceRay(PosV, ReflectDir, OutUV))
	{
		TraceColor = ColorTexture.SampleLevel(gsamPointClamp, OutUV, 0);
		TraceColor *= CalculateAlpha(OutUV);
	}

	float4 SceneColor = ColorTexture.SampleLevel(gsamPointClamp, pin.TexC, 0);
	Output = SceneColor + TraceColor;
	
#else
	const uint SampleCount = 10;
	float WeightSum = 0.0f;
	float4 TraceColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
	
	for (uint i = 0; i < SampleCount; i++)
	{
		//half2 Hash = BlueNoiseTexture.SampleLevel(gsamLinearWrap, (pin.TexC + gHaltonUniform2D.xy) * (gRenderTargetSize.xy * 0.5f) / float2(BLUE_NOISE_SIZE, BLUE_NOISE_SIZE), 0.0f).rg;
		half2 Hash = BlueNoiseTexture.SampleLevel(gsamLinearWrap, (pin.TexC + float2(float(i) / SampleCount, float(i) / SampleCount)) * gRenderTargetSize.xy / float2(BLUE_NOISE_SIZE, BLUE_NOISE_SIZE), 0.0f).rg;
		Hash.y = lerp(Hash.y, 0.0, BRDF_BIAS);
		
		float4 H = ImportanceSampleGGX(Hash, NormalV, Roughness);
		float3 SampleNormal = H.xyz;
		float HitPDF = H.w;
			
		float2 OutUV;
		float3 ReflectDir = reflect(normalize(PosV), SampleNormal);
		
		if (TraceRay(PosV, ReflectDir, OutUV))
		{
			float4 SampleColor = ColorTexture.SampleLevel(gsamPointClamp, OutUV, 0);
			SampleColor *= CalculateAlpha(OutUV);
				
			float NoL = saturate(dot(NormalV, ReflectDir));
			float Weight = DefaultBRDF(ReflectDir, NormalV, normalize(-PosV), Roughness, Metallic, BaseColor).r * NoL/ max(1e-5, HitPDF); //TODO
			
			TraceColor += SampleColor * Weight;
			WeightSum += Weight;
		}
	}
	
	TraceColor /= WeightSum;
	TraceColor = saturate(TraceColor);
	
	float4 SceneColor = ColorTexture.SampleLevel(gsamPointClamp, pin.TexC, 0);
	Output = SceneColor + TraceColor;
	
#endif
	
	return Output;	
}