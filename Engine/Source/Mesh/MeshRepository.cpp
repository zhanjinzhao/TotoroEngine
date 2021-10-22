#include "MeshRepository.h"

TMeshRepository::TMeshRepository()
{
	FbxLoader = std::make_unique<TFbxLoader>();
	FbxLoader->Init();
}

TMeshRepository& TMeshRepository::Get()
{
	static TMeshRepository Instance;
	return Instance;
}

void TMeshRepository::Load()
{
	TMesh BoxMesh;
	BoxMesh.CreateBox(1.0f, 1.0f, 1.0f, 3);
	BoxMesh.MeshName = "BoxMesh";
	BoxMesh.GenerateBoundingBox();
	MeshMap.emplace("BoxMesh", std::move(BoxMesh));

	TMesh SphereMesh;
	SphereMesh.CreateSphere(0.5f, 20, 20);
	SphereMesh.MeshName = "SphereMesh";
	SphereMesh.GenerateBoundingBox();
	MeshMap.emplace("SphereMesh", std::move(SphereMesh));

	TMesh CylinderMesh;
	CylinderMesh.CreateCylinder(0.5f, 0.3f, 3.0f, 20, 20);
	CylinderMesh.MeshName = "CylinderMesh";
	CylinderMesh.GenerateBoundingBox();
	MeshMap.emplace("CylinderMesh", std::move(CylinderMesh));

	TMesh GridMesh;
	GridMesh.CreateGrid(20.0f, 30.0f, 60, 40);
	GridMesh.MeshName = "GridMesh";
	GridMesh.GenerateBoundingBox();
	MeshMap.emplace("GridMesh", std::move(GridMesh));

	TMesh QuadMesh;
	QuadMesh.CreateQuad(-0.5f, 0.5f, 1.0f, 1.0f, 0.0f);
	QuadMesh.MeshName = "QuadMesh";
	QuadMesh.GenerateBoundingBox();
	MeshMap.emplace("QuadMesh", std::move(QuadMesh));

	TMesh ScreenQuadMesh;
	ScreenQuadMesh.CreateQuad(-1.0f, 1.0f, 2.0f, 2.0f, 0.0f);
	ScreenQuadMesh.MeshName = "ScreenQuadMesh";
	ScreenQuadMesh.GenerateBoundingBox();
	MeshMap.emplace("ScreenQuadMesh", std::move(ScreenQuadMesh));

	TMesh AssaultRifleMesh;
	AssaultRifleMesh.MeshName = "AssaultRifle";
	FbxLoader->LoadFBXMesh(L"LOW_WEPON.fbx", AssaultRifleMesh);
	AssaultRifleMesh.GenerateBoundingBox();
	MeshMap.emplace("AssaultRifle", std::move(AssaultRifleMesh));

	TMesh CyborgWeaponMesh;
	CyborgWeaponMesh.MeshName = "CyborgWeapon";
	FbxLoader->LoadFBXMesh(L"Cyborg_Weapon.fbx", CyborgWeaponMesh);
	CyborgWeaponMesh.GenerateBoundingBox();
	MeshMap.emplace("CyborgWeapon", std::move(CyborgWeaponMesh));

	TMesh HelmetMesh;
	HelmetMesh.MeshName = "Helmet";
	FbxLoader->LoadFBXMesh(L"helmet_low.fbx", HelmetMesh);
	HelmetMesh.GenerateBoundingBox();
	MeshMap.emplace("Helmet", std::move(HelmetMesh));

	TMesh ColumnMesh;
	ColumnMesh.MeshName = "Column";
	FbxLoader->LoadFBXMesh(L"column.fbx", ColumnMesh);
	ColumnMesh.GenerateBoundingBox();
	MeshMap.emplace("Column", std::move(ColumnMesh));
}

void TMeshRepository::Unload()
{
	MeshMap.clear();
}