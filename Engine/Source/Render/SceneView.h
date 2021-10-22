#pragma once
#include "D3D12/D3D12Utils.h"
#include "Math/Math.h"

struct TSceneView
{
	TVector3 EyePos = TVector3::Zero;
	TMatrix View = TMatrix::Identity;
	TMatrix Proj = TMatrix::Identity;

	float Near;
	float Far;
};