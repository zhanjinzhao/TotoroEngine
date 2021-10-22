#include "DirectionalLightActor.h"

using namespace DirectX;

TDirectionalLightActor::TDirectionalLightActor(const std::string& Name)
	:TLightActor(Name, ELightType::DirectionalLight)
{
	MeshComponent = AddComponent<TMeshComponent>();

	RootComponent = MeshComponent;

	//Mesh
	MeshComponent->SetMeshName("CylinderMesh");

	//Material
	MeshComponent->SetMaterialInstance("DefaultMatInst");
}

TDirectionalLightActor::~TDirectionalLightActor()
{

}

void TDirectionalLightActor::SetActorTransform(const TTransform& NewTransform)
{
	TActor::SetActorTransform(NewTransform);

	SetLightDirection(NewTransform.Rotation);
}

void TDirectionalLightActor::SetLightDirection(TRotator Rotation)
{
	//Calculate Direction
	TMatrix R = TMatrix::CreateFromYawPitchRoll(Rotation.Yaw * TMath::Pi / 180.0f, Rotation.Pitch * TMath::Pi / 180.0f, Rotation.Roll * TMath::Pi / 180.0f);
	Direction = R.TransformNormal(TVector3::Up);
}

TVector3 TDirectionalLightActor::GetLightDirection() const
{
	return Direction;
}

void TDirectionalLightActor::UpdateShadowData(TD3D12RHI* D3D12RHI, EShadowMapImpl SMImpl)
{
	if (ShadowMap == nullptr)
	{
		const UINT ShadowSize = 4096;
		ShadowMap = std::make_unique<TShadowMap2D>(ShadowSize, ShadowSize, DXGI_FORMAT_R24G8_TYPELESS, D3D12RHI);

		if (SMImpl == EShadowMapImpl::VSM)
		{
			TTextureInfo TextureInfo;
			TextureInfo.Type = ETextureType::TEXTURE_2D;
			TextureInfo.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
			TextureInfo.Width = ShadowSize;
			TextureInfo.Height = ShadowSize;
			TextureInfo.Depth = 1;
			TextureInfo.ArraySize = 1;
			TextureInfo.MipCount = 1;
			TextureInfo.Format = DXGI_FORMAT_R16G16_FLOAT;
			TextureInfo.InitState = D3D12_RESOURCE_STATE_COMMON;

			VSMTexture = D3D12RHI->CreateTexture(TextureInfo, TexCreate_SRV | TexCreate_UAV);

			VSMBlurTexture = D3D12RHI->CreateTexture(TextureInfo, TexCreate_SRV | TexCreate_UAV);
		}
	}

	// TODO: calculate shadow bounds according to the level of CSM
	TVector3 ShadowBounds = TVector3(50.0f, 50.0f, 20.0f);

	TVector3 LightPos = GetActorLocation();
	TVector3 TargetPos = LightPos + GetLightDirection();
	TVector3 LightUp = TVector3::Up;

	float Left   = - ShadowBounds.x * 0.5f;
	float Right  =   ShadowBounds.x * 0.5f;
	float Bottom = - ShadowBounds.y * 0.5f;
	float Top    =   ShadowBounds.y * 0.5f;
	float Near   =   0.0f;
	float Far    =   ShadowBounds.z;

	ShadowMap->CreateOrthographicView(LightPos, TargetPos, LightUp, Left, Right, Bottom, Top, Near, Far);
}