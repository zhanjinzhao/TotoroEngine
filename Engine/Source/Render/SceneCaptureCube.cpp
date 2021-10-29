#include "SceneCaptureCube.h"

TSceneCaptureCube::TSceneCaptureCube(bool RenderDepth, UINT Size, DXGI_FORMAT Format, TD3D12RHI* InD3D12RHI)
	:D3D12RHI(InD3D12RHI), CubeMapSize(Size)
{
	D3D12RHI = InD3D12RHI;

	RTCube = std::make_unique<TRenderTargetCube>(D3D12RHI, RenderDepth, CubeMapSize, Format);

	SetViewportAndScissorRect(CubeMapSize);
}

void TSceneCaptureCube::SetViewportAndScissorRect(UINT CubeMapSize)
{
	Viewport = { 0.0f, 0.0f, (float)CubeMapSize, (float)CubeMapSize, 0.0f, 1.0f };

	ScissorRect = { 0, 0, (int)(Viewport.Width), (int)(Viewport.Height) };
}

void TSceneCaptureCube::CreatePerspectiveViews(const TVector3& Eye, float NearPlane, float FarPlane)
{
	// Look along each coordinate axis. 
	TVector3 Targets[6] =
	{
		Eye + TVector3(1.0f,  0.0f,  0.0f), // +X 
		Eye + TVector3(-1.0f, 0.0f,  0.0f), // -X 
		Eye + TVector3(0.0f,  1.0f,  0.0f), // +Y 
		Eye + TVector3(0.0f,  -1.0f, 0.0f), // -Y 
		Eye + TVector3(0.0f,  0.0f,  1.0f), // +Z 
		Eye + TVector3(0.0f,  0.0f, -1.0f)  // -Z 
	};

	TVector3 Ups[6] =
	{
		{0.0f, 1.0f, 0.0f},  // +X 
		{0.0f, 1.0f, 0.0f},  // -X 
		{0.0f, 0.0f, -1.0f}, // +Y 
		{0.0f, 0.0f, +1.0f}, // -Y 
		{0.0f, 1.0f, 0.0f},	 // +Z 
		{0.0f, 1.0f, 0.0f}	 // -Z 
	};

	for (int i = 0; i < 6; ++i)
	{
		SceneViews[i].EyePos = Eye;
		SceneViews[i].View = TMatrix::CreateLookAt(Eye, Targets[i], Ups[i]);

		float Fov = 0.5f * TMath::Pi;
		float AspectRatio = 1.0f; //Square
		SceneViews[i].Proj = TMatrix::CreatePerspectiveFieldOfView(Fov, AspectRatio, NearPlane, FarPlane);

		SceneViews[i].Near = NearPlane;
		SceneViews[i].Far = FarPlane;
	}
}