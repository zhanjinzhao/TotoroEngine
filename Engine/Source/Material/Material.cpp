#include "Material.h"

TMaterial::TMaterial(const std::string& InName, const std::string& InShaderName)
    :Name(InName), 
    ShaderName(InShaderName)
{
}

TShader* TMaterial::GetShader(const TShaderDefines& ShaderDefines, TD3D12RHI* D3D12RHI)
{
    auto Iter = ShaderMap.find(ShaderDefines);

    if (Iter == ShaderMap.end())
    {
        // Create new shader
		TShaderInfo ShaderInfo;
		ShaderInfo.ShaderName = ShaderName;
        ShaderInfo.FileName = ShaderName;
        ShaderInfo.ShaderDefines = ShaderDefines;
		ShaderInfo.bCreateVS = true;
		ShaderInfo.bCreatePS = true;
        std::unique_ptr<TShader> NewShader = std::make_unique<TShader>(ShaderInfo, D3D12RHI);

        ShaderMap.insert({ ShaderDefines, std::move(NewShader)});

        return ShaderMap[ShaderDefines].get();
    }
    else
    {
        return Iter->second.get();
    }
}