#pragma once

#include <unordered_map>
#include <string>
#include <memory>
#include "Texture.h"

class TTextureRepository
{
public:
	static TTextureRepository& Get();

	void Load();

	void Unload();

public:
	std::unordered_map<std::string /*TextureName*/, std::shared_ptr<TTexture>> TextureMap;
};
