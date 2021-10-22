// Modified version of DirectXTK12's source file

//--------------------------------------------------------------------------------------
// File: SpriteFont.cpp
//
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// http://go.microsoft.com/fwlink/?LinkID=615561
//--------------------------------------------------------------------------------------

#include "File/BinaryReader.h"
#include "SpriteFont.h"


static const char spriteFontMagic[] = "DXTKfont";

TSpriteFont::TSpriteFont(ID3D12Device* device, _In_z_ wchar_t const* fileName)
{
	TBinaryReader reader(fileName);

	// Validate the header.
	for (char const* magic = spriteFontMagic; *magic; magic++)
	{
		if (reader.Read<uint8_t>() != *magic)
		{
			DebugTrace("ERROR: SpriteFont provided with an invalid .spritefont file\n");
			throw std::runtime_error("Not a MakeSpriteFont output binary");
		}
	}

	// Read the glyph data.
	auto glyphCount = reader.Read<uint32_t>();
	auto glyphData = reader.ReadArray<Glyph>(glyphCount);

	glyphs.assign(glyphData, glyphData + glyphCount);
	glyphsIndex.reserve(glyphs.size());

	for (auto& glyph : glyphs)
	{
		glyphsIndex.emplace_back(glyph.Character);
	}

	// Read font properties.
	lineSpacing = reader.Read<float>();

	SetDefaultCharacter(static_cast<wchar_t>(reader.Read<uint32_t>()));

	// Read the texture data.
	textureWidth = reader.Read<uint32_t>();
	textureHeight = reader.Read<uint32_t>();
	textureFormat = reader.Read<DXGI_FORMAT>();
	textureStride = reader.Read<uint32_t>();
	textureRows = reader.Read<uint32_t>();

	uint64_t dataSize = uint64_t(textureStride) * uint64_t(textureRows);
	if (dataSize > UINT32_MAX)
	{
		DebugTrace("ERROR: SpriteFont provided with an invalid .spritefont file\n");
		throw std::overflow_error("Invalid .spritefont file");
	}


	const uint8_t* Data = reader.ReadArray<uint8_t>(static_cast<size_t>(dataSize));
	textureData.resize(dataSize);
	memcpy_s(textureData.data(), dataSize, Data, dataSize);


	// Create FontTexture
	FontTexture = std::make_shared<TTexture2D>("FontTexture", false, L" ");
}

// Sets the missing-character fallback glyph.
void TSpriteFont::SetDefaultCharacter(wchar_t character)
{
	defaultGlyph = nullptr;

	if (character)
	{
		defaultGlyph = FindGlyph(character);
	}
}

// Looks up the requested glyph, falling back to the default character if it is not in the font.
TSpriteFont::Glyph const* TSpriteFont::FindGlyph(wchar_t character) const
{
	// Rather than use std::lower_bound (which includes a slow debug path when built for _DEBUG),
	// we implement a binary search inline to ensure sufficient Debug build performance to be useful
	// for text-heavy applications.

	size_t lower = 0;
	size_t higher = glyphs.size() - 1;
	size_t index = higher / 2;
	const size_t size = glyphs.size();

	while (index < size)
	{
		const auto curChar = glyphsIndex[index];
		if (curChar == character) { return &glyphs[index]; }
		if (curChar < character)
		{
			lower = index + 1;
		}
		else
		{
			higher = index - 1;
		}
		if (higher < lower) { break; }
		else if (higher - lower <= 4)
		{
			for (index = lower; index <= higher; index++)
			{
				if (glyphsIndex[index] == character)
				{
					return &glyphs[index];
				}
			}
		}
		index = lower + ((higher - lower) / 2);
	}

	if (defaultGlyph)
	{
		return defaultGlyph;
	}

	DebugTrace("ERROR: SpriteFont encountered a character not in the font (%u, %C), and no default glyph was provided\n", character, character);
	throw std::runtime_error("Character not in font");
}