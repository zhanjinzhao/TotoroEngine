#include "MaterialRepository.h"

TMaterialRepository& TMaterialRepository::Get()
{
	static TMaterialRepository Instance;
	return Instance;
}

void TMaterialRepository::Load()
{
	//--------------------------------------DefaultMat------------------------------------------------------
	{
		// Material
		TMaterial* DefaultMat = CreateMaterial("DefaultMat", "BasePassDefault");

		TMaterialParameters& Parameters = DefaultMat->Parameters;
		Parameters.TextureMap.emplace("BaseColorTexture", "NullTex");
		Parameters.TextureMap.emplace("NormalTexture", "NullTex");
		Parameters.TextureMap.emplace("MetallicTexture", "NullTex");
		Parameters.TextureMap.emplace("RoughnessTexture", "NullTex");

		// MaterialInstances
		CreateDefaultMaterialInstance(DefaultMat);

		{
			TMaterialInstance* AssaultRifleMatInst = CreateMaterialInstance(DefaultMat, "AssaultRifleMatInst");

			AssaultRifleMatInst->SetTextureParamter("BaseColorTexture", "AssaultRifle_BaseColor");
			AssaultRifleMatInst->SetTextureParamter("NormalTexture", "AssaultRifle_Normal");
			AssaultRifleMatInst->SetTextureParamter("MetallicTexture", "AssaultRifle_Metallic");
			AssaultRifleMatInst->SetTextureParamter("RoughnessTexture", "AssaultRifle_Roughness");
		}

		{
			TMaterialInstance* CyborgWeaponMatInst = CreateMaterialInstance(DefaultMat, "CyborgWeaponMatInst");

			CyborgWeaponMatInst->SetTextureParamter("BaseColorTexture", "CyborgWeapon_BaseColor");
			CyborgWeaponMatInst->SetTextureParamter("NormalTexture", "CyborgWeapon_Normal");
			CyborgWeaponMatInst->SetTextureParamter("MetallicTexture", "CyborgWeapon_Metallic");
			CyborgWeaponMatInst->SetTextureParamter("RoughnessTexture", "CyborgWeapon_Roughness");
		}

		{
			TMaterialInstance* HelmetMatInst = CreateMaterialInstance(DefaultMat, "HelmetMatInst");
			HelmetMatInst->SetTextureParamter("BaseColorTexture", "Helmet_BaseColor");
			HelmetMatInst->SetTextureParamter("NormalTexture", "Helmet_Normal");
			HelmetMatInst->SetTextureParamter("MetallicTexture", "Helmet_Metallic");
			HelmetMatInst->SetTextureParamter("RoughnessTexture", "Helmet_Roughness");
		}

		{
			TMaterialInstance* RustedIronMatInst = CreateMaterialInstance(DefaultMat, "RustedIronMatInst");
			RustedIronMatInst->SetTextureParamter("BaseColorTexture", "RustedIron_BaseColor");
			RustedIronMatInst->SetTextureParamter("NormalTexture", "RustedIron_Normal");
			RustedIronMatInst->SetTextureParamter("MetallicTexture", "RustedIron_Metallic");
			RustedIronMatInst->SetTextureParamter("RoughnessTexture", "RustedIron_Roughness");
		}

		{
			TMaterialInstance* GrayGraniteMatInst = CreateMaterialInstance(DefaultMat, "GrayGraniteMatInst");
			GrayGraniteMatInst->SetTextureParamter("BaseColorTexture", "GrayGranite_BaseColor");
			GrayGraniteMatInst->SetTextureParamter("NormalTexture", "GrayGranite_Normal");
			GrayGraniteMatInst->SetTextureParamter("MetallicTexture", "GrayGranite_Metallic");
			GrayGraniteMatInst->SetTextureParamter("RoughnessTexture", "GrayGranite_Roughness");
		}

		{
			TMaterialInstance* FloorMatInst = CreateMaterialInstance(DefaultMat, "FloorMatInst");
			FloorMatInst->SetTextureParamter("BaseColorTexture", "Floor_BaseColor");
			FloorMatInst->SetTextureParamter("NormalTexture", "Floor_Normal");
			FloorMatInst->SetTextureParamter("MetallicTexture", "Floor_Metallic");
			FloorMatInst->SetTextureParamter("RoughnessTexture", "Floor_Roughness");
		}

		{
			TMaterialInstance* ColumnMatInst = CreateMaterialInstance(DefaultMat, "ColumnMatInst");
			ColumnMatInst->SetTextureParamter("BaseColorTexture", "Column_BaseColor");
			ColumnMatInst->SetTextureParamter("NormalTexture", "Column_Normal");
			ColumnMatInst->SetTextureParamter("MetallicTexture", "NullTex");
			ColumnMatInst->SetTextureParamter("RoughnessTexture", "Column_Roughness");
		}

		{
			TMaterialInstance* EmissiveMatInst = CreateMaterialInstance(DefaultMat, "EmissiveMatInst");

			TMaterialParameters& Parameters = EmissiveMatInst->Parameters;
			Parameters.EmissiveColor = TVector3(1.0f);
			EmissiveMatInst->SetTextureParamter("BaseColorTexture", "NullTex");
			EmissiveMatInst->SetTextureParamter("NormalTexture", "NullTex");
			EmissiveMatInst->SetTextureParamter("MetallicTexture", "NullTex");
			EmissiveMatInst->SetTextureParamter("RoughnessTexture", "NullTex");
		}
	}



	//--------------------------------------SkyMat------------------------------------------------------
	{
		// Material
		TMaterial* SkyMat = CreateMaterial("SkyMat", "BasePassSky");

		TMaterialParameters& Parameters = SkyMat->Parameters;
		Parameters.TextureMap.emplace("SkyCubeTexture", "Shiodome_Stairs");

		TMaterialRenderState& RenderState = SkyMat->RenderState;
		RenderState.CullMode = D3D12_CULL_MODE_NONE;
		RenderState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

		SkyMat->ShadingModel = EShadingMode::Unlit;

		// MaterialInstance
		CreateDefaultMaterialInstance(SkyMat);
	}
}

void TMaterialRepository::Unload()
{
	MaterialInstanceMap.clear();

	MaterialMap.clear();
}

TMaterial* TMaterialRepository::CreateMaterial(const std::string& MaterialName, const std::string& ShaderName)
{
	MaterialMap.insert({ MaterialName, std::make_unique<TMaterial>(MaterialName, ShaderName)});

	return MaterialMap[MaterialName].get();
}

TMaterialInstance* TMaterialRepository::CreateMaterialInstance(TMaterial* Material, const std::string& MaterialInstanceName)
{
	MaterialInstanceMap.insert({ MaterialInstanceName, std::make_unique<TMaterialInstance>(Material, MaterialInstanceName) });

	return MaterialInstanceMap[MaterialInstanceName].get();
}

void TMaterialRepository::CreateDefaultMaterialInstance(TMaterial* Material)
{
	CreateMaterialInstance(Material, Material->Name + "Inst");
}

TMaterialInstance* TMaterialRepository::GetMaterialInstance(const std::string& MaterialInstanceName) const
{
	TMaterialInstance* Result = nullptr;

	auto Iter = MaterialInstanceMap.find(MaterialInstanceName);
	if (Iter != MaterialInstanceMap.end())
	{
		Result = Iter->second.get();
	}

	return Result;
}