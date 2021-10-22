#pragma once

#include "D3D12Resource.h"
#include "D3D12View.h"
#include "Math/Math.h"

class TD3D12Texture
{
public:
	TD3D12Resource* GetResource() { return ResourceLocation.UnderlyingResource; }

	ID3D12Resource* GetD3DResource() { return ResourceLocation.UnderlyingResource->D3DResource.Get(); }

	void SetRTVClearValue(TVector4 ClearValue) { RTVClearValue = ClearValue; }

	TVector4 GetRTVClearValue() { return RTVClearValue; }

	float* GetRTVClearValuePtr() { return (float*)&RTVClearValue; }

public:
	TD3D12ResourceLocation ResourceLocation;

	std::vector<std::unique_ptr<TD3D12ShaderResourceView>> SRVs;

	std::vector<std::unique_ptr<TD3D12RenderTargetView>> RTVs;

	std::vector<std::unique_ptr<TD3D12DepthStencilView>> DSVs;

	std::vector<std::unique_ptr<TD3D12UnorderedAccessView>> UAVs;

private:
	TVector4 RTVClearValue;
};

typedef std::shared_ptr<TD3D12Texture> TD3D12TextureRef;