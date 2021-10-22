#pragma once

#include "Component.h"

class TCameraComponent : public TComponent
{
public:
	TCameraComponent();
	~TCameraComponent();

	// Location
	virtual void SetWorldLocation(const TVector3& Location) override;

	// Get frustum properties.
	float GetNearZ()const;
	float GetFarZ()const;
	float GetAspect()const;
	float GetFovY()const;
	float GetFovX()const;

	// Get near and far plane dimensions in view space coordinates.
	float GetNearWindowWidth()const;
	float GetNearWindowHeight()const;
	float GetFarWindowWidth()const;
	float GetFarWindowHeight()const;

	// Set frustum.
	void SetLens(float fovY, float aspect, float zn, float zf);

	// Define camera space via LookAt parameters.
	void LookAt(const TVector3& pos, const TVector3& target, const TVector3& up);

	// Get View/Proj matrices.
	TMatrix GetView()const;
	TMatrix GetProj()const;

	// Move the camera a distance.
	void MoveRight(float Dist);
	void MoveForward(float Dist);
	void MoveUp(float Dist);

	// Rotate the camera.
	void Pitch(float Degrees);
	void RotateY(float Degrees);

	// After modifying camera position/orientation, call to rebuild the view matrix.
	void UpdateViewMatrix();

	void SetPrevViewProj(const TMatrix& VP);

	TMatrix GetPrevViewProj() const;

private:

	// Camera coordinate system with coordinates relative to world space.
	TVector3 Right = { 1.0f, 0.0f, 0.0f };
	TVector3 Up = { 0.0f, 1.0f, 0.0f };
	TVector3 Look = { 0.0f, 0.0f, 1.0f };

	// Cache frustum properties.
	float NearZ = 0.0f;
	float FarZ = 0.0f;
	float Aspect = 0.0f;
	float FovY = 0.0f;
	float NearWindowHeight = 0.0f;
	float FarWindowHeight = 0.0f;

	bool ViewDirty = true;

	// Cache View/Proj matrices.
	TMatrix View = TMatrix::Identity;
	TMatrix Proj = TMatrix::Identity;

	TMatrix PrevViewProj = TMatrix::Identity;
};
