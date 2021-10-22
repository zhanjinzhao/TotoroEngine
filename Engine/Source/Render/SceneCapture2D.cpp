#include "SceneCapture2D.h"

TSceneCapture2D::TSceneCapture2D(bool RenderDepth, UINT InWidth, UINT InHeight, DXGI_FORMAT Format, TD3D12RHI* InD3D12RHI)
	:D3D12RHI(InD3D12RHI), Width(InWidth), Height(InHeight)
{
	RT = std::make_unique<TRenderTarget2D>(D3D12RHI, RenderDepth, Width, Height, Format);

	SetViewportAndScissorRect();
}

void TSceneCapture2D::SetViewportAndScissorRect()
{
	Viewport = { 0.0f, 0.0f, (float)Width, (float)Height, 0.0f, 1.0f };

	ScissorRect = { 0, 0, (int)(Viewport.Width), (int)(Viewport.Height) };
}

void TSceneCapture2D::CreatePerspectiveView(const TVector3& Eye, const TVector3& Target, const TVector3& Up, float Fov, float AspectRatio, float NearPlane, float FarPlane)
{
	SceneView.EyePos = Eye;
	SceneView.View = TMatrix::CreateLookAt(Eye, Target, Up);
	SceneView.Proj = TMatrix::CreatePerspectiveFieldOfView(Fov, AspectRatio, NearPlane, FarPlane);

	SceneView.Near = NearPlane;
	SceneView.Far = FarPlane;
}

void TSceneCapture2D::CreateOrthographicView(const TVector3& Eye, const TVector3& Target, const TVector3& Up, float Left, float Right, float Bottom, float Top, float NearPlane, float FarPlane)
{
	SceneView.EyePos = Eye;
	SceneView.View = TMatrix::CreateLookAt(Eye, Target, Up);
	SceneView.Proj = TMatrix::CreateOrthographicOffCenter(Left, Right, Bottom, Top, NearPlane, FarPlane);

	SceneView.Near = NearPlane;
	SceneView.Far = FarPlane;
}