#include "TextureRepository.h"
#include "File/FileHelpers.h"

TTextureRepository& TTextureRepository::Get()
{
	static TTextureRepository Instance;
	return Instance;
}

void TTextureRepository::Load()
{
	std::wstring TextureDir = TFileHelpers::EngineDir() + L"Resource/Textures/";

	TextureMap.emplace("NullTex", std::make_shared<TTexture2D>("NullTex", false, TextureDir + L"white1x1.dds"));

	// PBR textures
	TextureMap.emplace("AssaultRifle_BaseColor", std::make_shared<TTexture2D>("AssaultRifle_BaseColor", true, TextureDir + L"AssaultRifle_BaseColor.png"));

	TextureMap.emplace("AssaultRifle_Normal", std::make_shared<TTexture2D>("AssaultRifle_Normal", false, TextureDir + L"AssaultRifle_Normal.png"));

	TextureMap.emplace("AssaultRifle_Metallic", std::make_shared<TTexture2D>("AssaultRifle_Metallic", false, TextureDir + L"AssaultRifle_Metallic.png"));

	TextureMap.emplace("AssaultRifle_Roughness", std::make_shared<TTexture2D>("AssaultRifle_Roughness", false, TextureDir + L"AssaultRifle_Roughness.png"));

	TextureMap.emplace("CyborgWeapon_BaseColor", std::make_shared<TTexture2D>("CyborgWeapon_BaseColor", true, TextureDir + L"CyborgWeapon_BaseColor.png"));

	TextureMap.emplace("CyborgWeapon_Normal", std::make_shared<TTexture2D>("CyborgWeapon_Normal", false, TextureDir + L"CyborgWeapon_Normal.png"));

	TextureMap.emplace("CyborgWeapon_Metallic", std::make_shared<TTexture2D>("CyborgWeapon_Metallic", false, TextureDir + L"CyborgWeapon_Metallic.png"));

	TextureMap.emplace("CyborgWeapon_Roughness", std::make_shared<TTexture2D>("CyborgWeapon_Roughness", false, TextureDir + L"CyborgWeapon_Roughness.png"));

	TextureMap.emplace("RustedIron_BaseColor", std::make_shared<TTexture2D>("RustedIron_BaseColor", true, TextureDir + L"rustediron2_basecolor.png"));

	TextureMap.emplace("RustedIron_Normal", std::make_shared<TTexture2D>("RustedIron_Normal", false, TextureDir + L"rustediron2_normal.png"));

	TextureMap.emplace("RustedIron_Metallic", std::make_shared<TTexture2D>("RustedIron_Metallic", false, TextureDir + L"rustediron2_metallic.png"));

	TextureMap.emplace("RustedIron_Roughness", std::make_shared<TTexture2D>("RustedIron_Roughness", false, TextureDir + L"rustediron2_roughness.png"));

	TextureMap.emplace("GrayGranite_BaseColor", std::make_shared<TTexture2D>("GrayGranite_BaseColor", true, TextureDir + L"gray-granite-flecks-albedo.png"));

	TextureMap.emplace("GrayGranite_Normal", std::make_shared<TTexture2D>("GrayGranite_Normal", false, TextureDir + L"gray-granite-flecks-Normal-dx.png"));

	TextureMap.emplace("GrayGranite_Metallic", std::make_shared<TTexture2D>("GrayGranite_Metallic", false, TextureDir + L"gray-granite-flecks-Metallic.png"));

	TextureMap.emplace("GrayGranite_Roughness", std::make_shared<TTexture2D>("GrayGranite_Roughness", false, TextureDir + L"gray-granite-flecks-Roughness.png"));

	TextureMap.emplace("Helmet_BaseColor", std::make_shared<TTexture2D>("Helmet_BaseColor", true, TextureDir + L"helmet_low_DefaultMaterial_BaseColor.png"));

	TextureMap.emplace("Helmet_Normal", std::make_shared<TTexture2D>("Helmet_Normal", false, TextureDir + L"helmet_low_DefaultMaterial_Normal.png"));

	TextureMap.emplace("Helmet_Metallic", std::make_shared<TTexture2D>("Helmet_Metallic", false, TextureDir + L"helmet_low_DefaultMaterial_Metallic.png"));

	TextureMap.emplace("Helmet_Roughness", std::make_shared<TTexture2D>("Helmet_Roughness", false, TextureDir + L"helmet_low_DefaultMaterial_Roughness.png"));

	TextureMap.emplace("Floor_BaseColor", std::make_shared<TTexture2D>("Floor_BaseColor", true, TextureDir + L"hardwood-brown-planks-albedo.png"));

	TextureMap.emplace("Floor_Normal", std::make_shared<TTexture2D>("Floor_Normal", false, TextureDir + L"hardwood-brown-planks-normal-dx.png"));

	TextureMap.emplace("Floor_Metallic", std::make_shared<TTexture2D>("Floor_Metallic", false, TextureDir + L"hardwood-brown-planks-metallic.png"));

	TextureMap.emplace("Floor_Roughness", std::make_shared<TTexture2D>("Floor_Roughness", false, TextureDir + L"hardwood-brown-planks-roughness.png"));

	TextureMap.emplace("Column_BaseColor", std::make_shared<TTexture2D>("Column_BaseColor", true, TextureDir + L"column_albedo.jpg"));

	TextureMap.emplace("Column_Normal", std::make_shared<TTexture2D>("Column_Normal", false, TextureDir + L"column_normal.png"));

	TextureMap.emplace("Column_Roughness", std::make_shared<TTexture2D>("Column_Roughness", false, TextureDir + L"column_roughness.jpg"));

	// LUT
	TextureMap.emplace("IBL_BRDF_LUT", std::make_shared<TTexture2D>("IBL_BRDF_LUT", false, TextureDir + L"IBL_BRDF_LUT.png"));

	// HDR
	TextureMap.emplace("Newport_Loft", std::make_shared<TTexture2D>("Newport_Loft", false, TextureDir + L"Newport_Loft_Ref.hdr"));

	TextureMap.emplace("Shiodome_Stairs", std::make_shared<TTexture2D>("Shiodome_Stairs", false, TextureDir + L"10-Shiodome_Stairs_3k.hdr"));

	// AreaLight(LTC)
	TextureMap.emplace("LtcMat_1", std::make_shared<TTexture2D>("LtcMat_1", false, TextureDir + L"ltc_1.dds"));

	TextureMap.emplace("LtcMat_2", std::make_shared<TTexture2D>("LtcMat_2", false, TextureDir + L"ltc_2.dds"));

	// Noise
	TextureMap.emplace("BlueNoiseTex", std::make_shared<TTexture2D>("BlueNoiseTex", false, TextureDir + L"BlueNoise.png"));
}

void TTextureRepository::Unload()
{
	TextureMap.clear();
}