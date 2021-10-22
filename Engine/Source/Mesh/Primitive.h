#pragma once

#include "Color.h"
#include "Mesh/BoundingBox.h"
#include "Mesh/Ray.h"
#include <string>

class TPrimitive
{
public:
	TPrimitive() {}

	virtual ~TPrimitive() {}

	virtual void GenerateBoundingBox() {}

	bool GetLocalBoundingBox(TBoundingBox& OutBox) const;

	bool GetWorldBoundingBox(TBoundingBox& OutBox) const;

	virtual bool Intersect(const TRay& Ray, float& Dist, bool& bBackFace) { return false; }

protected:
	TTransform WorldTransform;

	TBoundingBox BoundingBox;
};

class TPoint : public TPrimitive
{
public:
	TPoint() = default;

	TPoint(const TVector3& InPoint, const TColor& InColor)
		:Point(InPoint), Color(InColor)
	{}

public:
	TVector3 Point;
	TColor Color;
};

class TLine : public TPrimitive
{
public:
	TLine() = default;

	TLine(const TVector3& InPointA, const TVector3& InPointB, const TColor& InColor)
		:PointA(InPointA), PointB(InPointB), Color(InColor)
	{}

	virtual void GenerateBoundingBox() override;

public:
	TVector3 PointA;
	TVector3 PointB;
	TColor Color;
};


class TTriangle : public TPrimitive
{
public:
	TTriangle() = default;

	TTriangle(const TVector3& InPointA, const TVector3& InPointB, const TVector3& InPointC, const TColor& InColor)
		:PointA(InPointA), PointB(InPointB), PointC(InPointC), Color(InColor)
	{}

	TTriangle(const TTriangle& Other) = default;

	virtual void GenerateBoundingBox() override;

	// Don't cull backfacing triangles
	// Negative value will return for Dist when intersect backfacing triangle
	virtual bool Intersect(const TRay& Ray, float& Dist, bool& bBackFace) override;

public:
	TVector3 PointA;
	TVector3 PointB;
	TVector3 PointC;
	TColor Color;
};

class TMeshPrimitive : public TPrimitive
{
public:
	TMeshPrimitive(const std::string& InMeshName, const TTransform& InWorldTransform)
		:MeshName(InMeshName)
	{
		WorldTransform = InWorldTransform;
	}

	virtual void GenerateBoundingBox() override;

private:
	std::string MeshName;
};