#pragma once

#include <memory>
#include "Component.h"
#include "Mesh/Mesh.h"
#include "Material/MaterialInstance.h"

class TMeshComponent : public TComponent
{
public:
	void SetMeshName(std::string InMeshName);

	std::string GetMeshName() const;

	bool IsMeshValid() const;

	bool GetLocalBoundingBox(TBoundingBox& OutBox);

	bool GetWorldBoundingBox(TBoundingBox& OutBox);

	void SetMaterialInstance(std::string MaterialInstanceName);

	TMaterialInstance* GetMaterialInstance() { return MaterialInstance; }

public:
	TMatrix TexTransform = TMatrix::Identity;

	// Flags
	bool bUseSDF = true;

private:
	std::string MeshName;

	TMaterialInstance* MaterialInstance;
};