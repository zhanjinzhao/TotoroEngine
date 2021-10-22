#pragma once

#include "d3dx12.h"
#include <dxgi1_4.h>
#include <dxgidebug.h>
#include <comdef.h>
#include <windows.h>
#include <wrl.h>
#include <d3d12.h>
#include <D3Dcompiler.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>
#include <DirectXCollision.h>

#include "Utils/FormatConvert.h"
#include "Math/Math.h"



class DxException
{
public:
	DxException() = default;
	DxException(HRESULT hr, const std::wstring& functionName, const std::wstring& filename, int lineNumber);

	std::wstring ToString()const;

	HRESULT ErrorCode = S_OK;
	std::wstring FunctionName;
	std::wstring Filename;
	int LineNumber = -1;
};

#ifndef ThrowIfFailed
#define ThrowIfFailed(x)                                              \
{                                                                     \
    HRESULT hr__ = (x);                                               \
    std::wstring wfn = TFormatConvert::StrToWStr(__FILE__);           \
    if(FAILED(hr__)) { throw DxException(hr__, L#x, wfn, __LINE__); } \
}
#endif

#ifndef ReleaseCom
#define ReleaseCom(x) { if(x){ x->Release(); x = 0; } }
#endif

template<UINT TNameLength>
inline void SetDebugName(_In_ ID3D12DeviceChild* resource, _In_z_ const wchar_t(&name)[TNameLength]) noexcept
{
#if !defined(NO_D3D12_DEBUG_NAME) && (defined(_DEBUG) || defined(PROFILE))
	resource->SetName(name);
#else
	UNREFERENCED_PARAMETER(resource);
	UNREFERENCED_PARAMETER(name);
#endif
}


// Aligns a value to the nearest higher multiple of 'Alignment'.
inline uint32_t AlignArbitrary(uint32_t Val, uint32_t Alignment)
{
	return ((Val + Alignment - 1) / Alignment) * Alignment;
}

inline UIntPoint GetTextureSize(ID3D12Resource* Texture)
{
	const auto Desc = Texture->GetDesc();
	return UIntPoint(static_cast<uint32_t>(Desc.Width), static_cast<uint32_t>(Desc.Height));
}