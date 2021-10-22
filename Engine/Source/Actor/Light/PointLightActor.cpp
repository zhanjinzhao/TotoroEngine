#include "PointLightActor.h"


TPointLightActor::TPointLightActor(const std::string& Name)
	:TLightActor(Name, ELightType::PointLight)
{
	MeshComponent = AddComponent<TMeshComponent>();

	RootComponent = MeshComponent;

	//Mesh
	MeshComponent->SetMeshName("SphereMesh");

	//Material
	MeshComponent->SetMaterialInstance("DefaultMatInst");
}

TPointLightActor::~TPointLightActor()
{

}

void TPointLightActor::UpdateShadowData(TD3D12RHI* D3D12RHI)
{
	if (ShadowMap == nullptr)
	{
		const UINT ShadowSize = 4096;
		ShadowMap = std::make_unique<TShadowMapCube>(ShadowSize, DXGI_FORMAT_R24G8_TYPELESS, D3D12RHI);
	}

	TVector3 LightPos = GetActorLocation();

	float Near = 0.1f;
	float Far = AttenuationRange;

	ShadowMap->CreatePerspectiveViews(LightPos, Near, Far);
}
