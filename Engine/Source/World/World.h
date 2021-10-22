#pragma once

#include <vector>
#include <memory>
#include "Actor/Actor.h"
#include "Mesh/Color.h"
#include "Mesh/Primitive.h"
#include "Mesh/Sprite.h"
#include "Mesh/TextManager.h"
#include "Engine/GameTimer.h"
#include "Component/CameraComponent.h"

class TEngine;

class TWorld
{
public:
	TWorld();

	virtual ~TWorld() {}

	virtual void InitWorld(TEngine* InEngine);

	virtual void Update(const GameTimer& gt);

	virtual void EndFrame(const GameTimer& gt);

private:
	void SavePrevFrameData();

public:
	virtual void OnMouseDown(WPARAM btnState, int x, int y);

	virtual void OnMouseUp(WPARAM btnState, int x, int y);

	virtual void OnMouseMove(WPARAM btnState, int x, int y);

	virtual void OnMouseWheel(float WheelDistance);

	virtual void OnKeyboardInput(const GameTimer& gt);

public:
	template<typename T>
	T* AddActor(const std::string& Name)
	{
		auto NewActor = std::make_unique<T>(Name);
		T* Result = NewActor.get();
		Actors.push_back(std::move(NewActor));

		return Result;
	}

	std::vector<TActor*> GetActors()
	{
		std::vector<TActor*> Result;

		for (const auto& Actor : Actors)
		{
			Result.push_back(Actor.get());
		}

		return Result;
	}

	template<typename T>
	std::vector<T*> GetAllActorsOfClass()
	{
		std::vector<T*> Result;
		for (const auto& Actor : Actors)
		{
			T* ActorOfClass = dynamic_cast<T*>(Actor.get());
			if (ActorOfClass)
			{
				Result.push_back(ActorOfClass);
			}
		}

		return Result;
	}

	TCameraComponent* GetCameraComponent() { return CameraComponent; }

public:
	void DrawPoint(const TVector3& PointInWorld, const TColor& Color, int Size = 0);

	void DrawPoint(const TPoint& Point);

	const std::vector<TPoint>& GetPoints();

	void DrawLine(const TVector3& PointAInWorld, const TVector3& PointBInWorld, const TColor& Color);

	void DrawLine(const TLine& Line);

	const std::vector<TLine>& GetLines();

	void DrawBox3D(const TVector3& MinPointInWorld, const TVector3& MaxPointInWorld, const TColor& Color);

	void DrawTriangle(const TVector3& PointAInWorld, const TVector3& PointBInWorld, const TVector3& PointCInWorld, const TColor& Color);

	void DrawTriangle(const TTriangle& Triangle);

	const std::vector<TTriangle>& GetTriangles();

	void DrawSprite(const std::string& TextureName, const UIntPoint& TextureSize, const RECT& SourceRect, const RECT& DestRect);

	const std::vector<TSprite>& GetSprites();

	void DrawString(int ID, std::string Str, float Duration = 1.0f);

	void GetTexts(std::vector<TText>& OutTexts);

protected:
	std::vector<std::unique_ptr<TActor>> Actors;

	std::vector<TPoint> Points;

	std::vector<TLine> Lines;

	std::vector<TTriangle> Triangles;

	std::vector<TSprite> Sprites;

	TTextManager TextManager;

protected:
	TEngine* Engine = nullptr;

	HWND  MainWindowHandle = nullptr; // main window handle

	TCameraComponent* CameraComponent = nullptr;

	float MoveSpeed = 2.0f;

private:
	POINT LastMousePos;

	bool bKey_H_Pressed = false;

	bool bKey_J_Pressed = false;

	bool bKey_K_Pressed = false;

	bool bKey_L_Pressed = false;
};
