#pragma once

#include "Material.h"
#include "D3D12/D3D12Buffer.h"

class TD3D12RHI;

class TMaterialInstance
{
public:
	TMaterialInstance(TMaterial* Parent, const std::string& InName);

public:
	void SetTextureParamter(const std::string& Parameter, const std::string& TextureName);

	void CreateMaterialConstantBuffer(TD3D12RHI* D3D12RHI);

public:
	TMaterial* Material = nullptr;

	std::string Name;

	TMaterialParameters Parameters;

	TD3D12ConstantBufferRef MaterialConstantBuffer = nullptr;
};