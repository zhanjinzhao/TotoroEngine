#include "Shader.h"
#include "File/FileHelpers.h"

void TShaderDefines::GetD3DShaderMacro(std::vector<D3D_SHADER_MACRO>& OutMacros) const
{
	for (const auto& Pair : DefinesMap)
	{
		D3D_SHADER_MACRO Macro;
		Macro.Name = Pair.first.c_str();
		Macro.Definition = Pair.second.c_str();
		OutMacros.push_back(Macro);
	}

	D3D_SHADER_MACRO Macro;
	Macro.Name = NULL;
	Macro.Definition = NULL;
	OutMacros.push_back(Macro);
}

bool TShaderDefines::operator == (const TShaderDefines& Other) const
{
	if (DefinesMap.size() != Other.DefinesMap.size())
	{
		return false;
	}

	for (const auto& Pair : DefinesMap)
	{
		const std::string Key = Pair.first;
		const std::string Value = Pair.second;

		auto Iter = Other.DefinesMap.find(Key);
		if (Iter == Other.DefinesMap.end() || Iter->second != Value)
		{
			return false;
		}
	}

	return true;
}

void TShaderDefines::SetDefine(const std::string& Name, const std::string& Definition)
{
	DefinesMap.insert_or_assign(Name, Definition);
}


TShader::TShader(const TShaderInfo& InShaderInfo, TD3D12RHI* InD3D12RHI)
	: ShaderInfo(InShaderInfo), D3D12RHI(InD3D12RHI)
{
	Initialize();

	assert((ShaderInfo.bCreateVS | ShaderInfo.bCreatePS) ^ ShaderInfo.bCreateCS);
}

void TShader::Initialize()
{
	// Compile Shaders
	std::wstring ShaderDir = TFileHelpers::EngineDir() + L"Resource/Shaders/";
	std::wstring FilePath = ShaderDir + TFormatConvert::StrToWStr(ShaderInfo.FileName) + L".hlsl";

	std::vector<D3D_SHADER_MACRO> ShaderMacros;
	ShaderInfo.ShaderDefines.GetD3DShaderMacro(ShaderMacros);

	if (ShaderInfo.bCreateVS)
	{
		auto VSBlob = CompileShader(FilePath, ShaderMacros.data(), ShaderInfo.VSEntryPoint, "vs_5_1");
		ShaderPass["VS"] = VSBlob;

		GetShaderParameters(VSBlob, EShaderType::VERTEX_SHADER);
	}

	if (ShaderInfo.bCreatePS)
	{
		auto PSBlob = CompileShader(FilePath, ShaderMacros.data(), ShaderInfo.PSEntryPoint, "ps_5_1");
		ShaderPass["PS"] = PSBlob;

		GetShaderParameters(PSBlob, EShaderType::PIXEL_SHADER);
	}
	
	if (ShaderInfo.bCreateCS)
	{
		auto CSBlob = CompileShader(FilePath, ShaderMacros.data(), ShaderInfo.CSEntryPoint, "cs_5_1");
		ShaderPass["CS"] = CSBlob;

		GetShaderParameters(CSBlob, EShaderType::COMPUTE_SHADER);
	}
	
	// Create rootSignature
	CreateRootSignature();
}

Microsoft::WRL::ComPtr<ID3DBlob> TShader::CompileShader(const std::wstring& Filename, const D3D_SHADER_MACRO* Defines, const std::string& Entrypoint, const std::string& Target)
{
	UINT CompileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG) 
	// Enable better shader debugging with the graphics debugging tools.
	CompileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	HRESULT hr = S_OK;

	ComPtr<ID3DBlob> ByteCode = nullptr;
	ComPtr<ID3DBlob> Errors;
	hr = D3DCompileFromFile(Filename.c_str(), Defines, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		Entrypoint.c_str(), Target.c_str(), CompileFlags, 0, &ByteCode, &Errors);

	if (Errors != nullptr)
		OutputDebugStringA((char*)Errors->GetBufferPointer());

	ThrowIfFailed(hr);

	return ByteCode;
}

void TShader::GetShaderParameters(ComPtr<ID3DBlob> PassBlob, EShaderType ShaderType)
{
	ID3D12ShaderReflection* Reflection = NULL;
	D3DReflect(PassBlob->GetBufferPointer(), PassBlob->GetBufferSize(), IID_ID3D12ShaderReflection, (void**)&Reflection);

	D3D12_SHADER_DESC ShaderDesc;
	Reflection->GetDesc(&ShaderDesc);

	//printf("ShaderName: %s \n", ShaderInfo.ShaderName.c_str());

	for (UINT i = 0; i < ShaderDesc.BoundResources; i++)
	{
		D3D12_SHADER_INPUT_BIND_DESC  ResourceDesc;
		Reflection->GetResourceBindingDesc(i, &ResourceDesc);

		auto ShaderVarName = ResourceDesc.Name;
		auto ResourceType = ResourceDesc.Type;
		auto RegisterSpace = ResourceDesc.Space;	
		auto BindPoint = ResourceDesc.BindPoint;
		auto BindCount = ResourceDesc.BindCount;

		//printf("ShaderVarName: %s, ", ShaderVarName);
		//printf("ResourceType: %d, ", ResourceType);
		//printf("RegisterSpace: %d \n", RegisterSpace);
		//printf("BindPoint:  %d, ", BindPoint);
		//printf("BindCount: %d \n", BindCount);

		if (ResourceType == D3D_SHADER_INPUT_TYPE::D3D_SIT_CBUFFER)
		{
			TShaderCBVParameter Param;
			Param.Name = ShaderVarName;
			Param.ShaderType = ShaderType;
			Param.BindPoint = BindPoint;
			Param.RegisterSpace = RegisterSpace;

			CBVParams.push_back(Param);
		}
		else if (ResourceType == D3D_SHADER_INPUT_TYPE::D3D_SIT_STRUCTURED
			  || ResourceType == D3D_SHADER_INPUT_TYPE::D3D_SIT_TEXTURE)
		{
			TShaderSRVParameter Param;
			Param.Name = ShaderVarName;
			Param.ShaderType = ShaderType;
			Param.BindPoint = BindPoint;
			Param.BindCount = BindCount;
			Param.RegisterSpace = RegisterSpace;

			SRVParams.push_back(Param);
		}
		else if (ResourceType == D3D_SHADER_INPUT_TYPE::D3D_SIT_UAV_RWSTRUCTURED
			  || ResourceType == D3D_SHADER_INPUT_TYPE::D3D_SIT_UAV_RWTYPED)
		{
			assert(ShaderType == EShaderType::COMPUTE_SHADER);

			TShaderUAVParameter Param;
			Param.Name = ShaderVarName;
			Param.ShaderType = ShaderType;
			Param.BindPoint = BindPoint;
			Param.BindCount = BindCount;
			Param.RegisterSpace = RegisterSpace;

			UAVParams.push_back(Param);
		}
		else if (ResourceType == D3D_SHADER_INPUT_TYPE::D3D_SIT_SAMPLER)
		{
			assert(ShaderType == EShaderType::PIXEL_SHADER);

			TShaderSamplerParameter Param;
			Param.Name = ShaderVarName;
			Param.ShaderType = ShaderType;
			Param.BindPoint = BindPoint;
			Param.RegisterSpace = RegisterSpace;

			SamplerParams.push_back(Param);
		}
	}
}

D3D12_SHADER_VISIBILITY TShader::GetShaderVisibility(EShaderType ShaderType)
{
	D3D12_SHADER_VISIBILITY ShaderVisibility;
	if (ShaderType == EShaderType::VERTEX_SHADER)
	{
		ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	}
	else if (ShaderType == EShaderType::PIXEL_SHADER)
	{
		ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	}
	else if(ShaderType == EShaderType::COMPUTE_SHADER)
	{
		ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	}
	else
	{
		assert(0);
	}

	return ShaderVisibility;
}

std::vector<CD3DX12_STATIC_SAMPLER_DESC> TShader::CreateStaticSamplers()
{
	// Applications usually only need a handful of samplers.  So just define them all up front
	// and keep them available as part of the root signature.  

	CD3DX12_STATIC_SAMPLER_DESC PointWrap(
		0, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	CD3DX12_STATIC_SAMPLER_DESC PointClamp(
		1, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	CD3DX12_STATIC_SAMPLER_DESC LinearWrap(
		2, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	CD3DX12_STATIC_SAMPLER_DESC LinearClamp(
		3, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	CD3DX12_STATIC_SAMPLER_DESC AnisotropicWrap(
		4, // shaderRegister
		D3D12_FILTER_ANISOTROPIC, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressW
		0.0f,                             // mipLODBias
		8);                               // maxAnisotropy

	CD3DX12_STATIC_SAMPLER_DESC AnisotropicClamp(
		5, // shaderRegister
		D3D12_FILTER_ANISOTROPIC, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressW
		0.0f,                              // mipLODBias
		8);                                // maxAnisotropy

	CD3DX12_STATIC_SAMPLER_DESC Shadow(
		6, // shaderRegister
		D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressW
		0.0f,                               // mipLODBias
		16,                                 // maxAnisotropy
		D3D12_COMPARISON_FUNC_LESS_EQUAL,
		D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK);

	CD3DX12_STATIC_SAMPLER_DESC DepthMap(
		7, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR,   // filter
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressW
		0.0f,
		0,
		D3D12_COMPARISON_FUNC_LESS_EQUAL,
		D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE);


	std::vector<CD3DX12_STATIC_SAMPLER_DESC> StaticSamplers;
	StaticSamplers.push_back(PointWrap);
	StaticSamplers.push_back(PointClamp);
	StaticSamplers.push_back(LinearWrap);
	StaticSamplers.push_back(LinearClamp);
	StaticSamplers.push_back(AnisotropicWrap);
	StaticSamplers.push_back(AnisotropicClamp);
	StaticSamplers.push_back(Shadow);
	StaticSamplers.push_back(DepthMap);

	return StaticSamplers;
}

void TShader::CreateRootSignature()
{
	//------------------------------------------------Set SlotRootParameter---------------------------------------
	std::vector<CD3DX12_ROOT_PARAMETER> SlotRootParameter;

	// CBV
	for (const TShaderCBVParameter& Param : CBVParams)
	{
		if (CBVSignatureBaseBindSlot == -1)
		{
			CBVSignatureBaseBindSlot = (UINT)SlotRootParameter.size();
		}

		CD3DX12_ROOT_PARAMETER RootParam;
		RootParam.InitAsConstantBufferView(Param.BindPoint, Param.RegisterSpace, GetShaderVisibility(Param.ShaderType));
		SlotRootParameter.push_back(RootParam);
	}

	// SRV
	{
		for (const TShaderSRVParameter& Param : SRVParams)
		{
			SRVCount += Param.BindCount;
		}

		if (SRVCount > 0)
		{
			SRVSignatureBindSlot = (UINT)SlotRootParameter.size();

			CD3DX12_DESCRIPTOR_RANGE SRVTable;
			SRVTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, SRVCount, 0, 0);

			CD3DX12_ROOT_PARAMETER RootParam;
			D3D12_SHADER_VISIBILITY ShaderVisibility = ShaderInfo.bCreateCS ? D3D12_SHADER_VISIBILITY_ALL : D3D12_SHADER_VISIBILITY_PIXEL;  //TODO
			RootParam.InitAsDescriptorTable(1, &SRVTable, ShaderVisibility);
			SlotRootParameter.push_back(RootParam);
		}
	}

	// UAV
	{
		for (const TShaderUAVParameter& Param : UAVParams)
		{
			UAVCount += Param.BindCount;
		}

		if (UAVCount > 0)
		{
			UAVSignatureBindSlot = (UINT)SlotRootParameter.size();

			CD3DX12_DESCRIPTOR_RANGE UAVTable;
			UAVTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, UAVCount, 0, 0);

			CD3DX12_ROOT_PARAMETER RootParam;
			D3D12_SHADER_VISIBILITY ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL; //TODO
			RootParam.InitAsDescriptorTable(1, &UAVTable, ShaderVisibility);
			SlotRootParameter.push_back(RootParam);
		}
	}

	// Sampler
	// TODO
	auto StaticSamplers = CreateStaticSamplers();

	//------------------------------------------------SerializeRootSignature---------------------------------------

	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc((UINT)SlotRootParameter.size(), SlotRootParameter.data(),
		(UINT)StaticSamplers.size(), StaticSamplers.data(),
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

	if (errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}
	ThrowIfFailed(hr);

	ThrowIfFailed(D3D12RHI->GetDevice()->GetD3DDevice()->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(&RootSignature)));
}

bool TShader::SetParameter(std::string ParamName, TD3D12ConstantBufferRef ConstantBufferRef)
{
	bool FindParam = false;

	for (TShaderCBVParameter& Param : CBVParams)
	{
		if (Param.Name == ParamName)
		{
			Param.ConstantBufferRef = ConstantBufferRef;

			FindParam = true; 
		}
	}

	return FindParam;
}

bool TShader::SetParameter(std::string ParamName, TD3D12ShaderResourceView* SRV)
{
	std::vector<TD3D12ShaderResourceView*> SRVList;
	SRVList.push_back(SRV);

	return SetParameter(ParamName, SRVList);
}

bool TShader::SetParameter(std::string ParamName, const std::vector<TD3D12ShaderResourceView*>& SRVList)
{
	bool FindParam = false;

	for (TShaderSRVParameter& Param : SRVParams)
	{
		if (Param.Name == ParamName)
		{
			assert(SRVList.size() == Param.BindCount);

			Param.SRVList = SRVList;

			FindParam = true;
		}
	}

	return FindParam;
}

bool TShader::SetParameter(std::string ParamName, TD3D12UnorderedAccessView* UAV)
{
	std::vector<TD3D12UnorderedAccessView*> UAVList;
	UAVList.push_back(UAV);

	return SetParameter(ParamName, UAVList);
}

bool TShader::SetParameter(std::string ParamName, const std::vector<TD3D12UnorderedAccessView*>& UAVList)
{
	bool FindParam = false;

	for (TShaderUAVParameter& Param : UAVParams)
	{
		if (Param.Name == ParamName)
		{
			assert(UAVList.size() == Param.BindCount);

			Param.UAVList = UAVList;

			FindParam = true;
		}
	}

	return FindParam;
}

void TShader::BindParameters()
{
	auto CommandList = D3D12RHI->GetDevice()->GetCommandList();
	auto DescriptorCache = D3D12RHI->GetDevice()->GetCommandContext()->GetDescriptorCache();

	CheckBindings();

	bool bComputeShader = ShaderInfo.bCreateCS;

	// CBV binding
	for (int i = 0; i < CBVParams.size(); i++)
	{
		UINT RootParamIdx = CBVSignatureBaseBindSlot + i;
		D3D12_GPU_VIRTUAL_ADDRESS GPUVirtualAddress = CBVParams[i].ConstantBufferRef->ResourceLocation.GPUVirtualAddress;

		if (bComputeShader)
		{
			CommandList->SetComputeRootConstantBufferView(RootParamIdx, GPUVirtualAddress);
		}
		else
		{
			CommandList->SetGraphicsRootConstantBufferView(RootParamIdx, GPUVirtualAddress);
		}
	}


	// SRV binding
	if (SRVCount > 0)
	{
		std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> SrcDescriptors;
		SrcDescriptors.resize(SRVCount);

		for (const TShaderSRVParameter& Param : SRVParams)
		{
			for (UINT i = 0; i < Param.SRVList.size(); i++)
			{
				UINT Index = Param.BindPoint + i;
				SrcDescriptors[Index] = Param.SRVList[i]->GetDescriptorHandle();
			}
		}

		UINT RootParamIdx = SRVSignatureBindSlot;
		auto GpuDescriptorHandle = DescriptorCache->AppendCbvSrvUavDescriptors(SrcDescriptors);

		if (bComputeShader)
		{
			CommandList->SetComputeRootDescriptorTable(RootParamIdx, GpuDescriptorHandle);
		}
		else
		{
			CommandList->SetGraphicsRootDescriptorTable(RootParamIdx, GpuDescriptorHandle);
		}
	}

	// UAV binding
	if(UAVCount > 0)
	{
		std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> SrcDescriptors;
		SrcDescriptors.resize(UAVCount);

		for (const TShaderUAVParameter& Param : UAVParams)
		{
			for (UINT i = 0; i < Param.UAVList.size(); i++)
			{
				UINT Index = Param.BindPoint + i;
				SrcDescriptors[Index] = Param.UAVList[i]->GetDescriptorHandle();
			}
		}

		UINT RootParamIdx = UAVSignatureBindSlot;
		auto GpuDescriptorHandle = DescriptorCache->AppendCbvSrvUavDescriptors(SrcDescriptors);

		if (bComputeShader)
		{
			CommandList->SetComputeRootDescriptorTable(RootParamIdx, GpuDescriptorHandle);
		}
		else
		{
			assert(0);
		}
	}

	ClearBindings();
}

void TShader::CheckBindings()
{
	for (TShaderCBVParameter& Param : CBVParams)
	{
		assert(Param.ConstantBufferRef);
	}

	for (TShaderSRVParameter& Param : SRVParams)
	{
		assert(Param.SRVList.size() > 0);
	}

	for (TShaderUAVParameter& Param : UAVParams)
	{
		assert(Param.UAVList.size() > 0);
	}
}

void TShader::ClearBindings()
{
	for (TShaderCBVParameter& Param : CBVParams)
	{
		Param.ConstantBufferRef = nullptr;
	}

	for (TShaderSRVParameter& Param : SRVParams)
	{
		Param.SRVList.clear();
	}

	for (TShaderUAVParameter& Param : UAVParams)
	{
		Param.UAVList.clear();
	}
}