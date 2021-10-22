#pragma once

#include "Actor.h"
#include "Component/CameraComponent.h"

class TCameraActor : public TActor
{
public:
	TCameraActor(const std::string& Name);
	~TCameraActor();

	TCameraComponent* GetCameraComponent();

private:
	TCameraComponent* CameraComponent = nullptr;
};
