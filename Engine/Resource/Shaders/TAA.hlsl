#include "Common.hlsl"

Texture2D ColorTexture;
Texture2D PrevColorTexture;
Texture2D VelocityGBuffer;
Texture2D DepthGbuffer;


#define USE_YCOCG 1
#define AA_DYNAMIC 1

static const float VarianceClipGamma = 1.0f;
static const float Exposure = 10;
static const float BlendWeightLowerBound = 0.03f;
static const float BlendWeightUpperBound = 0.12f;
static const float BlendWeightVelocityScale = 100.0f * 60.0f;

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

float3 RGB2YCoCgR(float3 rgbColor)
{
	float3 YCoCgRColor;

	YCoCgRColor.y = rgbColor.r - rgbColor.b;
	float temp = rgbColor.b + YCoCgRColor.y / 2;
	YCoCgRColor.z = rgbColor.g - temp;
	YCoCgRColor.x = temp + YCoCgRColor.z / 2;

	return YCoCgRColor;
}

float3 YCoCgR2RGB(float3 YCoCgRColor)
{
	float3 rgbColor;

	float temp = YCoCgRColor.x - YCoCgRColor.z / 2;
	rgbColor.g = YCoCgRColor.z + temp;
	rgbColor.b = temp - YCoCgRColor.y / 2;
	rgbColor.r = rgbColor.b + YCoCgRColor.y;

	return rgbColor;
}

// Clips towards aabb center
// Ref: "Temporal Reprojection Anti-Aliasing in INSIDE"
float3 ClipAABB(float3 AabbMin, float3 AabbMax, float3 Point)
{
	float3 Center = 0.5 * (AabbMax + AabbMin);
	float3 Extend = 0.5 * (AabbMax - AabbMin);

	float3 Dist = Point - Center;
	float3 DistUnit = Dist.xyz / Extend;
	DistUnit = abs(DistUnit);
	float MaxDistUnit = max(DistUnit.x, max(DistUnit.y, DistUnit.z));

	if (MaxDistUnit > 1.0)
		return Center + Dist / MaxDistUnit;
	else
		return Point; // point inside aabb
}

float4 PS(VertexOut pin) : SV_TARGET
{	
	float4 Output;
	
	//Compute UVoffset to last frame
	float2 UVoffset;
	float Depth = DepthGbuffer.SampleLevel(gsamDepthMap, pin.TexC, 0.0f).r;
	
	float4 PosN = UVToNDC(pin.TexC, Depth); // Pos in NDC space	
	float4 PrevPosN = mul(PosN, gInvViewProj);
	PrevPosN = mul( PrevPosN, gPrevViewProj);
	PrevPosN /= PrevPosN.w;
	
	UVoffset = pin.TexC - NDCToUV(PrevPosN);
	
#if AA_DYNAMIC
	float2 VelocityUV = pin.TexC; //TODO, adjust jitter???
	float2 Velocity = VelocityGBuffer.SampleLevel(gsamPointClamp, VelocityUV, 0).rg;
	
	if(abs(Velocity.x) > 0.0f || abs(Velocity.y) > 0.0f)
	{
		UVoffset = Velocity;
	}	
#endif
	
	// Sample pixel color of current frame and last frame.
	float3 CurColor = ColorTexture.SampleLevel(gsamPointClamp, pin.TexC, 0).rgb;	
	float3 PrevColor = PrevColorTexture.SampleLevel(gsamPointClamp, pin.TexC - UVoffset, 0).rgb;
	
#if USE_YCOCG
	CurColor = RGB2YCoCgR(CurColor);
	PrevColor = RGB2YCoCgR(PrevColor);
#endif
	
	// Sample neighborhoods pixel color of current frame.
	uint SampleCount = 9;
	float3 Moment1 = 0.0f;
	float3 Moment2 = 0.0f;

	int x, y, i;
	for (y = -1; y <= 1; ++y)
	{
		for (x = -1; x <= 1; ++x)
		{
			i = (y + 1) * 3 + x + 1;
			float2 SampleOffset = float2(x, y) * gInvRenderTargetSize;
			float2 SampleUV = pin.TexC + SampleOffset;
			SampleUV = saturate(SampleUV);

			float3 NeighborhoodColor = ColorTexture.Sample(gsamPointClamp, SampleUV).rgb;
			NeighborhoodColor = max(NeighborhoodColor, 0.0f);
#if USE_YCOCG
			NeighborhoodColor = RGB2YCoCgR(NeighborhoodColor);
#endif		
			Moment1 += NeighborhoodColor;
			Moment2 += NeighborhoodColor * NeighborhoodColor;
		}
	}
	
	// Variance clipping.
	// Ref: "An Excursion in Temporal Supersampling"
	float3 Mean = Moment1 / SampleCount;
	float3 Sigma = sqrt(Moment2 / SampleCount - Mean * Mean); // Standard deviation
	float3 Min = Mean - VarianceClipGamma * Sigma;
	float3 Max = Mean + VarianceClipGamma * Sigma;
	
	PrevColor = ClipAABB(Min, Max, PrevColor);	
	
	// Blend
	const float Alpha = 0.1f;
	float3 Color = CurColor * Alpha + PrevColor * (1.0f - Alpha);	
	
#if USE_YCOCG
	Color = YCoCgR2RGB(Color);
#endif
	
	Output = float4(Color, 1.0f);
	
	return Output;
}