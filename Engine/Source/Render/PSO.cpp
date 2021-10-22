#include "PSO.h"
#include "Shader/Shader.h"
#include "D3D12/D3D12RHI.h"

TGraphicsPSOManager::TGraphicsPSOManager(TD3D12RHI* InD3D12RHI, TInputLayoutManager* InInputLayoutManager)
	:D3D12RHI(InD3D12RHI), InputLayoutManager(InInputLayoutManager)
{

}

void TGraphicsPSOManager::TryCreatePSO(const TGraphicsPSODescriptor& Descriptor)
{
	if (PSOMap.find(Descriptor) == PSOMap.end())
	{
		CreatePSO(Descriptor);
	}
}

void TGraphicsPSOManager::CreatePSO(const TGraphicsPSODescriptor& Descriptor)
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC PsoDesc;
	ZeroMemory(&PsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

	// Inputlayout
	std::vector<D3D12_INPUT_ELEMENT_DESC> InputLayout;
	InputLayoutManager->GetInputLayout(Descriptor.InputLayoutName, InputLayout);
	PsoDesc.InputLayout = { InputLayout.data(), (UINT)InputLayout.size() };

	// Shader
	TShader* Shader = Descriptor.Shader;
	auto RootSignature = Shader->RootSignature;
	PsoDesc.pRootSignature = RootSignature.Get();
	PsoDesc.VS = CD3DX12_SHADER_BYTECODE(Shader->ShaderPass.at("VS")->GetBufferPointer(), Shader->ShaderPass.at("VS")->GetBufferSize());
	PsoDesc.PS = CD3DX12_SHADER_BYTECODE(Shader->ShaderPass.at("PS")->GetBufferPointer(), Shader->ShaderPass.at("PS")->GetBufferSize());

	PsoDesc.RasterizerState = Descriptor.RasterizerDesc;
	PsoDesc.BlendState = Descriptor.BlendDesc;
	PsoDesc.DepthStencilState = Descriptor.DepthStencilDesc;
	PsoDesc.SampleMask = UINT_MAX;
	PsoDesc.PrimitiveTopologyType = Descriptor.PrimitiveTopologyType;
	PsoDesc.NumRenderTargets = Descriptor.NumRenderTargets;
	for (int i = 0; i < 8; i++)
	{
		PsoDesc.RTVFormats[i] = Descriptor.RTVFormats[i];
	}
	PsoDesc.SampleDesc.Count = Descriptor._4xMsaaState ? 4 : 1;
	PsoDesc.SampleDesc.Quality = Descriptor._4xMsaaState ? (Descriptor._4xMsaaQuality - 1) : 0;
	PsoDesc.DSVFormat = Descriptor.DepthStencilFormat;

	// Create PSO
	ComPtr<ID3D12PipelineState> PSO;
	auto D3DDevice = D3D12RHI->GetDevice()->GetD3DDevice();
	ThrowIfFailed(D3DDevice->CreateGraphicsPipelineState(&PsoDesc, IID_PPV_ARGS(&PSO)));
	PSOMap.insert({ Descriptor, PSO });
}

ID3D12PipelineState* TGraphicsPSOManager::GetPSO(const TGraphicsPSODescriptor& Descriptor) const
{
	auto Iter = PSOMap.find(Descriptor);

	if (Iter == PSOMap.end())
	{
		assert(0); //TODO

		return nullptr;
	}
	else
	{
		return Iter->second.Get();
	}
}


TComputePSOManager::TComputePSOManager(TD3D12RHI* InD3D12RHI)
	:D3D12RHI(InD3D12RHI)
{

}

void TComputePSOManager::TryCreatePSO(const TComputePSODescriptor& Descriptor)
{
	if (PSOMap.find(Descriptor) == PSOMap.end())
	{
		CreatePSO(Descriptor);
	}
}

void TComputePSOManager::CreatePSO(const TComputePSODescriptor& Descriptor)
{
	D3D12_COMPUTE_PIPELINE_STATE_DESC PsoDesc = {};
	TShader* Shader = Descriptor.Shader;
	PsoDesc.pRootSignature = Shader->RootSignature.Get();
	PsoDesc.CS = CD3DX12_SHADER_BYTECODE(Shader->ShaderPass.at("CS")->GetBufferPointer(), Shader->ShaderPass.at("CS")->GetBufferSize());
	PsoDesc.Flags = Descriptor.Flags;

	ComPtr<ID3D12PipelineState> PSO;
	auto D3DDevice = D3D12RHI->GetDevice()->GetD3DDevice();
	ThrowIfFailed(D3DDevice->CreateComputePipelineState(&PsoDesc, IID_PPV_ARGS(&PSO)));
	PSOMap.insert({ Descriptor, PSO });
}

ID3D12PipelineState* TComputePSOManager::GetPSO(const TComputePSODescriptor& Descriptor) const
{
	auto Iter = PSOMap.find(Descriptor);

	if (Iter == PSOMap.end())
	{
		assert(0); //TODO

		return nullptr;
	}
	else
	{
		return Iter->second.Get();
	}
}

