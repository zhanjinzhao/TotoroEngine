#include "LightingUtil.hlsl"

static const float F0_DIELECTRIC = 0.04f;


// Lambert diffuse
float3 LambertDiffuse(float3 BaseColor)
{   
    return BaseColor * (1 / PI); 
}

// GGX (Trowbridge-Reitz)
float GGX(float Roughness, float3 N, float3 H)
{
    float a = Roughness * Roughness;
    float a2 = a * a;
    float NoH = max(dot(N, H), 0.0f);
    float NoH2 = NoH * NoH;
    float d = NoH2 * (a2 - 1.0f) + 1.0f;

    return a2 / (PI * d * d);
}

// Fresnel, Schlick approx.
float3 FresnelSchlick(float3 F0, float3 V, float3 H)
{
    float VoH = max(dot(V, H), 0.0f);
    return F0 + (1 - F0) * exp2((-5.55473 * VoH - 6.98316) * VoH);
}

// Schlick-GGX
float GeometrySchlickGGX(float Roughness, float3 N, float3 V)
{
    float k = pow(Roughness + 1, 2) / 8.0f;
    float NoV = max(dot(N, V), 0.0f);

    return NoV / (NoV * (1 - k) + k);
}

// Cook-Torrance Specular
float3 CookTorrance(float3 N, float3 L, float3 V, float Roughness, float3 F0, out float3 OutF)
{
    float3 H = normalize(V + L);

    float  D = GGX(Roughness, N, H);
    float3 F = FresnelSchlick(F0, V, H);
    float  G = GeometrySchlickGGX(Roughness, N, V) * GeometrySchlickGGX(Roughness, N, L);
    
    OutF = F;
    
    float NoL = max(dot(N, L), 0.0f);
    float NoV = max(dot(N, V), 0.0f);
    
    return (D * F * G) / (4 * max(NoL * NoV, 0.01f)); // 0.01 is added to prevent division by 0
}

float3 DirectBRDF(float3 LightDir, float3 Normal, float3 ViewDir, float Roughness, float Metallic, float3 BaseColor)
{
    float3 F0 = lerp(F0_DIELECTRIC.rrr, BaseColor.rgb, Metallic);
    
    float3 F = float3(0.f, 0.f, 0.f);
    float3 SpecularBRDF = CookTorrance(Normal, LightDir, ViewDir, Roughness, F0, F);
    float3 DiffuseBRDF = LambertDiffuse(BaseColor);
    
    float3 Kd = float3(1.0f, 1.0f, 1.0f) - F;
    Kd *= 1.0 - Metallic; // Metallic surfaces have no diffuse reflections
    float NoL = max(dot(Normal, LightDir), 0.0);

    return (Kd * DiffuseBRDF + SpecularBRDF) * NoL;  
}

float3 DirectLighting(float3 Radiance, float3 LightDir, float3 Normal, float3 ViewDir, float Roughness, float Metallic, float3 BaseColor, float ShadowFactor)
{      
    float3 BRDF = DirectBRDF(LightDir, Normal, ViewDir, Roughness, Metallic, BaseColor);
    
    return Radiance * BRDF * ShadowFactor;
}

float3 AreaLighting(float3 Radiance, float3 Normal, float3 ViewDir, float3 WorldPos, float Roughness, float Metallic, float3 BaseColor, float4 t1, float4 t2, float3 Points[4])
{  
    float3x3 Minv = float3x3(
		float3(t1.x,   0,   t1.z),
		float3(0,      1,   0),
		float3(t1.y,   0,   t1.w)
	);
				
	float3x3 Identity = float3x3(
		float3(  1, 0, 0),
		float3(  0, 1, 0),
		float3(  0, 0, 1)
	);
      
    float3 F0 = lerp(F0_DIELECTRIC.rrr, BaseColor.rgb, Metallic);
    float3 H = Normal; // Don't know light direction, so use normal to replace half vector
    float3 F = FresnelSchlick(F0, ViewDir, H);
    
    float3 SpecularBRDFCos = LTC_Evaluate(Normal, ViewDir, WorldPos, Minv, Points, false);
    // BRDF shadowing and Fresnel
    SpecularBRDFCos *= (F0 * t2.x + (1.0f - F0) * t2.y);
    float3 DiffuseBRDFCos = LTC_Evaluate(Normal, ViewDir, WorldPos, Identity, Points, false);
    
    float3 Kd = float3(1.0f, 1.0f, 1.0f) - F;
    Kd *= 1.0 - Metallic; // Metallic surfaces have no diffuse reflections

    return (Kd * DiffuseBRDFCos + SpecularBRDFCos) * Radiance;
}

float3 FresnelSchlickRoughness(float3 F0, float3 V, float3 N, float Roughness)
{
    float NdotV = max(dot(V, N), 0.0f);
    float r1 = 1.0f - Roughness;
    float3 Lambda = max(float3(r1, r1, r1), F0);
    return F0 + (Lambda - F0) * pow(1 - NdotV, 5.0f);
}

float3 AmbientLighting(float3 Normal, float3 ViewDir, float Roughness, float Metallic,
    float3 BaseColor, float3 Irradiance, float3 PrefilteredColor, float2 EnvBRDF, float ShadowFactor, float AmbientAccess)
{
    // IBL diffuse
    float3 F0 = lerp(F0_DIELECTRIC.rrr, BaseColor.rgb, Metallic); 
    float3 F = FresnelSchlickRoughness(F0, ViewDir, Normal, Roughness);
    float3 Diffuse = BaseColor * (1.0f - Metallic) * (float3(1.0f, 1.0f, 1.0f) - F) * Irradiance;
    
    // IBL specular
    float3 Specular = PrefilteredColor * (F * EnvBRDF.x + EnvBRDF.y);

    float3 Ambient = ( Diffuse + Specular) * AmbientAccess;
    return Ambient;
}

