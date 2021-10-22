#pragma once

#include <string>
#include <vector>
#include "Mesh/Vertex.h"
#include "D3D12/D3D12View.h"

struct TSpriteItem
{
	TD3D12ShaderResourceView* SpriteSRV = nullptr;

	UIntPoint TextureSize;

	RECT SourceRect;

	RECT DestRect;
};

struct TSpriteBatch
{
	std::vector<TSpriteItem> SpriteItems;

	TD3D12VertexBufferRef VertexBufferRef = nullptr;

	TD3D12IndexBufferRef IndexBufferRef = nullptr;
};
