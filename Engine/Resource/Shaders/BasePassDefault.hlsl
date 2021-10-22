#include "Common.hlsl"

Texture2D BaseColorTexture;
Texture2D NormalTexture;
Texture2D MetallicTexture;
Texture2D RoughnessTexture;

struct VertexIn
{
    float3 PosL    : POSITION;
    float3 NormalL : NORMAL;
    float2 TexC    : TEXCOORD;
    float3 TangentU : TANGENT;
};

struct VertexOut
{
    float4 PosH     : SV_POSITION;
    float4 CurPosH  : POSITION0;
	float4 PrevPosH : POSITION1;
    float3 PosW     : POSITION2;
    float3 NormalW  : NORMAL;
    float2 TexC     : TEXCOORD;
    float3 TangentW : TANGENT;
};

struct PixelOutput
{
    float4 BaseColor    : SV_TARGET0;
    float4 Normal		: SV_TARGET1;
    float4 WorldPos		: SV_TARGET2;
    float4 OcclusionRoughnessMetallic	: SV_TARGET3;
    float2 Velocity		: SV_TARGET4;
    float4 Emissive     : SV_TARGET5;
};

VertexOut VS(VertexIn vin)
{
    VertexOut Out = (VertexOut)0.0f;

    // Fetch the material data.
	MaterialData MatData = cbMaterialData;

    // Transform to world space.
    float4 PosW = mul(float4(vin.PosL, 1.0f), gWorld);
    Out.PosW = PosW.xyz;

    // Assumes nonuniform scaling; otherwise, need to use inverse-transpose of world matrix.
    Out.NormalW = mul(vin.NormalL, (float3x3)gWorld);

    Out.TangentW = mul(vin.TangentU, (float3x3)gWorld);

    // Transform to homogeneous clip space.
    Out.PosH = mul(PosW, gViewProj);
    
    // CurPosH and PrevPosH
    Out.CurPosH = mul(PosW, gViewProj);
    
    float4 PrevPosW = mul(float4(vin.PosL, 1.0f), gPrevWorld);
    Out.PrevPosH = mul(PrevPosW, gPrevViewProj);

    // Output vertex attributes for interpolation across triangle.
    float4 TexC = mul(float4(vin.TexC, 0.0f, 1.0f), gTexTransform);
    Out.TexC = mul(TexC, MatData.MatTransform).xy;

    return Out;
}

PixelOutput PS(VertexOut pin) //: SV_Target
{
    PixelOutput Out;

    MaterialData MatData = cbMaterialData;

    uint Width = 0, Height = 0;
    
    // BaseColor
    BaseColorTexture.GetDimensions(Width, Height);
    if(Width == 1)
	{
        Out.BaseColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
	}
	else
	{
        Out.BaseColor = BaseColorTexture.Sample(gsamAnisotropicWrap, pin.TexC);
	}

    // Normal
	NormalTexture.GetDimensions(Width, Height);
	if (Width == 1)
	{
		Out.Normal = float4(normalize(pin.NormalW), 1.0f);
	}
	else
	{
		float4 NormalMapSample = NormalTexture.Sample(gsamAnisotropicWrap, pin.TexC);
		float3 Normal = NormalSampleToWorldSpace(NormalMapSample.rgb, pin.NormalW, pin.TangentW);
		Out.Normal = float4(normalize(Normal), 1.0f);
	}

    // WorldPos
    Out.WorldPos = float4(pin.PosW, 0.0f);
	
	// ShadingModel
	Out.WorldPos.a = (float)MatData.ShadingModel / (float)0xF;

    // Metallic
    float Metallic = 0.0f;
 	MetallicTexture.GetDimensions(Width, Height);  
	if (Width > 1)
	{
		Metallic = MetallicTexture.Sample(gsamAnisotropicWrap, pin.TexC).r;
	}   
    
    // Roughness
    float Roughtness = 0.64f;
	RoughnessTexture.GetDimensions(Width, Height);
	if (Width > 1)
    {
		Roughtness = RoughnessTexture.Sample(gsamAnisotropicWrap, pin.TexC).r;
	}
    
    Out.OcclusionRoughnessMetallic = float4(0, Roughtness, Metallic, 0);
    
    // Velocity
    float4 CurPos = pin.CurPosH;
    CurPos /= CurPos.w; // Complete projection division
    CurPos.xy = NDCToUV(CurPos);
    
    float4 PrevPos = pin.PrevPosH;
    PrevPos /= PrevPos.w; // Complete projection division
    PrevPos.xy = NDCToUV(PrevPos);
    
    Out.Velocity = CurPos.xy - PrevPos.xy;
    
    // Emissive
    Out.Emissive = float4(MatData.EmissiveColor, 1.0f);
    
    return Out;
}


