#include <windows.h>
#include "Engine/Engine.h"
#include "Actor/StaticMeshActor.h"
#include "Actor/CameraActor.h"
#include "Actor/Light/DirectionalLightActor.h"
#include "Actor/Light/PointLightActor.h"
#include "Actor/Light/SpotLightActor.h"
#include "Actor/Light/AreaLightActor.h"
#include "Actor/SkyActor.h"

class TMovableActor : public TStaticMeshActor
{
public:
	TMovableActor(const std::string& Name)
		: TStaticMeshActor(Name)
	{}

	virtual void Tick(float DeltaSeconds) override
	{
		TVector3 Location = GetActorLocation();

		SetActorLocation(Location + TVector3(0.05f, 0.0f, 0.0f));
	}
};


class TSampleWorld : public TWorld
{
public:
	TSampleWorld() {}

	~TSampleWorld() {}

	virtual void InitWorld(TEngine* InEngine) override
	{
		TWorld::InitWorld(InEngine);

		CreateSSRScene();
	}

	void CreateSSRScene()
	{
		// Add Camera
		auto Camera = AddActor<TCameraActor>("Camera");
		CameraComponent = Camera->GetCameraComponent();
		CameraComponent->SetWorldLocation(TVector3(2.0f, 2.0f, -10.0f));
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
			//auto Sphere = AddActor<TMovableActor>("Sphere");
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
			RenderSettings.bEnableSSR = true;
			RenderSettings.bEnableTAA = true;
			RenderSettings.bDrawDebugText = true;

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