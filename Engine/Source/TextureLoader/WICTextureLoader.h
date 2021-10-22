// Modified version of DirectXTK12's source file

//--------------------------------------------------------------------------------------
// File: WICTextureLoader.h
//
// Function for loading a WIC image and creating a Direct3D runtime texture for it
// (auto-generating mipmaps if possible)
//
// Note: Assumes application has already called CoInitializeEx
//
// Note these functions are useful for images created as simple 2D textures. For
// more complex resources, DDSTextureLoader is an excellent light-weight runtime loader.
// For a full-featured DDS file reader, writer, and texture processing pipeline see
// the 'Texconv' sample and the 'DirectXTex' library.
//
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// http://go.microsoft.com/fwlink/?LinkID=615561
//--------------------------------------------------------------------------------------

#pragma once

#include "Texture/TextureInfo.h"

namespace DirectX
{
	enum WIC_LOADER_FLAGS : uint32_t
	{
		WIC_LOADER_DEFAULT = 0,
		WIC_LOADER_FORCE_SRGB = 0x1,
		WIC_LOADER_IGNORE_SRGB = 0x2,
		WIC_LOADER_SRGB_DEFAULT = 0x4,
		WIC_LOADER_MIP_AUTOGEN = 0x8,
		WIC_LOADER_MIP_RESERVE = 0x10,
		WIC_LOADER_FIT_POW2 = 0x20,
		WIC_LOADER_MAKE_SQUARE = 0x40,
		WIC_LOADER_FORCE_RGBA32 = 0x80,
	};


	HRESULT __cdecl CreateWICTextureFromFile(
		_In_z_ const wchar_t* fileName,
		size_t maxsize,
		D3D12_RESOURCE_FLAGS resFlags,
		WIC_LOADER_FLAGS loadFlags,
		TTextureInfo& TextureInfo,
		D3D12_SUBRESOURCE_DATA& initData,
		std::vector<uint8_t>& decodedData);
}