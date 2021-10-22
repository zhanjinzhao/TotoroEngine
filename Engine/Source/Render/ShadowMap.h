#pragma once

#include "Render/SceneCapture2D.h"
#include "Render/SceneCaptureCube.h"

enum class EShadowMapType
{
	SM_SINGLE,
	SM_OMNI
};

enum class EShadowMapImpl
{
	PCF,
	PCSS,
	VSM,
	SDF,
};

class TShadowMap2D : public TSceneCapture2D
{
public:
	TShadowMap2D(UINT InWidth, UINT InHeight, DXGI_FORMAT Format, TD3D12RHI* InD3D12RHI);
};

class TShadowMapCube : public TSceneCaptureCube
{
public:
	TShadowMapCube(UINT Size, DXGI_FORMAT Format, TD3D12RHI* InD3D12RHI);
};

 