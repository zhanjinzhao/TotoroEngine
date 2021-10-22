#pragma once

#include <string>
#include "Shader/Shader.h"
#include "Texture/Texture.h"
#include "D3D12/D3D12Resource.h"
#include "D3D12/D3D12View.h"
#include "Math/Math.h"

struct TMaterialConstants
{
public:
	TVector4 DiffuseAlbedo;
	TVector3 FresnelR0;
	float Roughness;

	// Used in texture mapping.
	TMatrix MatTransform = TMatrix::Identity;

	TVector3 EmissiveColor;
	UINT ShadingModel;
};

// Defines a subrange of geometry in a TMeshProxy.  This is for when multiple
// geometries are stored in one vertex and index buffer.  It provides the offsets
// and data needed to draw a subset of geometry stores in the vertex and index 
// buffers
struct TSubmeshProxy
{
	UINT IndexCount = 0;
	UINT StartIndexLocation = 0;
	INT BaseVertexLocation = 0;

	// Bounding box of the geometry defined by this submesh. 
	DirectX::BoundingBox Bounds;
};

struct TMeshProxy
{
	// Give it a name so we can look it up by name.
	std::string Name;

	TD3D12VertexBufferRef VertexBufferRef;
	TD3D12IndexBufferRef IndexBufferRef;


	// Data about the buffers.
	UINT VertexByteStride = 0;
	UINT VertexBufferByteSize = 0;
	DXGI_FORMAT IndexFormat = DXGI_FORMAT_R16_UINT;
	UINT IndexBufferByteSize = 0;

	// A MeshGeometry may store multiple geometries in one vertex/index buffer.
	// Use this container to define the Submesh geometries so we can draw
	// the Submeshes individually.
	std::unordered_map<std::string, TSubmeshProxy> SubMeshs;
};

struct TLightShaderParameters
{
	TVector3 Color;       // All light
	float    Intensity;   // All light
	TVector3 Position;    // Point/Spot light only
	float    Range;       // Point/Spot light only
	TVector3 Direction;   // Directional/Spot light only
	float    SpotRadius;  // Spot light only
	TVector2 SpotAngles;  // Spot light only
	UINT     LightType;
	INT      ShadowMapIdx = 0;

	TMatrix LightProj = TMatrix::Identity;
	TMatrix  ShadowTransform = TMatrix::Identity;

	// AreaLight
	TVector3 AreaLightPoint0InWorld; 
	float LightPad2;
	TVector3 AreaLightPoint1InWorld;
	float LightPad3;
	TVector3 AreaLightPoint2InWorld;
	float LightPad4;
	TVector3 AreaLightPoint3InWorld;
	float LightPad5;
};

struct TLightCommonData
{
	UINT LightCount = 0;
};

#define MAX_LIGHT_COUNT_IN_TILE 500

struct TTileLightInfo
{
	UINT LightIndices[MAX_LIGHT_COUNT_IN_TILE];
	UINT LightCount;
};

struct PassConstants
{
	TMatrix View = TMatrix::Identity;
	TMatrix InvView = TMatrix::Identity;
	TMatrix Proj = TMatrix::Identity;
	TMatrix InvProj = TMatrix::Identity;
	TMatrix ViewProj = TMatrix::Identity;
	TMatrix InvViewProj = TMatrix::Identity;
	TMatrix PrevViewProj = TMatrix::Identity;
	TVector3 EyePosW = { 0.0f, 0.0f, 0.0f };
	float cbPassPad1 = 0.0f;
	TVector2 RenderTargetSize = { 0.0f, 0.0f };
	TVector2 InvRenderTargetSize = { 0.0f, 0.0f };
	float NearZ = 0.0f;
	float FarZ = 0.0f;
	float TotalTime = 0.0f;
	float DeltaTime = 0.0f;

	TVector4 FogColor = { 0.7f, 0.7f, 0.7f, 1.0f };
	float gFogStart = 5.0f;
	float gFogRange = 150.0f;
	TVector2 cbPassPad2;
};

struct ObjectConstants
{
	TMatrix World = TMatrix::Identity;
	TMatrix PrevWorld = TMatrix::Identity;
	TMatrix TexTransform = TMatrix::Identity;
};

struct SpritePassConstants
{
	TMatrix ScreenToNDC = TMatrix::Identity;
};

struct SpriteObjectConstants
{
	UINT	 TexRegisterOffset;
	UINT	 ObjPad0;
	UINT     ObjPad1;
	UINT     ObjPad2;
};

struct BlurSettingsConstants
{
	INT gBlurRadius;

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

struct TObjectSDFDescriptor
{
	TMatrix ObjWorld;
	TMatrix ObjInvWorld;
	TMatrix ObjInvWorld_IT;

	int SDFIndex;
	int pad1;
	int pad2;
	int pad3;
};

struct SDFConstants
{
	UINT ObjectCount;
	UINT Pad1;
	UINT Pad2;
	UINT Pad3;
};

struct SSAOPassConstants
{
	TMatrix ProjTex;

	// Coordinates given in view space.
	float    OcclusionRadius;
	float    OcclusionFadeStart;
	float    OcclusionFadeEnd;
	float    SurfaceEpsilon;
};

struct DeferredLightingPassConstants
{
	UINT EnableSSAO;
};

struct PrefilterEnvironmentConstant
{
	TMatrix View = TMatrix::Identity;
	TMatrix Proj = TMatrix::Identity;
	float Roughness;
};