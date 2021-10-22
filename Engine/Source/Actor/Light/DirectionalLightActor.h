#pragma once

#include "LightActor.h"
#include "Component/MeshComponent.h"

class TDirectionalLightActor : public TLightActor
{
public:
	TDirectionalLightActor(const std::string& Name);

	~TDirectionalLightActor();

public:
	virtual void SetActorTransform(const TTransform& NewTransform) override;

	TVector3 GetLightDirection() const;

	void UpdateShadowData(TD3D12RHI* D3D12RHI, EShadowMapImpl SMImpl);

	TShadowMap2D* GetShadowMap() { return ShadowMap.get(); }

	TD3D12TextureRef GetVSMTexture() { return VSMTexture; }

	TD3D12TextureRef GetVSMBlurTexture() { return VSMBlurTexture; }

private:
	void SetLightDirection(TRotator Rotation);

private:
	TVector3 Direction;

	TMeshComponent* MeshComponent = nullptr;

	std::unique_ptr<TShadowMap2D> ShadowMap = nullptr;

	TD3D12TextureRef VSMTexture = nullptr;

	TD3D12TextureRef VSMBlurTexture = nullptr;
};