#include "RenderTarget.h"

TRenderTarget::TRenderTarget(TD3D12RHI* InD3D12RHI, bool RenderDepth, UINT InWidth, UINT InHeight, DXGI_FORMAT InFormat, TVector4 InClearValue)
	: D3D12RHI(InD3D12RHI), bRenderDepth(RenderDepth), Width(InWidth), Height(InHeight), Format(InFormat), ClearValue(InClearValue)
{

}

TRenderTarget::~TRenderTarget()
{

}


TRenderTarget2D::TRenderTarget2D(TD3D12RHI* InD3D12RHI, bool RenderDepth, UINT InWidth, UINT InHeight, DXGI_FORMAT InFormat, TVector4 InClearValue)
	:TRenderTarget(InD3D12RHI, RenderDepth, InWidth, InHeight, InFormat, InClearValue)
{
	CreateTexture();
}

void TRenderTarget2D::CreateTexture()
{
	//Create D3DTexture
	TTextureInfo TextureInfo;
	TextureInfo.Type = ETextureType::TEXTURE_2D;
	TextureInfo.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	TextureInfo.Width = Width;
	TextureInfo.Height = Height;
	TextureInfo.Depth = 1;
	TextureInfo.MipCount = 1;
	TextureInfo.ArraySize = 1;
	TextureInfo.Format = Format;

	if (bRenderDepth)
	{
		TextureInfo.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
		TextureInfo.SRVFormat = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;

		D3DTexture = D3D12RHI->CreateTexture(TextureInfo, TexCreate_DSV | TexCreate_SRV);
	}
	else
	{
		D3DTexture = D3D12RHI->CreateTexture(TextureInfo, TexCreate_RTV | TexCreate_SRV, ClearValue);
	}
}

TD3D12RenderTargetView* TRenderTarget2D::GetRTV() const
{
	if (bRenderDepth)
	{
		return nullptr;
	}
	else
	{
		return D3DTexture->RTVs[0].get();
	}
}

TD3D12DepthStencilView* TRenderTarget2D::GetDSV() const
{
	if (bRenderDepth)
	{
		return D3DTexture->DSVs[0].get();
	}
	else
	{
		return nullptr;
	}
}

TD3D12ShaderResourceView* TRenderTarget2D::GetSRV() const
{
	return D3DTexture->SRVs[0].get();
}

TRenderTargetCube::TRenderTargetCube(TD3D12RHI* InD3D12RHI, bool RenderDepth, UINT InWidth, UINT InHeight, DXGI_FORMAT InFormat, TVector4 InClearValue)
	:TRenderTarget(InD3D12RHI, RenderDepth, InWidth, InHeight, InFormat, InClearValue)
{
	CreateTexture();
}

void TRenderTargetCube::CreateTexture()
{
	//Create D3DTexture
	TTextureInfo TextureInfo;
	TextureInfo.Type = ETextureType::TEXTURE_CUBE;
	TextureInfo.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	TextureInfo.Width = Width;
	TextureInfo.Height = Height;
	TextureInfo.Depth = 1;
	TextureInfo.MipCount = 1;
	TextureInfo.ArraySize = 6;
	TextureInfo.Format = Format;

	if (bRenderDepth)
	{
		TextureInfo.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
		TextureInfo.SRVFormat = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;

		D3DTexture = D3D12RHI->CreateTexture(TextureInfo, TexCreate_CubeDSV | TexCreate_SRV);
	}
	else
	{
		D3DTexture = D3D12RHI->CreateTexture(TextureInfo, TexCreate_CubeRTV | TexCreate_SRV, ClearValue);
	}
}

TD3D12RenderTargetView* TRenderTargetCube::GetRTV(int Index) const
{
	if (bRenderDepth)
	{
		return nullptr;
	}
	else
	{
		assert(D3DTexture->RTVs.size() > Index);

		return D3DTexture->RTVs[Index].get();
	}
}

TD3D12DepthStencilView* TRenderTargetCube::GetDSV(int Index) const
{
	if (bRenderDepth)
	{
		assert(D3DTexture->DSVs.size() > Index);

		return D3DTexture->DSVs[Index].get();
	}
	else
	{
		return nullptr;
	}
}

TD3D12ShaderResourceView* TRenderTargetCube::GetSRV() const
{
	return D3DTexture->SRVs[0].get();
}