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

		CreateDirectionalLightScene();

		//CreatePointLightScene();

		//CreateSpotLightScene();

		//CreateAreaLightScene();
	}


	void CreateDirectionalLightScene()
	{
		// Add Camera
		auto Camera = AddActor<TCameraActor>("Camera");
		CameraComponent = Camera->GetCameraComponent();
		CameraComponent->SetWorldLocation(TVector3(7.65f, 3.2f, 4.0f));
		CameraComponent->RotateY(-120.0f);
		CameraComponent->UpdateViewMatrix();
		//CameraComponent->Pitch(10.0f);
		//CameraComponent->UpdateViewMatrix();

		// Add Floor
		{
			float Size = 2.0f;

			auto Floor = AddActor<TStaticMeshActor>("Floor");
			Floor->SetMesh("GridMesh");
			Floor->SetMaterialInstance("FloorMatInst");
			Floor->SetTextureScale(TVector2(8.0, 8.0f));
			TTransform Transform;
			Transform.Location = TVector3(0.0f, 0.0f, 0.0f);
			Transform.Scale = TVector3(Size, 1.0f, Size);
			Floor->SetActorTransform(Transform);
			Floor->SetUseSDF(false);
		}

		// Add sphere
		{
			auto Sphere = AddActor<TStaticMeshActor>("Sphere");
			Sphere->SetMesh("SphereMesh");
			Sphere->SetMaterialInstance("RustedIronMatInst");
			TTransform SphereTransform;
			SphereTransform.Location = TVector3(4.0f, 2.0f, 0.0f);
			Sphere->SetActorTransform(SphereTransform);
		}

		// Add Column
		{
			auto Column = AddActor<TStaticMeshActor>("Column");
			Column->SetMesh("Column");
			Column->SetMaterialInstance("ColumnMatInst");
			TTransform Transform;
			Transform.Location = TVector3(0.0f, -0.1f, 0.0f);
			Transform.Scale = TVector3(10.0f, 10.0f, 10.0f);
			Column->SetActorTransform(Transform);
		}

		// Add DirectionalLight
		{
			auto Light = AddActor<TDirectionalLightActor>("DirectionalLight");
			TTransform Transform;
			Transform.Location = TVector3(0.0f, 10.0f, 0.0f);
			Transform.Rotation = TRotator(0.0f, 135.0f, 0.0f);
			Light->SetActorTransform(Transform);
			Light->SetLightColor({ 1.0f, 1.0f, 1.0f });
			Light->SetLightIntensity(2.0f);
			Light->SetDrawDebug(true);
		}
	}


	void CreatePointLightScene()
	{
		// Add Camera
		auto Camera = AddActor<TCameraActor>("Camera");
		CameraComponent = Camera->GetCameraComponent();
		CameraComponent->SetWorldLocation(TVector3(-3.0f, 11.0f, -13.0f));
		CameraComponent->Pitch(45.0f);
		CameraComponent->UpdateViewMatrix();

		// Add sphere
		{
			auto Sphere = AddActor<TStaticMeshActor>("Sphere");
			Sphere->SetMesh("SphereMesh");
			Sphere->SetMaterialInstance("RustedIronMatInst");
			TTransform SphereTransform;
			SphereTransform.Location = TVector3(4.0f, 1.0f, 0.0f);
			Sphere->SetActorTransform(SphereTransform);
		}

		// Add sphere2
		{
			auto Sphere2 = AddActor<TStaticMeshActor>("Sphere2");
			Sphere2->SetMesh("SphereMesh");
			Sphere2->SetMaterialInstance("RustedIronMatInst");
			TTransform SphereTransform;
			SphereTransform.Location = TVector3(4.0f, 1.0f, -4.0f);
			Sphere2->SetActorTransform(SphereTransform);
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
			Floor->SetUseSDF(false);
		}

		// Add Column
		{
			auto Column = AddActor<TStaticMeshActor>("Column");
			Column->SetMesh("Column");
			Column->SetMaterialInstance("ColumnMatInst");
			TTransform Transform;
			Transform.Location = TVector3(0.0f, -0.1f, 0.0f);
			Transform.Scale = TVector3(10.0f, 10.0f, 10.0f);
			Column->SetActorTransform(Transform);
		}

		// Add Column2
		{
			auto Column2 = AddActor<TStaticMeshActor>("Column2");
			Column2->SetMesh("Column");
			Column2->SetMaterialInstance("ColumnMatInst");
			TTransform Transform;
			Transform.Location = TVector3(0.0f, -0.1f, -4.0f);
			Transform.Scale = TVector3(10.0f, 10.0f, 10.0f);
			Column2->SetActorTransform(Transform);
		}

		// Add PointLight
		{
			auto Light = AddActor<TPointLightActor>("PointLight");
			TTransform Transform;
			Transform.Location = TVector3(2.0f, 3.0f, -2.0f);
			Light->SetActorTransform(Transform);
			Light->SetLightColor({ 1.0f, 1.0f, 1.0f });
			Light->SetLightIntensity(20.0f);
			Light->SetAttenuationRange(6.0f);
			Light->SetDrawDebug(true);
		}
	}

	void CreateSpotLightScene()
	{
		// Add Camera
		auto Camera = AddActor<TCameraActor>("Camera");
		CameraComponent = Camera->GetCameraComponent();
		CameraComponent->SetWorldLocation(TVector3(-3.0f, 11.0f, -13.0f));
		CameraComponent->Pitch(45.0f);
		CameraComponent->UpdateViewMatrix();

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
			Floor->SetUseSDF(false);
		}

		// Add sphere
		{
			auto Sphere = AddActor<TStaticMeshActor>("Sphere");
			Sphere->SetMesh("SphereMesh");
			Sphere->SetMaterialInstance("RustedIronMatInst");
			TTransform SphereTransform;
			SphereTransform.Location = TVector3(4.0f, 2.0f, 0.0f);
			Sphere->SetActorTransform(SphereTransform);
		}

		// Add Column
		{
			auto Column = AddActor<TStaticMeshActor>("Column");
			Column->SetMesh("Column");
			Column->SetMaterialInstance("ColumnMatInst");
			TTransform Transform;
			Transform.Location = TVector3(0.0f, -0.1f, 0.0f);
			Transform.Scale = TVector3(10.0f, 10.0f, 10.0f);
			Column->SetActorTransform(Transform);
		}

		// Add SpotLight
		{
			auto Light = AddActor<TSpotLightActor>("SpotLight");
			TTransform Transform;
			Transform.Location = TVector3(0.0f, 5.0f, -3.0f);
			Transform.Rotation = TRotator(0.0f, 150.0f, 0.0f);
			Light->SetActorTransform(Transform);
			Light->SetLightColor({ 1.0f, 1.0f, 1.0f });
			Light->SetLightIntensity(100.0f);
			Light->SetAttenuationRange(10.0f);
			Light->SetInnerConeAngle(10.0f);
			Light->SetOuterConeAngle(30.0f);
			Light->SetDrawDebug(true);
		}
	}

	void CreateAreaLightScene()
	{
		// Add Camera
		auto Camera = AddActor<TCameraActor>("Camera");
		CameraComponent = Camera->GetCameraComponent();
		CameraComponent->SetWorldLocation(TVector3(0.0f, 0.0f, -4.0f));
		CameraComponent->UpdateViewMatrix();

		// Add Floor
		{
			float Size = 1.0f;

			auto Floor = AddActor<TStaticMeshActor>("Floor");
			Floor->SetMesh("GridMesh");
			//Floor->SetMaterialInstance("FloorMatInst");
			Floor->SetMaterialInstance("GrayGraniteMatInst");
			Floor->SetTextureScale(TVector2(4.0, 4.0f));
			TTransform Transform;
			Transform.Location = TVector3(0.0f, 0.0f, 0.0f);
			Transform.Scale = TVector3(Size, 1.0f, Size);
			Floor->SetActorTransform(Transform);
			Floor->SetUseSDF(false);
		}

		// Add sphere
		{
			auto Sphere = AddActor<TStaticMeshActor>("Sphere");
			Sphere->SetMesh("SphereMesh");
			Sphere->SetMaterialInstance("RustedIronMatInst");
			TTransform SphereTransform;
			SphereTransform.Location = TVector3(4.0f, 0.5f, 0.0f);
			Sphere->SetActorTransform(SphereTransform);
		}

		// Add Column
		{
			auto Column = AddActor<TStaticMeshActor>("Column");
			Column->SetMesh("Column");
			Column->SetMaterialInstance("ColumnMatInst");
			TTransform Transform;
			Transform.Location = TVector3(0.0f, -0.2f, 0.0f);
			Transform.Scale = TVector3(10.0f, 10.0f, 10.0f);
			Column->SetActorTransform(Transform);
		}

		// Add AreaLight
		{
			auto Light = AddActor<TAreaLightActor>("AreaLight");
			TTransform Transform;
			Transform.Location = TVector3(2.0f, 0.5f, 1.0f);
			Transform.Scale = TVector3(2.0f, 1.0f, 0.0f);
			Light->SetActorTransform(Transform);
			Light->SetLightColor({ 0xFF/255.0f, 0xFF/255.0f, 0x00/255.0f });
			Light->SetLightIntensity(1.0f);
			Light->SetDrawMesh(true);
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
			RenderSettings.ShadowMapImpl = EShadowMapImpl::PCSS;

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