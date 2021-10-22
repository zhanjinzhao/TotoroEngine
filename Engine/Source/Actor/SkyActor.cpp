#include "SkyActor.h"

using namespace DirectX;

TSkyActor::TSkyActor(const std::string& Name)
	:TActor(Name)
{
	MeshComponent = AddComponent<TMeshComponent>();

	RootComponent = MeshComponent;

	//Mesh
	MeshComponent->SetMeshName("SphereMesh");
	MeshComponent->bUseSDF = false;
}

TSkyActor::~TSkyActor()
{}

void TSkyActor::SetMaterialInstance(std::string MaterialInstanceName)
{
	MeshComponent->SetMaterialInstance(MaterialInstanceName);
}