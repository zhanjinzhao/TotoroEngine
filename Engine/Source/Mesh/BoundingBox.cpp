#include "BoundingBox.h"

void TBoundingBox::Init(std::vector<TVector3> Points)
{
	if (Points.size() > 0)
	{
		bInit = true;

		for (const TVector3& Point : Points)
		{
			Min = TVector3::Min(Min, Point);
			Max = TVector3::Max(Max, Point);
		}
	}
}

void TBoundingBox::Init(std::vector<TVertex> Vertices)
{
	if (Vertices.size() > 0)
	{
		bInit = true;

		for (const TVertex& Vertex : Vertices)
		{
			TVector3 P = Vertex.Position;

			Min = TVector3::Min(Min, P);
			Max = TVector3::Max(Max, P);
		}
	}
}

DirectX::BoundingBox TBoundingBox::GetD3DBox()
{
	DirectX::BoundingBox D3DBox;

	D3DBox.Center = (Min + Max) * 0.5f;
	D3DBox.Extents = (Max - Min) * 0.5f;

	return D3DBox;
}

int TBoundingBox::GetWidestAxis() const
{
	TVector3 V = Max - Min;

	if (V.x > V.y && V.x > V.z)
		return 0;
	else if (V.y > V.z)
		return 1;
	else
		return 2;
}

float TBoundingBox::GetMaxWidth() const
{
	TVector3 V = Max - Min;

	if (V.x > V.y && V.x > V.z)
		return V.x;
	else if (V.y > V.z)
		return V.y;
	else
		return V.z;
}

float TBoundingBox::GetSurfaceArea() const
{
	if (bInit)
	{
		TVector3 V = Max - Min;

		return 2.0f * (V.x * V.y + V.x * V.z + V.y * V.z);
	}
	else
	{
		return 0.0f;
	}
}

TBoundingBox TBoundingBox::Union(const TBoundingBox& BoxA, const TBoundingBox& BoxB)
{
	TBoundingBox UnionBox;

	if (BoxA.bInit || BoxB.bInit)
	{
		UnionBox.bInit = true;

		UnionBox.Min = TVector3::Min(BoxA.Min, BoxB.Min);
		UnionBox.Max = TVector3::Max(BoxA.Max, BoxB.Max);
	}

	return UnionBox;
}

TBoundingBox TBoundingBox::Union(const TBoundingBox& Box, const TVector3& Point)
{
	TBoundingBox UnionBox;

	UnionBox.bInit = true;

	UnionBox.Min = TVector3::Min(Box.Min, Point);
	UnionBox.Max = TVector3::Max(Box.Max, Point);

	return UnionBox;
}

TBoundingBox TBoundingBox::Transform(const TTransform& T)
{
	TBoundingBox Box;

	if (bInit)
	{
		Box.bInit = true;

		// Transform eight corner points, and calculate new AABB
		TMatrix M = T.GetTransformMatrix();

		Box = Union(Box, M.Transform(TVector3(Min.x, Min.y, Min.z)));
		Box = Union(Box, M.Transform(TVector3(Max.x, Min.y, Min.z)));
		Box = Union(Box, M.Transform(TVector3(Min.x, Max.y, Min.z)));
		Box = Union(Box, M.Transform(TVector3(Min.x, Min.y, Max.z)));

		Box = Union(Box, M.Transform(TVector3(Min.x, Max.y, Max.z)));
		Box = Union(Box, M.Transform(TVector3(Max.x, Max.y, Min.z)));
		Box = Union(Box, M.Transform(TVector3(Max.x, Min.y, Max.z)));
		Box = Union(Box, M.Transform(TVector3(Max.x, Max.y, Max.z)));
	}

	return Box;
}

// Ref: pbrt-v3
bool TBoundingBox::Intersect(const TRay& Ray, float& Dist0, float& Dist1)
{
	float t0 = 0, t1 = Ray.MaxDist;
	for (int i = 0; i < 3; ++i)
	{
		// Update interval for i'th bounding box slab
		float InvRayDir = 1.0f / Ray.Direction[i];
		float tNear = (Min[i] - Ray.Origin[i]) * InvRayDir;
		float tFar = (Max[i] - Ray.Origin[i]) * InvRayDir;

		// Update parametric interval from slab intersection t values
		if (tNear > tFar) std::swap(tNear, tFar);

		// Update tFar to ensure robust ray--bounds intersection
		tFar *= 1 + 2 * TMath::gamma(3);
		t0 = tNear > t0 ? tNear : t0;
		t1 = tFar < t1 ? tFar : t1;
		if (t0 > t1) return false;
	}

	Dist0 = t0;
	Dist1 = t1;

	return true;
}