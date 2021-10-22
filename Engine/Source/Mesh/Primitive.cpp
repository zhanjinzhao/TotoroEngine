#include "Primitive.h"
#include "Mesh/MeshRepository.h"

bool TPrimitive::GetLocalBoundingBox(TBoundingBox& OutBox) const
{
	if (BoundingBox.bInit)
	{
		OutBox = BoundingBox;

		return true;
	}
	else
	{
		return false;
	}
}

bool TPrimitive::GetWorldBoundingBox(TBoundingBox& OutBox) const
{
	TBoundingBox LocalBox;

	if (GetLocalBoundingBox(LocalBox))
	{
		OutBox = LocalBox.Transform(WorldTransform);

		return true;
	}
	else
	{
		return false;
	}
}

void TLine::GenerateBoundingBox()
{
	std::vector<TVector3> Points;
	Points.push_back(PointA);
	Points.push_back(PointB);

	BoundingBox.Init(Points);
}

void TTriangle::GenerateBoundingBox()
{
	std::vector<TVector3> Points;
	Points.push_back(PointA);
	Points.push_back(PointB);
	Points.push_back(PointC);

	BoundingBox.Init(Points);
}

// Ref: "Fast, Minimum Storage Ray-Triangle Intersection"
// https://cadxfem.org/inf/Fast%20MinimumStorage%20RayTriangle%20Intersection.pdf
bool TTriangle::Intersect(const TRay& Ray, float& Dist, bool& bBackFace)
{
	const float EPSILON = 0.000001f;

	TVector3 Dir = Ray.Direction;
	TVector3 Orig = Ray.Origin;

	// Find vectors for two edges sharing PointA
	TVector3 Edge1 = PointB - PointA;
	TVector3 Edge2 = PointC - PointA;

	// Begin calculating determinant, also used to calculate U parameter
	TVector3 PVec = Dir.Cross(Edge2);

	// If determinant is near zero, ray lies in plane of f triangle
	float Det = Edge1.Dot(PVec);

	if (Det > -EPSILON && Det < EPSILON)
	{
		return false;
	}

	float InvDet = 1.0f / Det;

	// Calculate distance from vert to ray origin
	TVector3 TVec = Orig - PointA;

	// Calculate U parameter and test bounds 
	float U = TVec.Dot(PVec) * InvDet;
	if (U < 0.0f || U > 1.0f)
	{
		return false;
	}

	// Prepare to test V parameter
	TVector3 QVec = TVec.Cross(Edge1);

	// Calculate V parameter and test bounds
	float V = Dir.Dot(QVec) * InvDet;
	if (V < 0.0f || U + V > 1.0f)
	{
		return false;
	}

	// Calculate T
	float T = Edge2.Dot(QVec) * InvDet;

	if (T < 0.0f)
	{
		return false;
	}

	float TValue = std::abs(T);
	if(TValue > Ray.MaxDist)
	{
		return false;
	}

	Dist = TValue;
	bBackFace = Det < 0.0f ? true : false;

	return true;
}

void TMeshPrimitive::GenerateBoundingBox()
{
	TMesh& Mesh = TMeshRepository::Get().MeshMap.at(MeshName);

	BoundingBox = Mesh.GetBoundingBox();
}