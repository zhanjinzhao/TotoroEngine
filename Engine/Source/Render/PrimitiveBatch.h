#pragma once

#include "Mesh/Vertex.h"

struct TPrimitiveBatch
{
	D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_LINELIST;

	TD3D12VertexBufferRef VertexBufferRef = nullptr;

	int CurrentVertexNum = 0;
};
