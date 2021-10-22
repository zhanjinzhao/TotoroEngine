#pragma once

#include <unordered_map>
#include <string>
#include "Mesh.h"
#include "FbxLoader.h"

class TMeshRepository
{
public:
	TMeshRepository();

	static TMeshRepository& Get();

	void Load();

	void Unload();

public:
	std::unordered_map<std::string /*MeshName*/, TMesh> MeshMap;

private:
	std::unique_ptr<TFbxLoader> FbxLoader;
};