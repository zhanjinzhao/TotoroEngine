#pragma once

#include "RenderTarget.h"
#include "SceneView.h"

class TSceneCaptureCube
{
public:
	TSceneCaptureCube(bool RenderDepth, UINT Size, DXGI_FORMAT Format, TD3D12RHI* InD3D12RHI);

	TRenderTargetCube* GetRTCube() { return RTCube.get(); }

	const TSceneView& GetSceneView(UINT Index) const
	{
		return SceneViews[Index];
	}

	D3D12_VIEWPORT GetViewport() { return Viewport; }

	D3D12_RECT GetScissorRect() { return ScissorRect; }

	void CreatePerspectiveViews(const TVector3& Eye, float NearPlane, float FarPlane);

	UINT GetCubeMapSize() { return CubeMapSize; }

private:

	void SetViewportAndScissorRect(UINT CubeMapSize);

private:
	TD3D12RHI* D3D12RHI = nullptr;

	UINT CubeMapSize;

	std::unique_ptr<TRenderTargetCube> RTCube = nullptr;

	TSceneView SceneViews[6];

	D3D12_VIEWPORT Viewport;

	D3D12_RECT ScissorRect;
};