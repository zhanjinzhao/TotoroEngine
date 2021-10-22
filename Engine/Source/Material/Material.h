#pragma once

#include <string>
#include <unordered_map>
#include "D3D12/D3D12Utils.h"
#include "Shader/Shader.h"
#include "Math/Math.h"

enum class EShadingMode
{
    DefaultLit,
    Unlit,
};

struct TMaterialParameters
{
public:
    TVector4 DiffuseAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f };

    TVector3 FresnelR0 = { 0.01f, 0.01f, 0.01f };

    float Roughness = 64.0f;

    TVector3 EmissiveColor = { 0.0f, 0.0f, 0.0f };

    // Used in texture mapping.
    TMatrix MatTransform = TMatrix::Identity;

    std::unordered_map<std::string/*Parameter*/, std::string/*TextureName*/> TextureMap;
};

struct TMaterialRenderState
{
	D3D12_CULL_MODE CullMode = D3D12_CULL_MODE_BACK;

	D3D12_COMPARISON_FUNC DepthFunc = D3D12_COMPARISON_FUNC_LESS;
};


class TMaterial
{
public:
    TMaterial(const std::string& InName, const std::string& InShaderName);

    TShader* GetShader(const TShaderDefines& ShaderDefines, TD3D12RHI* D3D12RHI);

public:
	std::string Name;

    EShadingMode ShadingModel = EShadingMode::DefaultLit;

    TMaterialParameters Parameters;

    TMaterialRenderState RenderState;

private:
    std::string ShaderName;

    std::unordered_map<TShaderDefines, std::unique_ptr<TShader>> ShaderMap;
};
