#pragma once

#include "RenderTarget.h"
#include "SceneView.h"

class TSceneCapture2D
{
public:
	TSceneCapture2D(bool RenderDepth, UINT InWidth, UINT InHeight, DXGI_FORMAT Format, TD3D12RHI* InD3D12RHI);

	TRenderTarget2D* GetRT() { return RT.get(); }

	const TSceneView& GetSceneView() const
	{
		return SceneView;
	}

	D3D12_VIEWPORT GetViewport() { return Viewport; }

	D3D12_RECT GetScissorRect() { return ScissorRect; }

	// FOV: in radians
	void CreatePerspectiveView(const TVector3& Eye, const TVector3& Target, const TVector3& Up, 
		float Fov, float AspectRatio, float NearPlane, float FarPlane);

	void CreateOrthographicView(const TVector3& Eye, const TVector3& Target, const TVector3& Up, 
		float Left, float Right, float Bottom, float Top, float NearPlane, float FarPlane);

	UINT GetWidth() { return Width; }

	UINT GetHeight() { return Height; }

private:
	void SetViewportAndScissorRect();

private:
	TD3D12RHI* D3D12RHI = nullptr;

	UINT Width;

	UINT Height;

	std::unique_ptr<TRenderTarget2D> RT = nullptr;

	TSceneView SceneView;

	D3D12_VIEWPORT Viewport;

	D3D12_RECT ScissorRect;
};