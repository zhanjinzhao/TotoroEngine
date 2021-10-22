#include "AreaLightActor.h"

TAreaLightActor::TAreaLightActor(const std::string& Name)
	:TLightActor(Name, ELightType::AreaLight)
{
	MeshComponent = AddComponent<TMeshComponent>();

	RootComponent = MeshComponent;

	// Mesh
	MeshComponent->SetMeshName("QuadMesh");

	// Material
	MeshComponent->SetMaterialInstance("EmissiveMatInst");
}

TAreaLightActor::~TAreaLightActor()
{

}

void TAreaLightActor::SetLightColor(const TVector3& InColor)
{
	TLightActor::SetLightColor(InColor);

	UpdateEmissiveMaterial();
}

void TAreaLightActor::SetLightIntensity(float InIntensity)
{
	TLightActor::SetLightIntensity(InIntensity);

	UpdateEmissiveMaterial();
}

void TAreaLightActor::UpdateEmissiveMaterial()
{
	TVector3 EmissiveColor = Color * Intensity;
	MeshComponent->GetMaterialInstance()->Parameters.EmissiveColor = EmissiveColor;
}