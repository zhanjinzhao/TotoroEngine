#include "Sampler.hlsl"
#include "Utlis.hlsl"

TextureCube EnvironmentMap;

cbuffer cbPrefilterEnvPass
{
	float4x4 gView;
	float4x4 gProj;
	float roughness;
};

struct VertexIn
{
	float3 PosL    : POSITION;
	float3 NormalL : NORMAL;
	float2 TexC    : TEXCOORD;
	float3 TangentU : TANGENT;
};

struct VertexOut
{
	float4 PosH		: SV_POSITION;
	float3 PosL		: POSITION;
};


VertexOut VS(VertexIn vin)
{
	VertexOut Out;

	// Use local vertex position as cubemap lookup vector.
	Out.PosL = vin.PosL;
	
	// Remove translation from the view matrix
	float4x4 View = gView;
	View[3][0] = View[3][1] = View[3][2] = 0.0f;
	
	Out.PosH = mul(mul(float4(vin.PosL, 1.0f), View), gProj);
		
	return Out;
}


float4 PS(VertexOut pin) : SV_TARGET
{
	float3 N = normalize(pin.PosL); 
	float3 R = N;
    float3 V = R;
	
	const uint SAMPLE_COUNT = 1024u;
    float TotalWeight = 0.0;   
    float3 PrefilteredColor = float3(0.0f, 0.0f, 0.0f);
	
    for(uint i = 0u; i < SAMPLE_COUNT; ++i)
    {
        float2 Xi = Hammersley(i, SAMPLE_COUNT);
        float3 H  = ImportanceSampleGGX(Xi, N, roughness).xyz;
        float3 L  = normalize(2.0 * dot(V, H) * H - V);

        float NdotL = max(dot(N, L), 0.0);
        if(NdotL > 0.0)
        {	
            PrefilteredColor += EnvironmentMap.Sample(gsamAnisotropicWrap, L).rgb * NdotL;
            TotalWeight      += NdotL;
        }
    }
	
    PrefilteredColor = PrefilteredColor / TotalWeight;

	return float4(PrefilteredColor, 1.0f);
}