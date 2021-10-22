#pragma once

#include "Material/Material.h"
#include <string>
#include <unordered_map>

class TMeshComponent;

struct TMeshBatch
{
	std::string MeshName;

	std::string InputLayoutName;

	TD3D12ConstantBufferRef ObjConstantBuffer = nullptr;

	TMeshComponent* MeshComponent = nullptr;

	// Flags
	bool bUseSDF = false;
};

struct TMeshCommand
{
	struct TMeshShaderParamters
	{
		std::unordered_map<std::string, TD3D12ConstantBufferRef> CBVParams;

		std::unordered_map<std::string, std::vector<TD3D12ShaderResourceView*>> SRVParams;
	};

public:
	void SetShaderParameter(std::string Param, TD3D12ConstantBufferRef CBV)
	{
		ShaderParameters.CBVParams.insert(std::make_pair(Param, CBV));
	}

	void SetShaderParameter(std::string Param, TD3D12ShaderResourceView* SRV)
	{
		std::vector<TD3D12ShaderResourceView*> SRVs;
		SRVs.push_back(SRV);

		ShaderParameters.SRVParams.insert(std::make_pair(Param, SRVs));
	}

	void SetShaderParameter(std::string Param, std::vector<TD3D12ShaderResourceView*> SRVs)
	{
		ShaderParameters.SRVParams.insert(std::make_pair(Param, SRVs));
	}

	void ApplyShaderParamters(TShader* Shader) const
	{
		if (Shader)
		{
			for (const auto& Pair : ShaderParameters.CBVParams)
			{
				Shader->SetParameter(Pair.first, Pair.second);
			}

			for (const auto& Pair : ShaderParameters.SRVParams)
			{
				Shader->SetParameter(Pair.first, Pair.second);
			}
		}
	}

public:
	std::string MeshName;

	TMaterialRenderState RenderState;

	TMeshShaderParamters ShaderParameters;
};

typedef std::vector<TMeshCommand> TMeshCommandList;