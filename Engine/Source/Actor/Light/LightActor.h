#pragma once

#include "Actor/Actor.h"
#include "Render/ShadowMap.h"

enum ELightType
{
	None,
	AmbientLight,
	DirectionalLight,
	PointLight,
	SpotLight,
	AreaLight
};

class TLightActor : public TActor
{
public:
	TLightActor(const std::string& Name, ELightType InType);

	~TLightActor();

	ELightType GetType()
	{
		return Type;
	}

public:
	TVector3 GetLightColor() const
	{
		return Color;
	}

	virtual void SetLightColor(const TVector3& InColor)
	{
		Color = InColor;
	}

	float GetLightIntensity() const 
	{
		return Intensity;
	}

	virtual void SetLightIntensity(float InIntensity)
	{
		Intensity = InIntensity;
	}

	bool IsCastShadows() const
	{
		return bCastShadows;
	}

	void SetCastShadows(bool bCast)
	{
		bCastShadows = bCast;
	}

	bool IsDrawDebug()
	{
		return bDrawDebug;
	}

	void SetDrawDebug(bool bDraw)
	{
		bDrawDebug = bDraw;
	}

	bool IsDrawMesh()
	{
		return bDrawMesh;
	}

	void SetDrawMesh(bool bDraw)
	{
		bDrawMesh = bDraw;
	}

	virtual void UpdateShadowInfo(TShadowMap2D* ShadowMap)
	{}

protected:
	ELightType Type = ELightType::None;

	TVector3 Color = TVector3::One;

	float Intensity = 10.0f;

	bool bCastShadows = true;

	bool bDrawDebug = false;

	bool bDrawMesh = false;
};
