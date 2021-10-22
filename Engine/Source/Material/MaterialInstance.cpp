#include "MaterialInstance.h"
#include "Render/RenderProxy.h"
#include "D3D12/D3D12RHI.h"

TMaterialInstance::TMaterialInstance(TMaterial* Parent, const std::string& InName)
	:Material(Parent), Name(InName)
{
	Parameters = Material->Parameters;
}

void TMaterialInstance::SetTextureParamter(const std::string& Parameter, const std::string& TextureName)
{
	auto Iter = Parameters.TextureMap.find(Parameter);

	if (Iter != Parameters.TextureMap.end())
	{
		Iter->second = TextureName;
	}
}

void TMaterialInstance::CreateMaterialConstantBuffer(TD3D12RHI* D3D12RHI)
{
	TMaterialConstants MatConst;

	//Get material params
	MatConst.DiffuseAlbedo = Parameters.DiffuseAlbedo;
	MatConst.FresnelR0 =Parameters.FresnelR0;
	MatConst.Roughness = Parameters.Roughness;
	MatConst.MatTransform = Parameters.MatTransform;
	MatConst.EmissiveColor = Parameters.EmissiveColor;
	MatConst.ShadingModel = (UINT)Material->ShadingModel;

	//Create ConstantBuffer
	MaterialConstantBuffer = D3D12RHI->CreateConstantBuffer(&MatConst, sizeof(MatConst));
}