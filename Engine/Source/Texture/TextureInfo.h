#pragma once

#include "D3D12/d3dx12.h"

enum class ETextureType
{
	TEXTURE_2D,
	TEXTURE_CUBE,
	TEXTURE_3D,
};

struct TTextureInfo
{
	ETextureType Type;
	D3D12_RESOURCE_DIMENSION Dimension;
	size_t Width;
	size_t Height;
	size_t Depth;
	size_t ArraySize;
	size_t MipCount;
	DXGI_FORMAT Format;

	D3D12_RESOURCE_STATES InitState = D3D12_RESOURCE_STATE_GENERIC_READ;

	DXGI_FORMAT SRVFormat = DXGI_FORMAT_UNKNOWN;
	DXGI_FORMAT RTVFormat = DXGI_FORMAT_UNKNOWN;
	DXGI_FORMAT DSVFormat = DXGI_FORMAT_UNKNOWN;
	DXGI_FORMAT UAVFormat = DXGI_FORMAT_UNKNOWN;
};

enum ETextureCreateFlags
{
	TexCreate_None        = 0,
	TexCreate_RTV         = 1 << 0,
	TexCreate_CubeRTV     = 1 << 1,
	TexCreate_DSV         = 1 << 2,
	TexCreate_CubeDSV     = 1 << 3,
	TexCreate_SRV         = 1 << 4,
	TexCreate_UAV         = 1 << 5,	
};