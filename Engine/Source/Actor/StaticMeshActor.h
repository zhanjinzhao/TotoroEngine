#pragma once

#include "Actor/Actor.h"
#include "Component/MeshComponent.h"

class TStaticMeshActor : public TActor
{
public:
	TStaticMeshActor(const std::string& Name);

	~TStaticMeshActor();

	void SetMesh(std::string MeshName);

	void SetMaterialInstance(std::string MaterialInstanceName);

	void SetTextureScale(const TVector2& Scale);

	void SetUseSDF(bool bUseSDF);

private:
	TMeshComponent* StaticMeshComponent = nullptr;
};
