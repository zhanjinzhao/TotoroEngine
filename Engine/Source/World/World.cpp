#include "World.h"
#include "Engine/Engine.h"
#include <algorithm>


TWorld::TWorld()
{
}

void TWorld::InitWorld(TEngine* InEngine)
{
	Engine = InEngine;

	MainWindowHandle = Engine->GetMainWnd();
}

std::string GetToggleStateStr(bool bOn)
{
	if (bOn)
	{
		return "ON";
	}
	else
	{
		return "OFF";
	}
}

void TWorld::Update(const GameTimer& gt)
{
	OnKeyboardInput(gt);

	// Tick actors
	for (auto& Actor : Actors)
	{
		Actor->Tick(gt.DeltaTime());
	}

	// Calculate FPS and draw text
	{
		static float FPS = 0.0f;
		static float MSPF = 0.0f;

		static int FrameCnt = 0;
		static float TimeElapsed = 0.0f;
		FrameCnt++;

		// Compute averages over one second period.
		if ((gt.TotalTime() - TimeElapsed) >= 1.0f)
		{
			FPS = (float)FrameCnt;
			MSPF = 1000.0f / FPS;

			// Reset for next average.
			FrameCnt = 0;
			TimeElapsed += 1.0f;
		}

		DrawString(1, "FPS: " + std::to_string(FPS), 0.1f);
		DrawString(2, "MSPF: " + std::to_string(MSPF), 0.1f);
	}

	// Print hints
	const TRenderSettings& RenderSettings = Engine->GetRender()->GetRenderSettings();

	DrawString(3, "Hint: ", 0.1f);
	DrawString(4, std::string("      Press H to toggle TAA [") + GetToggleStateStr(RenderSettings.bEnableTAA) +"]", 0.1f);
	DrawString(5, std::string("      Press J to toggle SSR [") + GetToggleStateStr(RenderSettings.bEnableSSR) + "]", 0.1f);
	DrawString(6, std::string("      Press K to toggle SSAO [") + GetToggleStateStr(RenderSettings.bEnableSSAO) + "]", 0.1f);
	DrawString(7, std::string("      Press L to toggle SDF [") + GetToggleStateStr(RenderSettings.bDebugSDFScene) + "]", 0.1f);

	// Print camera message
	TVector3 CameraLocation = CameraComponent->GetWorldLocation();
	DrawString(11, "CameraLocation(" + std::to_string(CameraLocation.x) + ", " + std::to_string(CameraLocation.y) + ", " + std::to_string(CameraLocation.z) + ")");

	// TODO
	//TRotator CameraRotation = CameraComponent->GetWorldRotation();
	//DrawString(12, "CameraRotation(" + std::to_string(CameraRotation.Roll) + ", " + std::to_string(CameraRotation.Pitch) + ", " + std::to_string(CameraRotation.Yaw) + ")");
}

void TWorld::EndFrame(const GameTimer& gt)
{
	SavePrevFrameData();

	float DeltaTime = gt.DeltaTime();

	// Clear Primitives
	Points.clear();

	Lines.clear();

	Triangles.clear();

	// Clear Texts
	TextManager.UpdateTexts(DeltaTime);
}

void TWorld::SavePrevFrameData()
{
	for (auto& Actor : Actors)
	{
		if (Actor->GetRootComponent())
		{
			Actor->SetActorPrevTransform(Actor->GetActorTransform());
		}
	}

	TMatrix View = CameraComponent->GetView();
	TMatrix Proj = CameraComponent->GetProj();
	TMatrix ViewProj = View * Proj;
	CameraComponent->SetPrevViewProj(ViewProj);
}

void TWorld::OnMouseDown(WPARAM btnState, int x, int y)
{
	if ((btnState & MK_LBUTTON) != 0)
	{
		LastMousePos.x = x;
		LastMousePos.y = y;

		SetCapture(MainWindowHandle);
	}
}

void TWorld::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void TWorld::OnMouseMove(WPARAM btnState, int x, int y)
{
	if ((btnState & MK_LBUTTON) != 0)
	{
		// Make each pixel correspond to a quarter of a degree.
		float dx = 0.25f * static_cast<float>(x - LastMousePos.x);
		float dy = 0.25f * static_cast<float>(y - LastMousePos.y);

		CameraComponent->Pitch(dy);  
		CameraComponent->RotateY(dx);
	}

	LastMousePos.x = x;
	LastMousePos.y = y;
}

void TWorld::OnMouseWheel(float WheelDistance)
{
	// Positive value means rotated forward, negative value means rotated backward(toward the user)
	MoveSpeed += (WheelDistance / WHEEL_DELTA); 

	MoveSpeed = std::clamp(MoveSpeed, 1.0f, 10.0f);
}

void TWorld::OnKeyboardInput(const GameTimer& gt)
{
	const float dt = gt.DeltaTime();

	/*-------------Camera-------------*/
	if (GetAsyncKeyState('W') & 0x8000)
		CameraComponent->MoveForward(MoveSpeed * dt);

	if (GetAsyncKeyState('S') & 0x8000)
		CameraComponent->MoveForward(-MoveSpeed * dt);

	if (GetAsyncKeyState('A') & 0x8000)
		CameraComponent->MoveRight(-MoveSpeed * dt);

	if (GetAsyncKeyState('D') & 0x8000)
		CameraComponent->MoveRight(MoveSpeed * dt);

	if (GetAsyncKeyState('Q') & 0x8000)
		CameraComponent->MoveUp(-MoveSpeed * dt);

	if (GetAsyncKeyState('E') & 0x8000)
		CameraComponent->MoveUp(MoveSpeed * dt);

	CameraComponent->UpdateViewMatrix();


	/*-------------Render settings---------------*/
	if (GetAsyncKeyState('H') & 0x8000)
	{
		if (!bKey_H_Pressed)
		{
			bKey_H_Pressed = true;
			Engine->GetRender()->ToggleTAA();
		}
	}
	else
	{
		bKey_H_Pressed = false;
	}

	if (GetAsyncKeyState('J') & 0x8000)
	{
		if (!bKey_J_Pressed)
		{
			bKey_J_Pressed = true;
			Engine->GetRender()->ToggleSSR();
		}
	}
	else
	{
		bKey_J_Pressed = false;
	}

	if (GetAsyncKeyState('K') & 0x8000)
	{
		if (!bKey_K_Pressed)
		{
			bKey_K_Pressed = true;
			Engine->GetRender()->ToggleSSAO();
		}
	}
	else
	{
		bKey_K_Pressed = false;
	}

	if (GetAsyncKeyState('L') & 0x8000)
	{
		if (!bKey_L_Pressed)
		{
			bKey_L_Pressed = true;
			Engine->GetRender()->ToggleDebugSDF();
		}
	}
	else
	{
		bKey_L_Pressed = false;
	}
}

void TWorld::DrawPoint(const TVector3& PointInWorld, const TColor& Color, int Size)
{
	Points.emplace_back(PointInWorld, Color);

	if (Size != 0)
	{
		float Offset = 0.01f * Size;

		Points.emplace_back(PointInWorld + TVector3(Offset, 0.0f, 0.0f), Color);
		Points.emplace_back(PointInWorld + TVector3(0.0f, Offset, 0.0f), Color);
		Points.emplace_back(PointInWorld + TVector3(0.0f, 0.0f, Offset), Color);
		Points.emplace_back(PointInWorld + TVector3(-Offset, 0.0f, 0.0f), Color);
		Points.emplace_back(PointInWorld + TVector3(0.0f, -Offset, 0.0f), Color);
		Points.emplace_back(PointInWorld + TVector3(0.0f, 0.0f, -Offset), Color);
	}
}

void TWorld::DrawPoint(const TPoint& Point)
{
	Points.push_back(Point);
}

const std::vector<TPoint>& TWorld::GetPoints()
{
	return Points;
}

void TWorld::DrawLine(const TVector3& PointAInWorld, const TVector3& PointBInWorld, const TColor& Color)
{
	Lines.emplace_back(PointAInWorld, PointBInWorld, Color);
}

void TWorld::DrawLine(const TLine& Line)
{
	Lines.push_back(Line);
}

const std::vector<TLine>& TWorld::GetLines()
{
	return Lines;
}

void TWorld::DrawBox3D(const TVector3& MinPointInWorld, const TVector3& MaxPointInWorld, const TColor& Color)
{
	TVector3 Min = MinPointInWorld;
	TVector3 Max = MaxPointInWorld;

	DrawLine(TVector3(Min.x, Min.y, Min.z), TVector3(Min.x, Min.y, Max.z), Color);
	DrawLine(TVector3(Min.x, Max.y, Min.z), TVector3(Min.x, Max.y, Max.z), Color);
	DrawLine(TVector3(Max.x, Min.y, Min.z), TVector3(Max.x, Min.y, Max.z), Color);
	DrawLine(TVector3(Max.x, Max.y, Min.z), TVector3(Max.x, Max.y, Max.z), Color);

	DrawLine(TVector3(Min.x, Min.y, Min.z), TVector3(Max.x, Min.y, Min.z), Color);
	DrawLine(TVector3(Min.x, Max.y, Min.z), TVector3(Max.x, Max.y, Min.z), Color);
	DrawLine(TVector3(Min.x, Min.y, Max.z), TVector3(Max.x, Min.y, Max.z), Color);
	DrawLine(TVector3(Min.x, Max.y, Max.z), TVector3(Max.x, Max.y, Max.z), Color);

	DrawLine(TVector3(Min.x, Min.y, Min.z), TVector3(Min.x, Max.y, Min.z), Color);
	DrawLine(TVector3(Max.x, Min.y, Min.z), TVector3(Max.x, Max.y, Min.z), Color);
	DrawLine(TVector3(Min.x, Min.y, Max.z), TVector3(Min.x, Max.y, Max.z), Color);
	DrawLine(TVector3(Max.x, Min.y, Max.z), TVector3(Max.x, Max.y, Max.z), Color);
}

void TWorld::DrawTriangle(const TVector3& PointAInWorld, const TVector3& PointBInWorld, const TVector3& PointCInWorld, const TColor& Color)
{
	Triangles.emplace_back(PointAInWorld, PointBInWorld, PointCInWorld, Color);
}

void TWorld::DrawTriangle(const TTriangle& Triangle)
{
	Triangles.push_back(Triangle);
}

const std::vector<TTriangle>& TWorld::GetTriangles()
{
	return Triangles;
}

void TWorld::DrawSprite(const std::string& TextureName, const UIntPoint& TextureSize, const RECT& SourceRect, const RECT& DestRect)
{
	Sprites.emplace_back(TSprite(TextureName, TextureSize, SourceRect, DestRect));
}

const std::vector<TSprite>& TWorld::GetSprites()
{
	return Sprites;
}

void TWorld::DrawString(int ID, std::string Str, float Duration)
{	
	TextManager.AddText(ID, Str, Duration);
}

void TWorld::GetTexts(std::vector<TText>& OutTexts)
{
	TextManager.GetTexts(OutTexts);
}

