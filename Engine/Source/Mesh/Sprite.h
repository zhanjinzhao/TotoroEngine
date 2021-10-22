#pragma once

#include "Math/Math.h"
#include <string>

struct TSprite
{
	TSprite() {}
	TSprite(const std::string& InTextureName, const UIntPoint& InTextureSize, const RECT& InSourceRect, const RECT& InDestRect)
		:TextureName(InTextureName), TextureSize(InTextureSize), SourceRect(InSourceRect), DestRect(InDestRect)
	{}

	std::string TextureName;
	UIntPoint TextureSize;
	RECT SourceRect;
	RECT DestRect;
};
