#pragma once

#include <vector>
#include <memory>
#include <string>
#include "Component/Component.h"
#include "Math/Transform.h"


class TActor
{
public:
	TActor(const std::string& Name);

	virtual ~TActor() {}

public:
	virtual void Tick(float DeltaSeconds) {}

public:
	template<typename T>
	T* AddComponent()
	{
		auto NewComponent = std::make_unique<T>();
		T* Result = NewComponent.get();
		Components.push_back(std::move(NewComponent));

		return Result;
	}

	std::vector<TComponent*> GetComponets()
	{
		std::vector<TComponent*> Result;

		for (const auto& Component : Components)
		{
			Result.push_back(Component.get());
		}

		return Result;
	}

	template<typename T>
	std::vector<T*> GetComponentsOfClass()
	{
		std::vector<T*> Result;
		for (const auto& Component : Components)
		{
			T* ComponentOfClass = dynamic_cast<T*>(Component.get());
			if (ComponentOfClass)
			{
				Result.push_back(ComponentOfClass);
			}
		}

		return Result;
	}

	TComponent* GetRootComponent() const
	{
		return RootComponent;
	}

	virtual void SetActorTransform(const TTransform& NewTransform);

	TTransform GetActorTransform() const;

	void SetActorLocation(const TVector3& NewLocation);

	TVector3 GetActorLocation() const;

	void SetActorRotation(const TRotator& NewRotation);

	TRotator GetActorRotation() const;

	void SetActorPrevTransform(const TTransform& PrevTransform);

	TTransform GetActorPrevTransform() const;

	void SetName(std::string Name) { ActorName = Name; }

	std::string GetName() const { return ActorName; }

protected:
	std::string ActorName;

	std::vector<std::unique_ptr<TComponent>> Components;

	TComponent* RootComponent = nullptr;
};
