#pragma once

#include "Math/Math.h"

class TRay
{
public:
	TRay()
		:MaxDist(TMath::Infinity)
	{}

	TRay(const TVector3& InOrigin, const TVector3& InDirection, float InMaxDist = TMath::Infinity)
		:Origin(InOrigin), Direction(InDirection), MaxDist(InMaxDist)
	{}

public:
	TVector3 Origin;

	TVector3 Direction;

	mutable float MaxDist;
};