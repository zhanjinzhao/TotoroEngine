#pragma once

#include <DirectXCollision.h>
#include <vector>
#include "Vertex.h"
#include "Math/Transform.h"
#include "Mesh/Ray.h"

class TBoundingBox
{
public:
	void Init(std::vector<TVector3> Points);

	void Init(std::vector<TVertex> Vertices);

	TVector3 GetCenter() const { return (Min + Max) * 0.5f; }

	TVector3 GetExtend() const { return (Max - Min) * 0.5f; }

	TVector3 GetSize() const { return  (Max - Min); }

	int GetWidestAxis() const;

	float GetMaxWidth() const;

	float GetSurfaceArea() const;

	static TBoundingBox Union(const TBoundingBox& BoxA, const TBoundingBox& BoxB);

	static TBoundingBox Union(const TBoundingBox& Box, const TVector3& Point);

	TBoundingBox Transform(const TTransform& T);

	// If the ray¡¯s origin is inside the box, 0 is returned for Dist0
	bool Intersect(const TRay& Ray, float& Dist0, float& Dist1);

	DirectX::BoundingBox GetD3DBox();

public:
	bool bInit = false;

	TVector3 Min = TVector3(TMath::Infinity);
	TVector3 Max = TVector3(-TMath::Infinity);
};