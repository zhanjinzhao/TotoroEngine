#include "ShadowMap.h"

TShadowMap2D::TShadowMap2D(UINT InWidth, UINT InHeight, DXGI_FORMAT Format, TD3D12RHI* InD3D12RHI)
	:TSceneCapture2D(true, InWidth, InHeight, Format, InD3D12RHI)
{

}
 
TShadowMapCube::TShadowMapCube(UINT Size, DXGI_FORMAT Format, TD3D12RHI* InD3D12RHI)
	:TSceneCaptureCube(true, Size, Format, InD3D12RHI)
{

}








