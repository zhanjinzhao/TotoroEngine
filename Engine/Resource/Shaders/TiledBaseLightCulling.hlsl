#include "Shadows.hlsl"


Texture2D DepthTexture;
RWTexture2D<float2> TiledDepthDebugTexture;

// groupshared
groupshared uint MinTileDepthInt;
groupshared uint MaxTileDepthInt;

groupshared	uint TileLightIndices[MAX_LIGHT_COUNT_IN_TILE];
groupshared	uint TileLightCount;

// Output
RWStructuredBuffer<TileLightInfo> TileLightInfoList;


// Just for debug
// only check if light postion in the tile
bool IntersectPosition(uint LightIndex, uint3 GroupID)
{
	LightParameters Light = Lights[LightIndex];
	float2 LightUV = WorldToUV(float4(Light.Position, 1.0f), gView, gProj);
	
	float2 TileMinUV = ScreenToUV(GroupID.xy * TILE_BLOCK_SIZE, gRenderTargetSize);
	float2 TileMaxUV = ScreenToUV((GroupID.xy + float2(1.0f, 1.0f)) * TILE_BLOCK_SIZE, gRenderTargetSize);
		
	if (LightUV.x > TileMinUV.x && LightUV.y > TileMinUV.y && LightUV.x < TileMaxUV.x && LightUV.y < TileMaxUV.y)
	{
		return true;
	}
	else
	{
		return false;
	}
}

struct TSphere
{
    float3 Center;   // Center point.
    float  Radius;   // Radius.
};

struct TCone
{
    float3 Tip;      // Tip point.
    float  Height;   // Height.
    float3 Dir;      // Direction.
    float  Radius;   // Bottom radius.
};

struct TPlane
{
    float3 N;   // Plane normal.
    float  D;   // Signed distance from origin to this plane.
};

// Compute a plane from 3 noncollinear points that form a triangle.
//     P1
//    /  \
//   /    \
//  P0-----P2
TPlane ComputePlane( float3 P0, float3 P1, float3 P2 )
{
    TPlane Plane;
 
    float3 Edge1 = P1 - P0;
    float3 Edge2 = P2 - P0;
 
    Plane.N = normalize(cross(Edge1, Edge2));
    Plane.D = - dot(Plane.N, P0);
 
    return Plane;
}

// If the point in the negative half-space of plane, return true.
bool PointInNegativePlane( float3 P, TPlane Plane)
{
    return dot(P, Plane.N) + Plane.D < 0;
}

// If the cone is fully inside the negative half-space of the plane, return true.
bool ConeInNegativePlane(TCone Cone, TPlane Plane)
{
    // Compute point Q(the farthest point on the end of the cone to the positive space of the plane).
    float3 M = cross(cross( Plane.N, Cone.Dir ), Cone.Dir );
    float3 Q = Cone.Tip + Cone.Dir * Cone.Height -  M * Cone.Radius;
 
    // The cone is in the negative halfspace of the plane if both tip point and point Q
    // are both inside the negative halfspace of the plane.
    return PointInNegativePlane(Cone.Tip, Plane) && PointInNegativePlane(Q, Plane);
}

// Frustum in view space
struct TFrustum
{
    TPlane Planes[4];   // Left, right, top, bottom frustum planes.
	
	float NearZ;
	
	float FarZ;
};

// If the sphere is fully or partially contained within the frustum, return true.
bool SphereInsideFrustum(TSphere Sphere, TFrustum Frustum)
{
	// Check z value
	if (Sphere.Center.z - Sphere.Radius > Frustum.FarZ || Sphere.Center.z + Sphere.Radius < Frustum.NearZ)
	{
		return false;
	}
 
    // Check frustum planes
	for (int i = 0; i < 4; i++)
	{
		if (dot(Sphere.Center, Frustum.Planes[i].N) + Frustum.Planes[i].D < -Sphere.Radius) // Sphere in the negative half-space of plane
		{
			return false;
		}
	}
	
	return true;
}

// If the cone is fully or partially contained within the frustum, return true.
bool ConeInsideFrustum(TCone Cone, TFrustum Frustum)
{
    TPlane NearPlane = { float3( 0, 0, 1 ), - Frustum.NearZ };
    TPlane FarPlane = { float3( 0, 0, -1 ), Frustum.FarZ };
 
    // First check the near and far clipping planes.
    if (ConeInNegativePlane(Cone, NearPlane) || ConeInNegativePlane(Cone, FarPlane) )
    {
        return false;
    }
 
    // Then check frustum planes
    for (int i = 0; i < 4; i++)
    {
        if (ConeInNegativePlane(Cone, Frustum.Planes[i]))
        {
            return false;
        }
    }
 
    return true;
}

bool Intersect(uint LightIndex, uint3 GroupId, float MinTileZ, float MaxTileZ)
{
	// View space eye position is always at the origin.
    const float3 EyePos = float3( 0, 0, 0 );
	
	// Compute the 4 corner points as the frustum vertices.
    float2 ScreenSpace[4];
    // Top left point
    ScreenSpace[0] = float2( GroupId.xy * TILE_BLOCK_SIZE);
    // Top right point
    ScreenSpace[1] = float2( float2( GroupId.x + 1, GroupId.y ) * TILE_BLOCK_SIZE);
    // Bottom left point
    ScreenSpace[2] = float2( float2( GroupId.x, GroupId.y + 1 ) * TILE_BLOCK_SIZE);
    // Bottom right point
    ScreenSpace[3] = float2( float2( GroupId.x + 1, GroupId.y + 1 ) * TILE_BLOCK_SIZE);
	
	// Convert the screen space points to view space on the far clipping plane
    float3 ViewSpace[4];  
    for ( int i = 0; i < 4; i++ )
    {
        ViewSpace[i] = ScreenToView(ScreenSpace[i].xy, gRenderTargetSize, 1.0f, gInvProj).xyz;
    }
	
	// Build the frustum planes from the view space points
    TFrustum Frustum;
    // Left plane
    Frustum.Planes[0] = ComputePlane(EyePos, ViewSpace[0], ViewSpace[2]);
    // Right plane
    Frustum.Planes[1] = ComputePlane(EyePos, ViewSpace[3], ViewSpace[1]);
    // Top plane
    Frustum.Planes[2] = ComputePlane(EyePos, ViewSpace[1], ViewSpace[0]);
    // Bottom plane
    Frustum.Planes[3] = ComputePlane(EyePos, ViewSpace[2], ViewSpace[3]);
	// NearZ and FarZ
	Frustum.NearZ = NDCToView(float4(0, 0, MinTileZ, 1.0f), gInvProj).z;
	Frustum.FarZ = NDCToView(float4(0, 0, MaxTileZ, 1.0f), gInvProj).z;
	
	LightParameters Light = Lights[LightIndex];
	
	if(Light.LightType == 3) // PointLight
	{
		TSphere LightSphere;
		LightSphere.Center = WorldToView(float4(Light.Position, 1.0f), gView).xyz;   
		LightSphere.Radius = Light.Range;
		
		return SphereInsideFrustum(LightSphere, Frustum);
	}
	if(Light.LightType == 4) // SpotLight
	{
		TCone LightCone;
		LightCone.Tip = WorldToView(float4(Light.Position, 1.0f), gView).xyz;  
		LightCone.Height = Light.Range;
		LightCone.Dir = WorldToView(float4(Light.Direction, 0.0f), gView).xyz;
		LightCone.Radius = Light.SpotRadius;
		
		return ConeInsideFrustum(LightCone, Frustum);
	}
	else
	{
		return true;
	}			
}


[numthreads(TILE_BLOCK_SIZE, TILE_BLOCK_SIZE, 1)]
void CS(uint3 GroupId       : SV_GroupID,
		uint  GroupIndex    : SV_GroupIndex,
		uint3 DispatchThreadID : SV_DispatchThreadID)
{
	uint TileCountX = ceil(gRenderTargetSize.x / TILE_BLOCK_SIZE);
	uint TileIndex = GroupId.y * TileCountX + GroupId.x;
	
	uint2 TexUV = DispatchThreadID.xy;
	
	//----------------------------Initialize tile data------------------------------------//
	if(GroupIndex == 0) // Only thread 0
	{
		MinTileDepthInt = 0xFFFFFFFF;
		MaxTileDepthInt = 0;			
		TileLightCount = 0;
	}
	
	// Wait for all threads to finish.
	GroupMemoryBarrierWithGroupSync();
	
	//-----------------------Calculate min/max depth of tile------------------------------//	
	float Depth = DepthTexture[TexUV].r;
	uint DepthInt = asuint(Depth);
	
	InterlockedMin(MinTileDepthInt, DepthInt);
	InterlockedMax(MaxTileDepthInt, DepthInt);
	
	GroupMemoryBarrierWithGroupSync();
	float MinTileDepth = asfloat(MinTileDepthInt);
	float MaxTileDepth = asfloat(MaxTileDepthInt);
	
	// Debug
	//TiledDepthDebugTexture[TexUV] =  float2(MinTileDepth, MaxTileDepth);
	
	//-----------------------------Light culling------------------------------------------//		
	const uint ThreadCount = TILE_BLOCK_SIZE * TILE_BLOCK_SIZE;
	for(uint LightIndex = GroupIndex; LightIndex < LightCount; LightIndex += ThreadCount) // All threads
	{		
		//if(IntersectPosition(LightIndex, GroupId))
		if(Intersect(LightIndex, GroupId, MinTileDepth, MaxTileDepth))
		{
			uint Offset;		
			InterlockedAdd(TileLightCount, 1, Offset);
			TileLightIndices[Offset] = LightIndex;
		}	
	}
	
	GroupMemoryBarrierWithGroupSync();
	
	//-------------------------Copy to TileLightInfoList----------------------------------//
	if(GroupIndex == 0)  // Only thread 0
	{
		TileLightInfoList[TileIndex].LightCount = TileLightCount;
	}
	
	for (uint i = GroupIndex; i < TileLightCount; i += ThreadCount) // All threads
    {
        TileLightInfoList[TileIndex].LightIndices[i] = TileLightIndices[i];
    }
	
	GroupMemoryBarrierWithGroupSync();
	
	// Debug
	float DebugColor = TileLightCount / 10.0f;
	TiledDepthDebugTexture[TexUV] =  float2(DebugColor, DebugColor);
}