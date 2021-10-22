#pragma once

#include "LightActor.h"
#include "Component/MeshComponent.h"

class TAreaLightActor : public TLightActor
{
public:
	TAreaLightActor(const std::string& Name);

	~TAreaLightActor();

	virtual void SetLightColor(const TVector3& InColor) override;

	virtual void SetLightIntensity(float InIntensity) override;

private:
	void UpdateEmissiveMaterial();

private:
	TMeshComponent* MeshComponent = nullptr;
};