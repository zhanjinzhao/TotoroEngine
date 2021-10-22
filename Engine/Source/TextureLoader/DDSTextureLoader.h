// Modified version of DirectXTK12's source file

//--------------------------------------------------------------------------------------
// File: DDSTextureLoader.h
//
// Functions for loading a DDS texture and creating a Direct3D runtime resource for it
//
// Note these functions are useful as a light-weight runtime loader for DDS files. For
// a full-featured DDS file reader, writer, and texture processing pipeline see
// the 'Texconv' sample and the 'DirectXTex' library.
//
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// http://go.microsoft.com/fwlink/?LinkID=615561
//--------------------------------------------------------------------------------------

#ifdef _MSC_VER
#pragma once
#endif

#include <wrl.h>
#include <d3d11_1.h>
#include "Texture/TextureInfo.h"

#pragma warning(push)
#pragma warning(disable : 4005)
#include <stdint.h>

#pragma warning(pop)

#if defined(_MSC_VER) && (_MSC_VER<1610) && !defined(_In_reads_)
#define _In_reads_(exp)
#define _Out_writes_(exp)
#define _In_reads_bytes_(exp)
#define _In_reads_opt_(exp)
#define _Outptr_opt_
#endif

#ifndef _Use_decl_annotations_
#define _Use_decl_annotations_
#endif

namespace DirectX
{
    enum DDS_ALPHA_MODE
    {
        DDS_ALPHA_MODE_UNKNOWN       = 0,
        DDS_ALPHA_MODE_STRAIGHT      = 1,
        DDS_ALPHA_MODE_PREMULTIPLIED = 2,
        DDS_ALPHA_MODE_OPAQUE        = 3,
        DDS_ALPHA_MODE_CUSTOM        = 4,
    };


	HRESULT CreateDDSTextureFromFile(_In_z_ const wchar_t* szFileName,
                                _Out_ TTextureInfo& TextureInfo,
                                _Out_ std::vector<D3D12_SUBRESOURCE_DATA>& initData,
                                _Out_ std::vector<uint8_t>& ddsData,
                                _In_ bool forceSRGB,
		                        _In_ size_t maxsize = 0,
		                        _Out_opt_ DDS_ALPHA_MODE* alphaMode = nullptr
	);
}