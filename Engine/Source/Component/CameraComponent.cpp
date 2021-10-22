#include "CameraComponent.h"

TCameraComponent::TCameraComponent()
{
	SetLens(0.25f * TMath::Pi, 1.0f, 0.1f, 100.0f);
}

TCameraComponent::~TCameraComponent()
{
}

void TCameraComponent::SetWorldLocation(const TVector3& Location)
{
	WorldTransform.Location = Location;
	ViewDirty = true;
}

float TCameraComponent::GetNearZ()const
{
	return NearZ;
}

float TCameraComponent::GetFarZ()const
{
	return FarZ;
}

float TCameraComponent::GetAspect()const
{
	return Aspect;
}

float TCameraComponent::GetFovY()const
{
	return FovY;
}

float TCameraComponent::GetFovX()const
{
	float halfWidth = 0.5f * GetNearWindowWidth();
	return float(2.0f * atan(halfWidth / NearZ));
}

float TCameraComponent::GetNearWindowWidth()const
{
	return Aspect * NearWindowHeight;
}

float TCameraComponent::GetNearWindowHeight()const
{
	return NearWindowHeight;
}

float TCameraComponent::GetFarWindowWidth()const
{
	return Aspect * FarWindowHeight;
}

float TCameraComponent::GetFarWindowHeight()const
{
	return FarWindowHeight;
}

void TCameraComponent::SetLens(float fovY, float aspect, float zn, float zf)
{
	// cache properties
	FovY = fovY;
	Aspect = aspect;
	NearZ = zn;
	FarZ = zf;

	NearWindowHeight = 2.0f * NearZ * tanf(0.5f * FovY);
	FarWindowHeight = 2.0f * FarZ * tanf(0.5f * FovY);

	Proj = TMatrix::CreatePerspectiveFieldOfView(FovY, Aspect, NearZ, FarZ);
}

void TCameraComponent::LookAt(const TVector3& pos, const TVector3& target, const TVector3& up)
{
	TVector3 L = target - pos;
	L.Normalize();
	TVector3 R = up.Cross(L);
	R.Normalize();
	TVector3 U = L.Cross(R);

	WorldTransform.Location = pos;
	Look = L;
	Right = R;
	Up = U;

	ViewDirty = true;
}


TMatrix TCameraComponent::GetView()const
{
	assert(!ViewDirty);
	return View;
}

TMatrix TCameraComponent::GetProj()const
{
	return Proj;
}

void TCameraComponent::MoveRight(float Dist)
{
	WorldTransform.Location += Dist * Right;

	ViewDirty = true;
}

void TCameraComponent::MoveForward(float Dist)
{
	WorldTransform.Location += Dist * Look;

	ViewDirty = true;
}

void TCameraComponent::MoveUp(float Dist)
{
	WorldTransform.Location += Dist * Up;

	ViewDirty = true;
}

void TCameraComponent::Pitch(float Degrees)
{
	float Radians = TMath::DegreesToRadians(Degrees);

	// Rotate up and look vector about the right vector.
	TMatrix R = TMatrix::CreateFromAxisAngle(Right, Radians);

	Up = R.TransformNormal(Up);
	Look = R.TransformNormal(Look);

	ViewDirty = true;
}

void TCameraComponent::RotateY(float Degrees)
{
	float Radians = TMath::DegreesToRadians(Degrees);

	// Rotate the basis vectors about the world y-axis.
	TMatrix R = TMatrix::CreateRotationY(Radians);

	Right = R.TransformNormal(Right);
	Up = R.TransformNormal(Up);
	Look = R.TransformNormal(Look);

	ViewDirty = true;
}

void TCameraComponent::UpdateViewMatrix()
{
	if (ViewDirty)
	{
		// Keep camera's axes orthogonal to each other and of unit length.
		Look.Normalize();
		Up = Look.Cross(Right);
		Up.Normalize();

		// Up, Look already ortho-normal, so no need to normalize cross product.
		Right = Up.Cross(Look);

		// Fill in the view matrix entries.
		float x = - WorldTransform.Location.Dot(Right);
		float y = - WorldTransform.Location.Dot(Up);
		float z = - WorldTransform.Location.Dot(Look);

		View(0, 0) = Right.x;
		View(1, 0) = Right.y;
		View(2, 0) = Right.z;
		View(3, 0) = x;

		View(0, 1) = Up.x;
		View(1, 1) = Up.y;
		View(2, 1) = Up.z;
		View(3, 1) = y;

		View(0, 2) = Look.x;
		View(1, 2) = Look.y;
		View(2, 2) = Look.z;
		View(3, 2) = z;

		View(0, 3) = 0.0f;
		View(1, 3) = 0.0f;
		View(2, 3) = 0.0f;
		View(3, 3) = 1.0f;

		ViewDirty = false;
	}
}

void TCameraComponent::SetPrevViewProj(const TMatrix& VP)
{
	PrevViewProj = VP;
}

TMatrix TCameraComponent::GetPrevViewProj() const
{
	return PrevViewProj;
}