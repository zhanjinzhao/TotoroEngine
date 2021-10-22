#pragma once

#include "LightActor.h"
#include "Component/MeshComponent.h"

class TPointLightActor : public TLightActor
{
public:
	TPointLightActor(const std::string& Name);

	~TPointLightActor();

	float GetAttenuationRange() const
	{
		return AttenuationRange;
	}

	void SetAttenuationRange(float Radius)
	{
		AttenuationRange = Radius;
	}

	void UpdateShadowData(TD3D12RHI* D3D12RHI);

	TShadowMapCube* GetShadowMap() { return ShadowMap.get(); }

private:
	float AttenuationRange = 10.0f;

	TMeshComponent* MeshComponent = nullptr;

	std::unique_ptr<TShadowMapCube> ShadowMap = nullptr;
};
