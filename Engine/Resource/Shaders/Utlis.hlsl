#ifndef __SHADER_UTILS__
#define __SHADER_UTILS__

const static float PI = 3.14159265359;

float Square( float x )
{
	return x*x;
}

float Pow4( float x )
{
	float xx = x*x;
	return xx * xx;
}

// Transforms a normal map sample to world space.
float3 NormalSampleToWorldSpace(float3 NormalMapSample, float3 UnitNormalW, float3 TangentW)
{
	// Uncompress each component from [0,1] to [-1,1].
	float3 NormalT = 2.0f* NormalMapSample - 1.0f;

	// Build orthonormal basis.
	float3 N = UnitNormalW;
	float3 T = normalize(TangentW - dot(TangentW, N)*N);
	float3 B = cross(N, T);

	float3x3 TBN = float3x3(T, B, N);

	// Transform from tangent space to world space.
	float3 BumpedNormalW = mul(NormalT, TBN);

	return BumpedNormalW;
}

/*
* Clips a ray to an AABB.  Does not handle rays parallel to any of the planes.
*
* @param RayOrigin - The origin of the ray in world space.
* @param RayEnd - The end of the ray in world space.
* @param BoxMin - The minimum extrema of the box.
* @param BoxMax - The maximum extrema of the box.
* @return - Returns the closest intersection along the ray in x, and furthest in y.
*			If the ray did not intersect the box, then the furthest intersection <= the closest intersection.
*			The intersections will always be in the range [0,1], which corresponds to [RayOrigin, RayEnd] in worldspace.
*			To find the world space position of either intersection, simply plug it back into the ray equation:
*			WorldPos = RayOrigin + (RayEnd - RayOrigin) * Intersection;
*/
float2 LineBoxIntersect(float3 RayOrigin, float3 RayEnd, float3 BoxMin, float3 BoxMax)
{
	float3 InvRayDir = 1.0f / (RayEnd - RayOrigin);

	//find the ray intersection with each of the 3 planes defined by the minimum extrema.
	float3 FirstPlaneIntersections = (BoxMin - RayOrigin) * InvRayDir;
	//find the ray intersection with each of the 3 planes defined by the maximum extrema.
	float3 SecondPlaneIntersections = (BoxMax - RayOrigin) * InvRayDir;
	//get the closest of these intersections along the ray
	float3 ClosestPlaneIntersections = min(FirstPlaneIntersections, SecondPlaneIntersections);
	//get the furthest of these intersections along the ray
	float3 FurthestPlaneIntersections = max(FirstPlaneIntersections, SecondPlaneIntersections);

	float2 BoxIntersections;
	//find the furthest near intersection
	BoxIntersections.x = max(ClosestPlaneIntersections.x, max(ClosestPlaneIntersections.y, ClosestPlaneIntersections.z));
	//find the closest far intersection
	BoxIntersections.y = min(FurthestPlaneIntersections.x, min(FurthestPlaneIntersections.y, FurthestPlaneIntersections.z));
	//clamp the intersections to be between RayOrigin and RayEnd on the ray
	return saturate(BoxIntersections);
}

float4 WorldToView(float4 WorldPos, float4x4 View)
{
	return mul(WorldPos, View);
}

float4 ViewToWolrd(float4 ViewPos, float4x4 InvView)
{
	return mul(ViewPos, InvView);
}

float4 ViewToNDC(float4 ViewPos, float4x4 Proj)
{
	float4 NDC = mul(ViewPos, Proj);
	NDC /= NDC.w;
	
	return NDC;
}

float4 NDCToView(float4 NDCPos, float4x4 InvProj)
{
    float4 View = mul(NDCPos, InvProj);
    View /= View.w;
 
    return View;
}

float NDCDepthToViewDepth(float NDCDepth, float4x4 Proj)
{
	float ViewDepth = Proj[3][2] / (NDCDepth - Proj[2][2]);
	
	return ViewDepth;
}

float ViewDepthToNDCDepth(float ViewDepth, float4x4 Proj)
{
	float NDCDepth = Proj[2][2] + Proj[3][2] / ViewDepth;
	
	return NDCDepth;
}

float2 NDCToUV(float4 NDCPos)
{
	return float2(0.5 + 0.5 * NDCPos.x, 0.5 - 0.5 * NDCPos.y);
}

float4 UVToNDC(float2 UVPos, float Depth)
{
	return float4(2 * UVPos.x - 1, 1 - 2 * UVPos.y, Depth, 1.0f);
}

float2 UVToScreen(float2 UVPos, float2 ScreenSize)
{
	return UVPos * ScreenSize;
}

float2 ScreenToUV(float2 ScreenPos, float2 ScreenSize)
{
	return ScreenPos /= ScreenSize;
}

float2 WorldToUV(float4 WorldPos, float4x4 View, float4x4 Proj)
{
	float4 ViewPos = WorldToView(WorldPos, View);
	
	float4 NDC = ViewToNDC(ViewPos, Proj);
	
	float2 UV = NDCToUV(NDC);
	
	return UV;
}

float4 UVToWorld(float2 UV, float NDCDepth, float4x4 InvProj, float4x4 InvView)
{
	float4 NDC = UVToNDC(UV, NDCDepth);
	
	float4 View = NDCToView(NDC, InvProj);
	
	float4 World = ViewToWolrd(View, InvView);
	
	return World;
}

float2 ViewToUV(float4 ViewPos, float4x4 Proj)
{	
	float4 NDC = ViewToNDC(ViewPos, Proj);
	
	float2 UV = NDCToUV(NDC);
	
	return UV;
}

float4 UVToView(float2 UV, float NDCDepth, float4x4 InvProj)
{
	float4 NDC = UVToNDC(UV, NDCDepth);
	
	float4 View = NDCToView(NDC, InvProj);
	
	return View;
}

float4 ScreenToView(float2 ScreenPos, float2 ScreenSize, float NDCDepth, float4x4 InvProj)
{
	float2 UV = ScreenToUV(ScreenPos, ScreenSize);
	
	float4 NDC = UVToNDC(UV, NDCDepth);
	
	float4 View = NDCToView(NDC, InvProj);
	
	return View;
}

float2 PackHalfFloat(float Value)
{
	const float2 Shift = float2(256, 1.0);
	const float2 Mask = float2(0, 1.0 / 256.0);
	float2 Comp = frac(Value * Shift);
	Comp -= Comp.xx * Mask;
	return Comp;
}

float UnpackHalfFloat(float2 RG)
{
	const float2 Shift = float2(1.0 / 256.0, 1.0);
	return dot(RG, Shift);
}

#endif //__SHADER_UTILS__