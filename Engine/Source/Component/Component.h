#pragma once

#include "Math/Transform.h"

class TComponent
{
public:
	TComponent() {}

	virtual ~TComponent() {}

public:
	virtual void SetWorldLocation(const TVector3& Location)
	{
		WorldTransform.Location = Location;
	}

	TVector3 GetWorldLocation() const
	{
		return WorldTransform.Location;
	}

	virtual void SetWorldRotation(const TRotator& Rotation)
	{
		WorldTransform.Rotation = Rotation;
	}

	TRotator GetWorldRotation() const
	{
		return WorldTransform.Rotation;
	}

	void SetWorldTransform(const TTransform& Transform)
	{
		WorldTransform = Transform;
	}

	TTransform GetWorldTransform() const
	{
		return WorldTransform;
	}

	void SetPrevWorldTransform(const TTransform& Transform)
	{
		PrevWorldTransform = Transform;
	}

	TTransform GetPrevWorldTransform() const
	{
		return PrevWorldTransform;
	}

protected:
	TTransform RelativeTransform; //TODO

	TTransform WorldTransform;

	TTransform PrevWorldTransform;
};
