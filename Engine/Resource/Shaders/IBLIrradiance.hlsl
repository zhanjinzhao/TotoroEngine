#include "Common.hlsl"

TextureCube EnvironmentMap;

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
	float3 Irradiance = float3(0.0f, 0.0f, 0.0f);

	// The sample direction equals the hemisphere's orientation
	float3 Normal = normalize(pin.PosL);
	float3 Up = float3(0.0, 1.0, 0.0);
	float3 Right = cross(Up, Normal);
	Up = cross(Normal, Right);

	float SampleDelta = 0.025f;
	float SampleCount = 0.0f;
	for (float phi = 0.0f; phi < 2.0f * PI; phi += SampleDelta)
	{
		for (float theta = 0.0; theta < 0.5 * PI; theta += SampleDelta)
		{
			// Spherical to cartesian (in tangent space)
			float3 TangentSample = float3(sin(theta) * cos(phi),  sin(theta) * sin(phi), cos(theta));
			// Tangent space to world
			float3 SampleVec = TangentSample.x * Right + TangentSample.y * Up + TangentSample.z * Normal;
			
			Irradiance += EnvironmentMap.Sample(gsamAnisotropicWrap, SampleVec).rgb * cos(theta) * sin(theta);
			SampleCount++;
		}
	}
	Irradiance = PI * Irradiance * (1.0f / SampleCount);

	return float4(Irradiance, 1.0f);
}