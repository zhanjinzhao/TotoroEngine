#pragma once

#include "D3D12/D3D12Texture.h"
#include "D3D12/D3D12RHI.h"

class TRenderTarget
{
protected:
public:
	TRenderTarget(TD3D12RHI* InD3D12RHI, bool RenderDepth, UINT InWidth, UINT InHeight, DXGI_FORMAT InFormat, TVector4 InClearValue = TVector4::Zero);

	virtual ~TRenderTarget();

public:
	TD3D12TextureRef GetTexture() const { return D3DTexture; }

	TD3D12Resource* GetResource() const { return D3DTexture->GetResource(); }

	DXGI_FORMAT GetFormat() const { return Format; }

	TVector4 GetClearColor()  { return ClearValue; }

	float* GetClearColorPtr() { return (float*)&ClearValue; }

protected:
	bool bRenderDepth = false;

	TD3D12RHI* D3D12RHI = nullptr;

	TD3D12TextureRef D3DTexture = nullptr;

	UINT Width;

	UINT Height;

	DXGI_FORMAT Format;

	TVector4 ClearValue;
};

class TRenderTarget2D : public TRenderTarget
{
public:
	TRenderTarget2D(TD3D12RHI* InD3D12RHI, bool RenderDepth, UINT InWidth, UINT InHeight, DXGI_FORMAT InFormat, TVector4 InClearValue = TVector4::Zero);

	TD3D12RenderTargetView* GetRTV() const;

	TD3D12DepthStencilView* GetDSV() const;

	TD3D12ShaderResourceView* GetSRV() const;

private:
	void CreateTexture();
};

class TRenderTargetCube : public TRenderTarget
{
public:
	TRenderTargetCube(TD3D12RHI* InD3D12RHI, bool RenderDepth, UINT Size, DXGI_FORMAT InFormat, TVector4 InClearValue = TVector4::Zero);

	TD3D12RenderTargetView* GetRTV(int Index) const;

	TD3D12DepthStencilView* GetDSV(int Index) const;

	TD3D12ShaderResourceView* GetSRV() const;

private:
	void CreateTexture();
};