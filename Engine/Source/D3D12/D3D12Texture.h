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

	TD3D12ShaderResourceView* GetSRV(UINT Index = 0)
	{
		assert(SRVs.size() > Index);

		return SRVs[Index].get();
	}

	void AddSRV(std::unique_ptr<TD3D12ShaderResourceView>& SRV)
	{
		SRVs.emplace_back(std::move(SRV));
	}

	TD3D12RenderTargetView* GetRTV(UINT Index = 0)
	{
		assert(RTVs.size() > Index);

		return RTVs[Index].get();
	}

	void AddRTV(std::unique_ptr<TD3D12RenderTargetView>& RTV)
	{
		RTVs.emplace_back(std::move(RTV));
	}

	TD3D12DepthStencilView* GetDSV(UINT Index = 0)
	{
		assert(DSVs.size() > Index);

		return DSVs[Index].get();
	}

	void AddDSV(std::unique_ptr<TD3D12DepthStencilView>& DSV)
	{
		DSVs.emplace_back(std::move(DSV));
	}

	TD3D12UnorderedAccessView* GetUAV(UINT Index = 0)
	{
		assert(UAVs.size() > Index);

		return UAVs[Index].get();
	}

	void AddUAV(std::unique_ptr<TD3D12UnorderedAccessView>& UAV)
	{
		UAVs.emplace_back(std::move(UAV));
	}

public:
	TD3D12ResourceLocation ResourceLocation;

private:

	std::vector<std::unique_ptr<TD3D12ShaderResourceView>> SRVs;

	std::vector<std::unique_ptr<TD3D12RenderTargetView>> RTVs;

	std::vector<std::unique_ptr<TD3D12DepthStencilView>> DSVs;

	std::vector<std::unique_ptr<TD3D12UnorderedAccessView>> UAVs;

private:
	TVector4 RTVClearValue;
};

typedef std::shared_ptr<TD3D12Texture> TD3D12TextureRef;