#include "SDFShared.hlsl"

Texture2D DepthGbuffer;

struct VertexIn
{
    float3 PosL    : POSITION;
    float2 TexC    : TEXCOORD;
};

struct VertexOut
{
	float4 PosH  : SV_POSITION;
    float2 TexC    : TEXCOORD;
};


VertexOut VS(VertexIn vin)
{
    VertexOut vout = (VertexOut)0.0f;

	// Already in homogeneous clip space.
	vout.PosH = float4(vin.PosL, 1.0f);

    vout.TexC = vin.TexC;

    return vout;
}

void RayMarchDistanceFields(float3 WorldRayStart, float3 WorldRayEnd, float MaxRayTime, out float MinRayTime, out float TotalStepsTaken)
{
	MinRayTime = MaxRayTime;
	TotalStepsTaken = 0;
	
	for (uint i = 0; i < gObjectCount; i++)
	{
		int SDFIndex = ObjectSDFDescriptors[i].SDFIndex;
		
		float3 LocalRayStart = mul(float4(WorldRayStart, 1.0f), ObjectSDFDescriptors[i].ObjInvWorld).xyz;
		float3 LocalRayEnd = mul(float4(WorldRayEnd, 1.0f), ObjectSDFDescriptors[i].ObjInvWorld).xyz;
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
			
			float MinDistance = 1000000;
			int Step = 0;			
			
			[loop]
			for (; Step < MAX_SDF_STEP; Step++)
			{
				float3 SampleLocalPosition = LocalRayStart + SampleRayTime * LocalRayDir;
				float3 ClampedSamplePosition = clamp(SampleLocalPosition, -LocalExtent, LocalExtent);
				float3 VolumeUV = (ClampedSamplePosition * RcpExtent) * 0.5f + 0.5f;
				float SDFWidth = Extent * 2.0f;
				float DistanceField = SampleMeshDistanceField(SDFIndex, VolumeUV, SDFWidth);
				MinDistance = min(MinDistance, DistanceField);
				
				float MinStepSize = 1.0f / (4 * MAX_SDF_STEP);
				float StepDistance = max(DistanceField, MinStepSize);
				SampleRayTime += StepDistance;
				
				// Terminate the trace if we reached a negative area or went past the end of the ray
				if (DistanceField < 0 || SampleRayTime > IntersectionTimes.y * LocalRayLength)
				{
					break;
				}
			}
			
			if (MinDistance < 0 || Step == MAX_SDF_STEP)
			{
				MinRayTime = min(MinRayTime, SampleRayTime);
			}
				
			TotalStepsTaken += Step;
		}
	}	
}

float4 PS(VertexOut pin) : SV_Target
{
	float NDCDepth = DepthGbuffer.SampleLevel(gsamDepthMap, pin.TexC, 0.0f).r;	
	float3 OpaqueWorldPosition = UVToWorld(pin.TexC, NDCDepth, gInvProj, gInvView).xyz;
	
	float TraceDistance = 400;
	float3 WorldRayStart = gEyePosW;
	float3 WorldRayEnd = WorldRayStart + normalize(OpaqueWorldPosition - WorldRayStart) * TraceDistance;
	
	float MinRayTime;
	float TotalStepsTaken;
	RayMarchDistanceFields(WorldRayStart, WorldRayEnd, TraceDistance, MinRayTime, TotalStepsTaken);
	
	float Color = saturate(TotalStepsTaken / 100);
	
	if (MinRayTime < TraceDistance)
	{
		Color += 0.1f;
	}
	
	return float4(Color, Color, Color, 1.0f);
}


