#include "Shadows.hlsl"
#include "PBRLighting.hlsl"

Texture2D BaseColorGbuffer;
Texture2D NormalGbuffer;
Texture2D WorldPosGbuffer;
Texture2D OrmGbuffer;
Texture2D EmissiveGbuffer;
TextureCube IBLIrradianceMap;

#define IBL_PREFILTER_ENVMAP_MIP_LEVEL 5
TextureCube IBLPrefilterEnvMaps[IBL_PREFILTER_ENVMAP_MIP_LEVEL];

Texture2D BrdfLUT;

Texture2D SSAOBuffer;

// AreaLight 
const static float LTC_LUT_SIZE  = 64.0;
const static float LTC_LUT_SCALE = (LTC_LUT_SIZE - 1.0)/LTC_LUT_SIZE;
const static float LTC_LUT_BIAS  = 0.5 / LTC_LUT_SIZE;

Texture2D LTC_LUT1;
Texture2D LTC_LUT2;

StructuredBuffer<TileLightInfo> LightInfoList;

#define DIRECTIONAL_LIGHT_PIXEL_WIDTH       5.0f
#define SPOT_LIGHT_PIXEL_WIDTH              10.0f

cbuffer cbDeferredLighting
{
	uint EnableSSAO;
};

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

float3 GetPrefilteredColor(float Roughness, float3 ReflectDir)
{
	float Level = Roughness * (IBL_PREFILTER_ENVMAP_MIP_LEVEL - 1);
	int FloorLevel = floor(Level);
	int CeilLevel = ceil(Level);

	float3 FloorSample = IBLPrefilterEnvMaps[FloorLevel].SampleLevel(gsamLinearClamp, ReflectDir, 0).rgb;
	float3 CeilSample = IBLPrefilterEnvMaps[CeilLevel].SampleLevel(gsamLinearClamp, ReflectDir, 0).rgb;
	
	float3 PrefilteredColor = lerp(FloorSample, CeilSample, (Level - FloorLevel));
	return PrefilteredColor;
}

float4 PS(VertexOut pin) : SV_TARGET
{
	float3 FinalColor = 0.0f;

	// Get Gbuffer data
	float3 BaseColor = BaseColorGbuffer.Sample(gsamPointClamp, pin.TexC).rgb;	
	float3 Normal = NormalGbuffer.Sample(gsamPointClamp, pin.TexC).rgb;			
	float3 WorldPos = WorldPosGbuffer.Sample(gsamPointClamp, pin.TexC).rgb;
	float ShadingModelValue = WorldPosGbuffer.Sample(gsamPointClamp, pin.TexC).a;
	uint ShadingModel = (uint)round(ShadingModelValue * (float)0xF);	
	float Roughness = OrmGbuffer.Sample(gsamPointClamp, pin.TexC).g;
	float Metallic = OrmGbuffer.Sample(gsamPointClamp, pin.TexC).b;	
	float3 EmissiveColor = EmissiveGbuffer.Sample(gsamPointClamp, pin.TexC).rgb;
	
	// AO
	float AmbientAccess = 1.0f;
	if (EnableSSAO == 1)
	{
		AmbientAccess = SSAOBuffer.Sample(gsamPointClamp, pin.TexC).r;
	}
	
	if(ShadingModel == 0) // DefaultLit
	{		
		float3 CameraPosition = gEyePosW;
		float3 ViewDir = normalize(CameraPosition - WorldPos);
		Normal = normalize(Normal);
		
		// Emissive light
		FinalColor += EmissiveColor;
		
#if USE_TBDR		
		// Get the tile index of current pixel
		float2 ScreenPos = UVToScreen(pin.TexC, gRenderTargetSize);
		uint TileX = floor(ScreenPos.x / TILE_BLOCK_SIZE);
		uint TileY = floor(ScreenPos.y / TILE_BLOCK_SIZE);
		uint TileCountX = ceil(gRenderTargetSize.x / TILE_BLOCK_SIZE);
		uint TileIndex = TileY * TileCountX + TileX;
		
		TileLightInfo LightInfo = LightInfoList[TileIndex];
		
		// Calculate direct lighting
		[unroll(10)]
		for (uint i = 0; i < LightInfo.LightCount; i++)
		{
			uint LightIndex = LightInfo.LightIndices[i];
			LightParameters Light = Lights[LightIndex];
#else
		[unroll(10)]
		for (uint i = 0; i < LightCount; i++)
		{
			LightParameters Light = Lights[i];			
#endif //USE_TBDR
			
			// IBL ambient light
			if (Light.LightType == 1) 
			{
				// Irradiance
				float3 Irradiance = IBLIrradianceMap.SampleLevel(gsamLinearClamp, Normal, 0).rgb;
			
				// PrefilteredColor
				float3 ReflectDir = reflect(-ViewDir, Normal);
				float3 PrefilteredColor  = GetPrefilteredColor(Roughness, ReflectDir);
			
				// LUT value
				float NoV = dot(Normal, ViewDir);
				float2 LUT = BrdfLUT.SampleLevel(gsamLinearClamp, float2(NoV, Roughness), 0).rg;
			
				FinalColor += AmbientLighting(Metallic, BaseColor, Irradiance, PrefilteredColor, LUT, AmbientAccess);
			}
			
			// Directional light
			else if (Light.LightType == 2) 
			{
				// Get shadow factor        
				float4 ShadowPosH = mul(float4(WorldPos, 1.0f), Light.ShadowTransform);
				float3 LightDir = normalize(-Light.Direction);
				
				// Note: Don't use the positon of DirectionalLightActor,
				// because we should keep the trace direction of SDF shadow always equal to the inverse light direction
				float3 LightPosition = WorldPos + LightDir * 100.0f;
				
				float TanLightAngle = tan(3 * PI / 180.0f);  //TODO
				float ShadowFactor = CalcVisibility(ShadowPosH, Light.ShadowMapIdx, Light.LightProj, false, DIRECTIONAL_LIGHT_PIXEL_WIDTH, 
					WorldPos, LightPosition, TanLightAngle); 						
				
				float3 Radiance = Light.Intensity * Light.Color;

				FinalColor += DirectLighting(Radiance, LightDir, Normal, ViewDir, Roughness, Metallic, BaseColor, ShadowFactor);			
			}
			
			// PointLight
			else if(Light.LightType == 3)
			{		
				float3 LightToPoint = WorldPos - Light.Position;
				float ShadowFactor = CalcVisibilityOmni(LightToPoint, Light.ShadowMapIdx, Light.Range);
							
				float3 LightDir = normalize(Light.Position - WorldPos);
				float Attenuation = CalcDistanceAttenuation(WorldPos, Light.Position, Light.Range);
				float3 Radiance = Light.Intensity * Attenuation * Light.Color;
				
				FinalColor += DirectLighting(Radiance, LightDir, Normal, ViewDir, Roughness, Metallic, BaseColor, ShadowFactor);
			}	
			
			// SpotLight
			else if (Light.LightType == 4) 
			{	
			    float4 ShadowPosH = mul(float4(WorldPos, 1.0f), Light.ShadowTransform);
				
				float MaxAngle = tan(10 * PI / 180.0f);
				float LightSourceRadius = 0.2f; // TODO
				float LightDistance = length(Light.Position - WorldPos);
				float TanLightAngle = min(LightSourceRadius / LightDistance, MaxAngle);
				
				float ShadowFactor = CalcVisibility(ShadowPosH, Light.ShadowMapIdx, Light.LightProj, true, SPOT_LIGHT_PIXEL_WIDTH, WorldPos, 
					Light.Position, TanLightAngle);
				
				float3 LightDir = normalize(Light.Position - WorldPos);
				float Attenuation = CalcDistanceAttenuation(WorldPos, Light.Position, Light.Range);
				Attenuation *= SpotAttenuation(LightDir, Light.Direction, Light.SpotAngles);
				float3 Radiance = Light.Intensity * Attenuation * Light.Color;

				FinalColor += DirectLighting(Radiance, LightDir, Normal, ViewDir, Roughness, Metallic, BaseColor, ShadowFactor);
			}
			
			// AreaLight
			else if(Light.LightType == 5)
			{
				float ShadowFactor = 1.f;
				
				float NoV = saturate(dot(Normal, ViewDir));	
				float2 UV = float2(Roughness, sqrt(1.0f - NoV)); 
				UV = UV * LTC_LUT_SCALE + LTC_LUT_BIAS;
				
				float4 t1 = LTC_LUT1.SampleLevel(gsamLinearClamp, UV, 0);
				float4 t2 = LTC_LUT2.SampleLevel(gsamLinearClamp, UV, 0);
								
				float3 Points[4];
				Points[0] = Light.AreaLightPoint0InWorld;
				Points[1] = Light.AreaLightPoint1InWorld;
				Points[2] = Light.AreaLightPoint2InWorld;
				Points[3] = Light.AreaLightPoint3InWorld;
				
				float3 Radiance = Light.Intensity * Light.Color;
				FinalColor += AreaLighting(Radiance, Normal, ViewDir, WorldPos, Roughness, Metallic, BaseColor, t1, t2, Points);
				
			}
		}
	}
	else if(ShadingModel == 1) //Unlit
	{
		FinalColor = BaseColor;
	}	

    return float4(FinalColor, 1.0f);
}