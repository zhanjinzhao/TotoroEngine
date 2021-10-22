#include "Common.hlsl"

#define MAX_SDF_STEP 256
#define MAX_SDF_TEXTURE_COUNT 20
#define EIGHT_BIT_MESH_DISTANCE_FIELDS 1

struct MeshSDFDescriptor
{
	float3 Center;
	float Extent;
	
	int Resolution;
	int pad1;
	int pad2;
	int pad3;
};

struct ObjectSDFDescriptor
{
	float4x4 ObjWorld;
	float4x4 ObjInvWorld;
	float4x4 ObjInvWorld_IT;
	
	int SDFIndex;
	int pad1;
	int pad2;
	int pad3;
};

cbuffer cbSDF
{	
	uint gObjectCount;
	uint Pad1;
	uint Pad2;
	uint Pad3;
};

Texture3D SDFTextures[MAX_SDF_TEXTURE_COUNT];

StructuredBuffer<MeshSDFDescriptor> MeshSDFDescriptors; 
StructuredBuffer<ObjectSDFDescriptor> ObjectSDFDescriptors; 

float SampleMeshDistanceField(float SDFIndex, float3 VolumeUV, float SDFWidth)
{
	float DistanceField = SDFTextures[SDFIndex].SampleLevel(gsamLinearClamp, VolumeUV, 0).x;				
#if EIGHT_BIT_MESH_DISTANCE_FIELDS
	DistanceField = (DistanceField - 0.5f) * 2.0f * SDFWidth;
#endif
	
	return DistanceField;
}

