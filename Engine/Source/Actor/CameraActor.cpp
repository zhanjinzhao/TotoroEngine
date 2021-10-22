#include "CameraActor.h"

TCameraActor::TCameraActor(const std::string& Name)
	:TActor(Name)
{
	//CameraComponent
	CameraComponent = AddComponent<TCameraComponent>();

	RootComponent = CameraComponent;
}

TCameraActor::~TCameraActor()
{

}

TCameraComponent* TCameraActor::GetCameraComponent()
{
	return CameraComponent;
}