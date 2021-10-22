#pragma once

#include <unordered_map>
#include <string>
#include "Material.h"
#include "MaterialInstance.h"

class TMaterialRepository
{
public:
	static TMaterialRepository& Get();

	void Load();

	void Unload();

	TMaterialInstance* GetMaterialInstance (const std::string& MaterialInstanceName) const;

private:
	TMaterial* CreateMaterial(const std::string& MaterialName, const std::string& ShaderName);

	TMaterialInstance* CreateMaterialInstance(TMaterial* Material, const std::string& MaterialInstanceName);

	void CreateDefaultMaterialInstance(TMaterial* Material);

public:
	std::unordered_map<std::string /*MaterialName*/, std::unique_ptr<TMaterial>> MaterialMap;

	std::unordered_map<std::string /*MaterialInstanceName*/, std::unique_ptr<TMaterialInstance>> MaterialInstanceMap;
};
