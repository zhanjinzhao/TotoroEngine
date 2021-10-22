#include "StaticMeshActor.h"


TStaticMeshActor::TStaticMeshActor(const std::string& Name)
	:TActor(Name)
{
	StaticMeshComponent = AddComponent<TMeshComponent>();

	RootComponent = StaticMeshComponent;
}
TStaticMeshActor::~TStaticMeshActor()
{}

void TStaticMeshActor::SetMesh(std::string MeshName)
{
	StaticMeshComponent->SetMeshName(MeshName);
}

void TStaticMeshActor::SetMaterialInstance(std::string MaterialInstanceName)
{
	StaticMeshComponent->SetMaterialInstance(MaterialInstanceName);
}

void TStaticMeshActor::SetTextureScale(const TVector2& Scale)
{
	StaticMeshComponent->TexTransform = TMatrix::CreateScale(Scale.x, Scale.y, 1.0f);
}

void TStaticMeshActor::SetUseSDF(bool bUseSDF)
{
	StaticMeshComponent->bUseSDF = bUseSDF;
}