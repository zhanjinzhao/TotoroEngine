#include "D3D12Texture.h"
#include "D3D12RHI.h"


TD3D12TextureRef TD3D12RHI::CreateTexture(const TTextureInfo& TextureInfo, uint32_t CreateFlags, TVector4 RTVClearValue)
{
	TD3D12TextureRef TextureRef = CreateTextureResource(TextureInfo, CreateFlags, RTVClearValue);
	
	CreateTextureViews(TextureRef, TextureInfo, CreateFlags);

	return TextureRef;
}

TD3D12TextureRef TD3D12RHI::CreateTexture(Microsoft::WRL::ComPtr<ID3D12Resource> D3DResource, TTextureInfo& TextureInfo, uint32_t CreateFlags)
{
	TD3D12TextureRef TextureRef = std::make_shared<TD3D12Texture>();

	TD3D12Resource* NewResource = new TD3D12Resource(D3DResource, TextureInfo.InitState);
	TextureRef->ResourceLocation.UnderlyingResource = NewResource;
	TextureRef->ResourceLocation.SetType(TD3D12ResourceLocation::EResourceLocationType::StandAlone);

	CreateTextureViews(TextureRef, TextureInfo, CreateFlags);

	return TextureRef;
}

TD3D12TextureRef TD3D12RHI::CreateTextureResource(const TTextureInfo& TextureInfo, uint32_t CreateFlags, TVector4 RTVClearValue)
{
	TD3D12TextureRef TextureRef = std::make_shared<TD3D12Texture>();

	//Create default resource
	D3D12_RESOURCE_STATES ResourceState = D3D12_RESOURCE_STATE_COMMON;

	D3D12_RESOURCE_DESC TexDesc;
	ZeroMemory(&TexDesc, sizeof(D3D12_RESOURCE_DESC));
	TexDesc.Dimension = TextureInfo.Dimension;
	TexDesc.Alignment = 0;
	TexDesc.Width = TextureInfo.Width;
	TexDesc.Height = (uint32_t)TextureInfo.Height;
	TexDesc.DepthOrArraySize = (TextureInfo.Depth > 1) ? (uint16_t)TextureInfo.Depth : (uint16_t)TextureInfo.ArraySize;
	TexDesc.MipLevels = (uint16_t)TextureInfo.MipCount;
	TexDesc.Format = TextureInfo.Format;
	TexDesc.SampleDesc.Count = 1;
	TexDesc.SampleDesc.Quality = 0;
	TexDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

	bool bCreateRTV = CreateFlags & (TexCreate_RTV | TexCreate_CubeRTV);
	bool bCreateDSV = CreateFlags & (TexCreate_DSV | TexCreate_CubeDSV);
	bool bCreateUAV = CreateFlags & TexCreate_UAV;

	if (bCreateRTV)
	{
		TexDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET; // | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	}
	else if (bCreateDSV)
	{
		TexDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	}
	else if (bCreateUAV)
	{
		TexDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	}
	else
	{
		TexDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	}


	bool bReadOnlyTexture = !(bCreateRTV | bCreateDSV | bCreateUAV);
	if (bReadOnlyTexture)
	{
		auto TextureResourceAllocator = GetDevice()->GetTextureResourceAllocator();
		TextureResourceAllocator->AllocTextureResource(ResourceState, TexDesc, TextureRef->ResourceLocation);

		auto TextureResource = TextureRef->GetD3DResource();
		assert(TextureResource);
	}
	else
	{
		Microsoft::WRL::ComPtr<ID3D12Resource> Resource;

		CD3DX12_CLEAR_VALUE ClearValue = {};
		CD3DX12_CLEAR_VALUE* ClearValuePtr = nullptr;

		// Set clear value for RTV and DSV
		if (bCreateRTV)
		{
			ClearValue = CD3DX12_CLEAR_VALUE(TexDesc.Format, (float*)&RTVClearValue);
			ClearValuePtr = &ClearValue;

			TextureRef->SetRTVClearValue(RTVClearValue);
		}
		else if (bCreateDSV)
		{
			FLOAT Depth = 1.0f;
			UINT8 Stencil = 0;
			ClearValue = CD3DX12_CLEAR_VALUE(DXGI_FORMAT_D24_UNORM_S8_UINT, Depth, Stencil);
			ClearValuePtr = &ClearValue;
		}

		GetDevice()->GetD3DDevice()->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&TexDesc,
			TextureInfo.InitState,
			ClearValuePtr,
			IID_PPV_ARGS(&Resource));

		TD3D12Resource* NewResource = new TD3D12Resource(Resource, TextureInfo.InitState);
		TextureRef->ResourceLocation.UnderlyingResource = NewResource;
		TextureRef->ResourceLocation.SetType(TD3D12ResourceLocation::EResourceLocationType::StandAlone);
	}

	return TextureRef;
}

void  TD3D12RHI::CreateTextureViews(TD3D12TextureRef TextureRef, const TTextureInfo& TextureInfo, uint32_t CreateFlags)
{
	auto TextureResource = TextureRef->GetD3DResource();

	// Create SRV
	if (CreateFlags & TexCreate_SRV)
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC SrvDesc = {};
		SrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		if (TextureInfo.SRVFormat == DXGI_FORMAT_UNKNOWN)
		{
			SrvDesc.Format = TextureInfo.Format;
		}
		else
		{
			SrvDesc.Format = TextureInfo.SRVFormat;
		}

		if (TextureInfo.Type == ETextureType::TEXTURE_2D)
		{
			SrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			SrvDesc.Texture2D.MostDetailedMip = 0;
			SrvDesc.Texture2D.MipLevels = (uint16_t)TextureInfo.MipCount;
		}
		else if(TextureInfo.Type == ETextureType::TEXTURE_CUBE)
		{
			SrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
			SrvDesc.TextureCube.MostDetailedMip = 0;
			SrvDesc.TextureCube.MipLevels = (uint16_t)TextureInfo.MipCount;
		}
		else
		{
			SrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
			SrvDesc.Texture3D.MostDetailedMip = 0;
			SrvDesc.Texture3D.MipLevels = (uint16_t)TextureInfo.MipCount;
		}

		TextureRef->SRVs.push_back(std::make_unique<TD3D12ShaderResourceView>(GetDevice(), SrvDesc, TextureResource));
	}

	// Create RTV
	if (CreateFlags & TexCreate_RTV)
	{
		D3D12_RENDER_TARGET_VIEW_DESC RtvDesc = {};
		RtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		RtvDesc.Texture2D.MipSlice = 0;
		RtvDesc.Texture2D.PlaneSlice = 0;		

		if (TextureInfo.RTVFormat == DXGI_FORMAT_UNKNOWN)
		{
			RtvDesc.Format = TextureInfo.Format;
		}
		else
		{
			RtvDesc.Format = TextureInfo.RTVFormat;
		}

		TextureRef->RTVs.push_back(std::make_unique<TD3D12RenderTargetView>(GetDevice(), RtvDesc, TextureResource));
	}
	else if (CreateFlags & TexCreate_CubeRTV)
	{
		for (size_t i = 0; i < 6; i++)
		{
			D3D12_RENDER_TARGET_VIEW_DESC RtvDesc = {};
			RtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
			RtvDesc.Texture2DArray.MipSlice = 0;
			RtvDesc.Texture2DArray.PlaneSlice = 0;
			RtvDesc.Texture2DArray.FirstArraySlice = (UINT)i;
			RtvDesc.Texture2DArray.ArraySize = 1;

			if (TextureInfo.RTVFormat == DXGI_FORMAT_UNKNOWN)
			{
				RtvDesc.Format = TextureInfo.Format;
			}
			else
			{
				RtvDesc.Format = TextureInfo.RTVFormat;
			}

			TextureRef->RTVs.push_back(std::make_unique<TD3D12RenderTargetView>(GetDevice(), RtvDesc, TextureResource));
		}
	}

	// Create DSV
	if (CreateFlags & TexCreate_DSV)
	{
		D3D12_DEPTH_STENCIL_VIEW_DESC DSVDesc = {};
		DSVDesc.Flags = D3D12_DSV_FLAG_NONE;
		DSVDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		DSVDesc.Texture2D.MipSlice = 0;

		if (TextureInfo.DSVFormat == DXGI_FORMAT_UNKNOWN)
		{
			DSVDesc.Format = TextureInfo.Format;
		}
		else
		{
			DSVDesc.Format = TextureInfo.DSVFormat;
		}

		TextureRef->DSVs.push_back(std::make_unique<TD3D12DepthStencilView>(GetDevice(), DSVDesc, TextureResource));
	}
	else if (CreateFlags & TexCreate_CubeDSV)
	{
		for (size_t i = 0; i < 6; i++)
		{
			D3D12_DEPTH_STENCIL_VIEW_DESC DSVDesc = {};
			DSVDesc.Flags = D3D12_DSV_FLAG_NONE;
			DSVDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
			DSVDesc.Texture2DArray.MipSlice = 0;
			DSVDesc.Texture2DArray.FirstArraySlice = (UINT)i;
			DSVDesc.Texture2DArray.ArraySize = 1;

			if (TextureInfo.DSVFormat == DXGI_FORMAT_UNKNOWN)
			{
				DSVDesc.Format = TextureInfo.Format;
			}
			else
			{
				DSVDesc.Format = TextureInfo.DSVFormat;
			}

			TextureRef->DSVs.push_back(std::make_unique<TD3D12DepthStencilView>(GetDevice(), DSVDesc, TextureResource));
		}
	}

	// Create UAV
	if(CreateFlags & TexCreate_UAV)
	{
		D3D12_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {};
		UAVDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
		UAVDesc.Texture2D.MipSlice = 0;
		
		if (TextureInfo.UAVFormat == DXGI_FORMAT_UNKNOWN)
		{
			UAVDesc.Format = TextureInfo.Format;
		}
		else
		{
			UAVDesc.Format = TextureInfo.UAVFormat;
		}

		TextureRef->UAVs.push_back(std::make_unique<TD3D12UnorderedAccessView>(GetDevice(), UAVDesc, TextureResource));
	}
}

void TD3D12RHI::UploadTextureData(TD3D12TextureRef Texture, const std::vector<D3D12_SUBRESOURCE_DATA>& InitData)
{
	auto TextureResource = Texture->GetResource();
	D3D12_RESOURCE_DESC TexDesc = TextureResource->D3DResource->GetDesc();

	//GetCopyableFootprints
	const UINT NumSubresources = (UINT)InitData.size();
	std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> Layouts(NumSubresources);
	std::vector<uint32_t> NumRows(NumSubresources);
	std::vector<uint64_t> RowSizesInBytes(NumSubresources);

	uint64_t RequiredSize = 0;
	Device->GetD3DDevice()->GetCopyableFootprints(&TexDesc, 0, NumSubresources, 0, &Layouts[0], &NumRows[0], &RowSizesInBytes[0], &RequiredSize);

	//Create upload resource
	TD3D12ResourceLocation UploadResourceLocation;
	auto UploadBufferAllocator = GetDevice()->GetUploadBufferAllocator();
	void* MappedData = UploadBufferAllocator->AllocUploadResource((uint32_t)RequiredSize, D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT, UploadResourceLocation);
	ID3D12Resource* UploadBuffer = UploadResourceLocation.UnderlyingResource->D3DResource.Get();

	//Copy contents to upload resource
	for (uint32_t i = 0; i < NumSubresources; ++i)
	{
		if (RowSizesInBytes[i] > SIZE_T(-1))
		{
			assert(0);
		}
		D3D12_MEMCPY_DEST DestData = { (BYTE*)MappedData + Layouts[i].Offset, Layouts[i].Footprint.RowPitch, SIZE_T(Layouts[i].Footprint.RowPitch) * SIZE_T(NumRows[i]) };
		MemcpySubresource(&DestData, &(InitData[i]), static_cast<SIZE_T>(RowSizesInBytes[i]), NumRows[i], Layouts[i].Footprint.Depth);
	}

	//Copy data from upload resource to default resource
	TransitionResource(TextureResource, D3D12_RESOURCE_STATE_COPY_DEST);

	for (UINT i = 0; i < NumSubresources; ++i)
	{
		Layouts[i].Offset += UploadResourceLocation.OffsetFromBaseOfResource;

		CD3DX12_TEXTURE_COPY_LOCATION Src;
		Src.pResource = UploadBuffer;
		Src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		Src.PlacedFootprint = Layouts[i];

		CD3DX12_TEXTURE_COPY_LOCATION Dst;
		Dst.pResource = TextureResource->D3DResource.Get();
		Dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		Dst.SubresourceIndex = i;

		CopyTextureRegion(&Dst, 0, 0, 0, &Src, nullptr);
	}

	TransitionResource(TextureResource, D3D12_RESOURCE_STATE_COMMON);
}