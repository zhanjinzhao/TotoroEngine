#include "MeshComponent.h"
#include "Material/MaterialRepository.h"
#include "Mesh/MeshRepository.h"

void TMeshComponent::SetMeshName(std::string InMeshName)
{
	MeshName = InMeshName;
}

std::string TMeshComponent::GetMeshName() const
{
	return MeshName;
}

bool TMeshComponent::IsMeshValid() const
{
	return (MeshName != "");
}

bool TMeshComponent::GetLocalBoundingBox(TBoundingBox& OutBox)
{
	TMesh& Mesh = TMeshRepository::Get().MeshMap.at(MeshName);
	TBoundingBox Box = Mesh.GetBoundingBox();

	if (Box.bInit)
	{
		OutBox = Box;

		return true;
	}
	else
	{
		return false;
	}
}

bool TMeshComponent::GetWorldBoundingBox(TBoundingBox& OutBox)
{
	TBoundingBox LocalBox;
	
	if (GetLocalBoundingBox(LocalBox))
	{
		OutBox = LocalBox.Transform(WorldTransform);

		return true;
	}
	else
	{
		return false;
	}
}

void TMeshComponent::SetMaterialInstance(std::string MaterialInstanceName)
{
	MaterialInstance = TMaterialRepository::Get().GetMaterialInstance(MaterialInstanceName);

	assert(MaterialInstance);  //TODO
}