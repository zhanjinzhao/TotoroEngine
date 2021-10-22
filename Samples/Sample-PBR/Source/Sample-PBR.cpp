#include <windows.h>
#include "Engine/Engine.h"
#include "Actor/StaticMeshActor.h"
#include "Actor/CameraActor.h"
#include "Actor/Light/DirectionalLightActor.h"
#include "Actor/Light/PointLightActor.h"
#include "Actor/Light/SpotLightActor.h"
#include "Actor/Light/AreaLightActor.h"
#include "Actor/SkyActor.h"


class TSampleWorld : public TWorld
{
public:
	TSampleWorld() {}

	~TSampleWorld() {}

	virtual void InitWorld(TEngine* InEngine) override
	{
		TWorld::InitWorld(InEngine);

		// Add Camera
		auto Camera = AddActor<TCameraActor>("Camera");
		CameraComponent = Camera->GetCameraComponent();
		CameraComponent->SetWorldLocation(TVector3(0.55f, 1.50f, -3.84f));
		CameraComponent->RotateY(-120.0f);
		CameraComponent->UpdateViewMatrix();

		// Add CyborgWeapon
		{
			auto CyborgWeapon = AddActor<TStaticMeshActor>("CyborgWeapon");
			CyborgWeapon->SetMesh("CyborgWeapon");
			CyborgWeapon->SetMaterialInstance("CyborgWeaponMatInst");
			TTransform Transform;
			Transform.Location = TVector3(-2.0f, 1.0f, -5.0f);
			Transform.Rotation = TRotator(0.0f, 0.0f, -45.0f);
			Transform.Scale = TVector3(300.0f, 300.0f, 300.0f);
			CyborgWeapon->SetActorTransform(Transform);
		}

		// Add AssaultRifle
		{
			auto Rifle = AddActor<TStaticMeshActor>("Rifle");
			Rifle->SetMesh("AssaultRifle");
			Rifle->SetMaterialInstance("AssaultRifleMatInst");
			TTransform Transform;
			Transform.Location = TVector3(-6.0f, 1.0f, -5.0f);
			Transform.Rotation = TRotator(0.0f, 0.0f, -45.0f);
			Transform.Scale = TVector3(60.0f, 60.0f, 60.0f);
			Rifle->SetActorTransform(Transform);
		}

		// Add Helmet
		{
			auto Helmet = AddActor<TStaticMeshActor>("Helmet");
			Helmet->SetMesh("Helmet");
			Helmet->SetMaterialInstance("HelmetMatInst");
			TTransform Transform;
			Transform.Location = TVector3(0.0f, 1.0f, 0.0f);
			Transform.Rotation = TRotator(0.0f, 0.0f, 90.0f);
			Transform.Scale = TVector3(1.0f, 1.0f, 1.0f);
			Helmet->SetActorTransform(Transform);
		}

		// Add sphere
		{
			auto Sphere = AddActor<TStaticMeshActor>("Sphere");
			Sphere->SetMesh("SphereMesh");
			Sphere->SetMaterialInstance("RustedIronMatInst");
			TTransform SphereTransform;
			SphereTransform.Location = TVector3(2.0f, 1.0f, -5.0f);
			Sphere->SetActorTransform(SphereTransform);
		}

		// Add Floor
		{
			float Size = 1.0f;

			auto Floor = AddActor<TStaticMeshActor>("Floor");
			Floor->SetMesh("GridMesh");
			Floor->SetMaterialInstance("FloorMatInst");
			Floor->SetTextureScale(TVector2(4.0, 4.0f));
			TTransform Transform;
			Transform.Location = TVector3(0.0f, 0.0f, 0.0f);
			Transform.Scale = TVector3(Size, 1.0f, Size);
			Floor->SetActorTransform(Transform);
		}


		// Add DirectionalLight
		{
			auto Light = AddActor<TDirectionalLightActor>("DirectionalLight");
			TTransform Transform;
			Transform.Location = TVector3(0.0f, 10.0f, 0.0f);
			Transform.Rotation = TRotator(0.0f, -90.0f, 0.0f);
			Light->SetActorTransform(Transform);
			Light->SetLightColor({ 1.0f, 1.0f, 1.0f });
			Light->SetLightIntensity(5.0f);
		}

		// Add Sky
		{
			auto Sky = AddActor<TSkyActor>("Sky");
			Sky->SetMaterialInstance("SkyMatInst");
			TTransform Transform;
			Transform.Scale = TVector3(5000.0f, 5000.0f, 5000.0f);
			Sky->SetActorTransform(Transform);
		}
	}
};




#if defined(DEBUG) || defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

_Use_decl_annotations_
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
	PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	try
	{
		{
			//_CrtSetBreakAlloc(550388);

			TSampleWorld* World = new TSampleWorld();

			TRenderSettings RenderSettings;

			TEngine Engine(hInstance);
			if (!Engine.Initialize(World, RenderSettings))
				return 0;

			Engine.Run();

			Engine.Destroy();
		}

		return 0;
	}
	catch (DxException& e)
	{
		MessageBox(nullptr, e.ToString().c_str(), L"HR Failed", MB_OK);
		return 0;
	}
}