#include "Actor.h"

TActor::TActor(const std::string& Name)
{
	SetName(Name);
}

void TActor::SetActorTransform(const TTransform& NewTransform)
{
	RootComponent->SetWorldTransform(NewTransform);
}

TTransform TActor::GetActorTransform() const
{
	return RootComponent->GetWorldTransform();
}

void TActor::SetActorLocation(const TVector3& NewLocation)
{
	RootComponent->SetWorldLocation(NewLocation);
}

TVector3 TActor::GetActorLocation() const
{
	return RootComponent->GetWorldLocation();
}

void TActor::SetActorRotation(const TRotator& NewRotation)
{
	RootComponent->SetWorldRotation(NewRotation);
}

TRotator TActor::GetActorRotation() const
{
	return RootComponent->GetWorldRotation();
}

void TActor::SetActorPrevTransform(const TTransform& PrevTransform)
{
	RootComponent->SetPrevWorldTransform(PrevTransform);
}

TTransform TActor::GetActorPrevTransform() const
{
	return RootComponent->GetPrevWorldTransform();
}