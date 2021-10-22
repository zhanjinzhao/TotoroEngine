#include <windows.h>
#include "Engine/Engine.h"
#include "Actor/StaticMeshActor.h"
#include "Actor/CameraActor.h"
#include "Actor/Light/DirectionalLightActor.h"
#include "Actor/Light/PointLightActor.h"
#include "Actor/Light/SpotLightActor.h"
#include "Actor/Light/AreaLightActor.h"
#include "Actor/SkyActor.h"
#include <random>


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
		CameraComponent->SetWorldLocation(TVector3(-1.8f, 13.0f, -22.0f));
		CameraComponent->Pitch(35.0f);
		CameraComponent->UpdateViewMatrix();

		// Add Floor
		{
			float Size = 1.5f;

			auto Floor = AddActor<TStaticMeshActor>("Floor");
			Floor->SetMesh("GridMesh");
			Floor->SetMaterialInstance("FloorMatInst");
			Floor->SetTextureScale(TVector2(4.0, 4.0f));
			TTransform Transform;
			Transform.Location = TVector3(0.0f, 0.0f, 0.0f);
			Transform.Scale = TVector3(Size, 1.0f, Size);
			Floor->SetActorTransform(Transform);
		}

		// Add spheres
		for (int i = -3; i < 3; i++)
		{
			for (int j = -3; j < 3; j++)
			{
				auto Sphere = AddActor<TStaticMeshActor>("Sphere");
				Sphere->SetMesh("SphereMesh");
				Sphere->SetMaterialInstance("RustedIronMatInst");
				TTransform SphereTransform;
				SphereTransform.Location = TVector3(i * 4.0f, 1.0f, j * 4.0f);
				Sphere->SetActorTransform(SphereTransform);
			}
		}


		// Add Columns
		for (int i = -2; i < 2; i++)
		{
			for (int j = -2; j < 2; j++)
			{
				auto Column = AddActor<TStaticMeshActor>("Column");
				Column->SetMesh("Column");
				Column->SetMaterialInstance("ColumnMatInst");
				TTransform Transform;
				Transform.Location = TVector3(i * 5.0f + 0.5f, -0.1f, j * 5.0f);
				Transform.Scale = TVector3(10.0f, 10.0f, 10.0f);
				Column->SetActorTransform(Transform);
			}
		}


		// Add PointLights
		std::random_device rd;
		std::default_random_engine eng(rd());
		std::uniform_real_distribution<float> distr(0.0f, 1.0f);

		for (int i = -3; i < 3; i++)
		{
			for (int j = -3; j < 3; j++)
			{
				float ColorX = distr(eng);
				float ColorY = distr(eng);
				float ColorZ = distr(eng);

				auto Light = AddActor<TPointLightActor>("PointLight");
				TTransform Transform;
				Transform.Location = TVector3(i * 4.0f + 1.0f, 1.0f, j * 4.0f);
				Light->SetActorTransform(Transform);
				Light->SetLightColor({ ColorX, ColorY, ColorZ });
				Light->SetLightIntensity(10.0f);
				Light->SetAttenuationRange(3.0f);
				Light->SetCastShadows(false);
				Light->SetDrawDebug(false);
			}
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
			RenderSettings.bUseTBDR = true;
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