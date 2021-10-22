// Modified version of DirectXTK12's source file

//--------------------------------------------------------------------------------------
// File: WICTextureLoader.cpp
//
// Function for loading a WIC image and creating a Direct3D runtime texture for it
// (auto-generating mipmaps if possible)
//
// Note: Assumes application has already called CoInitializeEx
//
// Note these functions are useful for images created as simple 2D textures. For
// more complex resources, DDSTextureLoader is an excellent light-weight runtime loader.
// For a full-featured DDS file reader, writer, and texture processing pipeline see
// the 'Texconv' sample and the 'DirectXTex' library.
//
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// http://go.microsoft.com/fwlink/?LinkID=615561
//--------------------------------------------------------------------------------------

#include "WICTextureLoader.h"
#include <wincodec.h>
#include <assert.h>
#include "LoaderHelpers.h"
#include "File/PlatformHelpers.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

namespace
{
	//-------------------------------------------------------------------------------------
	// WIC Pixel Format Translation Data
	//-------------------------------------------------------------------------------------
	struct WICTranslate
	{
		const GUID& wic;
		DXGI_FORMAT         format;
	};

	constexpr WICTranslate g_WICFormats[] =
	{
		{ GUID_WICPixelFormat128bppRGBAFloat,       DXGI_FORMAT_R32G32B32A32_FLOAT },

		{ GUID_WICPixelFormat64bppRGBAHalf,         DXGI_FORMAT_R16G16B16A16_FLOAT },
		{ GUID_WICPixelFormat64bppRGBA,             DXGI_FORMAT_R16G16B16A16_UNORM },

		{ GUID_WICPixelFormat32bppRGBA,             DXGI_FORMAT_R8G8B8A8_UNORM },
		{ GUID_WICPixelFormat32bppBGRA,             DXGI_FORMAT_B8G8R8A8_UNORM },
		{ GUID_WICPixelFormat32bppBGR,              DXGI_FORMAT_B8G8R8X8_UNORM },

		{ GUID_WICPixelFormat32bppRGBA1010102XR,    DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM },
		{ GUID_WICPixelFormat32bppRGBA1010102,      DXGI_FORMAT_R10G10B10A2_UNORM },

		{ GUID_WICPixelFormat16bppBGRA5551,         DXGI_FORMAT_B5G5R5A1_UNORM },
		{ GUID_WICPixelFormat16bppBGR565,           DXGI_FORMAT_B5G6R5_UNORM },

		{ GUID_WICPixelFormat32bppGrayFloat,        DXGI_FORMAT_R32_FLOAT },
		{ GUID_WICPixelFormat16bppGrayHalf,         DXGI_FORMAT_R16_FLOAT },
		{ GUID_WICPixelFormat16bppGray,             DXGI_FORMAT_R16_UNORM },
		{ GUID_WICPixelFormat8bppGray,              DXGI_FORMAT_R8_UNORM },

		{ GUID_WICPixelFormat8bppAlpha,             DXGI_FORMAT_A8_UNORM },

		{ GUID_WICPixelFormat96bppRGBFloat,         DXGI_FORMAT_R32G32B32_FLOAT },
	};

	//-------------------------------------------------------------------------------------
	// WIC Pixel Format nearest conversion table
	//-------------------------------------------------------------------------------------

	struct WICConvert
	{
		const GUID& source;
		const GUID& target;
	};

	constexpr WICConvert g_WICConvert[] =
	{
		// Note target GUID in this conversion table must be one of those directly supported formats (above).

		{ GUID_WICPixelFormatBlackWhite,            GUID_WICPixelFormat8bppGray }, // DXGI_FORMAT_R8_UNORM

		{ GUID_WICPixelFormat1bppIndexed,           GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM 
		{ GUID_WICPixelFormat2bppIndexed,           GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM 
		{ GUID_WICPixelFormat4bppIndexed,           GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM 
		{ GUID_WICPixelFormat8bppIndexed,           GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM 

		{ GUID_WICPixelFormat2bppGray,              GUID_WICPixelFormat8bppGray }, // DXGI_FORMAT_R8_UNORM 
		{ GUID_WICPixelFormat4bppGray,              GUID_WICPixelFormat8bppGray }, // DXGI_FORMAT_R8_UNORM 

		{ GUID_WICPixelFormat16bppGrayFixedPoint,   GUID_WICPixelFormat16bppGrayHalf }, // DXGI_FORMAT_R16_FLOAT 
		{ GUID_WICPixelFormat32bppGrayFixedPoint,   GUID_WICPixelFormat32bppGrayFloat }, // DXGI_FORMAT_R32_FLOAT 

		{ GUID_WICPixelFormat16bppBGR555,           GUID_WICPixelFormat16bppBGRA5551 }, // DXGI_FORMAT_B5G5R5A1_UNORM

		{ GUID_WICPixelFormat32bppBGR101010,        GUID_WICPixelFormat32bppRGBA1010102 }, // DXGI_FORMAT_R10G10B10A2_UNORM

		{ GUID_WICPixelFormat24bppBGR,              GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM 
		{ GUID_WICPixelFormat24bppRGB,              GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM 
		{ GUID_WICPixelFormat32bppPBGRA,            GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM 
		{ GUID_WICPixelFormat32bppPRGBA,            GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM 

		{ GUID_WICPixelFormat48bppRGB,              GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM
		{ GUID_WICPixelFormat48bppBGR,              GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM
		{ GUID_WICPixelFormat64bppBGRA,             GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM
		{ GUID_WICPixelFormat64bppPRGBA,            GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM
		{ GUID_WICPixelFormat64bppPBGRA,            GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM

		{ GUID_WICPixelFormat48bppRGBFixedPoint,    GUID_WICPixelFormat64bppRGBAHalf }, // DXGI_FORMAT_R16G16B16A16_FLOAT 
		{ GUID_WICPixelFormat48bppBGRFixedPoint,    GUID_WICPixelFormat64bppRGBAHalf }, // DXGI_FORMAT_R16G16B16A16_FLOAT 
		{ GUID_WICPixelFormat64bppRGBAFixedPoint,   GUID_WICPixelFormat64bppRGBAHalf }, // DXGI_FORMAT_R16G16B16A16_FLOAT 
		{ GUID_WICPixelFormat64bppBGRAFixedPoint,   GUID_WICPixelFormat64bppRGBAHalf }, // DXGI_FORMAT_R16G16B16A16_FLOAT 
		{ GUID_WICPixelFormat64bppRGBFixedPoint,    GUID_WICPixelFormat64bppRGBAHalf }, // DXGI_FORMAT_R16G16B16A16_FLOAT 
		{ GUID_WICPixelFormat64bppRGBHalf,          GUID_WICPixelFormat64bppRGBAHalf }, // DXGI_FORMAT_R16G16B16A16_FLOAT 
		{ GUID_WICPixelFormat48bppRGBHalf,          GUID_WICPixelFormat64bppRGBAHalf }, // DXGI_FORMAT_R16G16B16A16_FLOAT 

		{ GUID_WICPixelFormat128bppPRGBAFloat,      GUID_WICPixelFormat128bppRGBAFloat }, // DXGI_FORMAT_R32G32B32A32_FLOAT 
		{ GUID_WICPixelFormat128bppRGBFloat,        GUID_WICPixelFormat128bppRGBAFloat }, // DXGI_FORMAT_R32G32B32A32_FLOAT 
		{ GUID_WICPixelFormat128bppRGBAFixedPoint,  GUID_WICPixelFormat128bppRGBAFloat }, // DXGI_FORMAT_R32G32B32A32_FLOAT 
		{ GUID_WICPixelFormat128bppRGBFixedPoint,   GUID_WICPixelFormat128bppRGBAFloat }, // DXGI_FORMAT_R32G32B32A32_FLOAT 
		{ GUID_WICPixelFormat32bppRGBE,             GUID_WICPixelFormat128bppRGBAFloat }, // DXGI_FORMAT_R32G32B32A32_FLOAT 

		{ GUID_WICPixelFormat32bppCMYK,             GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM
		{ GUID_WICPixelFormat64bppCMYK,             GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM
		{ GUID_WICPixelFormat40bppCMYKAlpha,        GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM
		{ GUID_WICPixelFormat80bppCMYKAlpha,        GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM

		{ GUID_WICPixelFormat32bppRGB,              GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM
		{ GUID_WICPixelFormat64bppRGB,              GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM
		{ GUID_WICPixelFormat64bppPRGBAHalf,        GUID_WICPixelFormat64bppRGBAHalf }, // DXGI_FORMAT_R16G16B16A16_FLOAT

		{ GUID_WICPixelFormat96bppRGBFixedPoint,   GUID_WICPixelFormat96bppRGBFloat }, // DXGI_FORMAT_R32G32B32_FLOAT

		// We don't support n-channel formats
	};


	//-------------------------------------------------------------------------------------


	BOOL WINAPI InitializeWICFactory(PINIT_ONCE, PVOID, PVOID* ifactory) noexcept
	{
		return SUCCEEDED(CoCreateInstance(
			CLSID_WICImagingFactory2,
			nullptr,
			CLSCTX_INPROC_SERVER,
			__uuidof(IWICImagingFactory2),
			ifactory)) ? TRUE : FALSE;
	}



}

namespace DirectX
{
	IWICImagingFactory2* _GetWIC() noexcept;
	// Also used by ScreenGrab

	IWICImagingFactory2* _GetWIC() noexcept
	{
		static INIT_ONCE s_initOnce = INIT_ONCE_STATIC_INIT;

		IWICImagingFactory2* factory = nullptr;
		if (!InitOnceExecuteOnce(
			&s_initOnce,
			InitializeWICFactory,
			nullptr,
			reinterpret_cast<LPVOID*>(&factory)))
		{
			return nullptr;
		}

		return factory;
	}
} // namespace DirectX


namespace
{
	//---------------------------------------------------------------------------------
	DXGI_FORMAT _WICToDXGI(const GUID& guid) noexcept
	{
		for (size_t i = 0; i < std::size(g_WICFormats); ++i)
		{
			if (memcmp(&g_WICFormats[i].wic, &guid, sizeof(GUID)) == 0)
				return g_WICFormats[i].format;
		}

		return DXGI_FORMAT_UNKNOWN;
	}

	//---------------------------------------------------------------------------------
	size_t _WICBitsPerPixel(REFGUID targetGuid) noexcept
	{
		auto pWIC = _GetWIC();
		if (!pWIC)
			return 0;

		ComPtr<IWICComponentInfo> cinfo;
		if (FAILED(pWIC->CreateComponentInfo(targetGuid, cinfo.GetAddressOf())))
			return 0;

		WICComponentType type;
		if (FAILED(cinfo->GetComponentType(&type)))
			return 0;

		if (type != WICPixelFormat)
			return 0;

		ComPtr<IWICPixelFormatInfo> pfinfo;
		if (FAILED(cinfo.As(&pfinfo)))
			return 0;

		UINT bpp;
		if (FAILED(pfinfo->GetBitsPerPixel(&bpp)))
			return 0;

		return bpp;
	}

	//--------------------------------------------------------------------------------------
	void SetDebugTextureInfo(
		_In_z_ const wchar_t* fileName,
		_In_ ID3D12Resource** texture) noexcept
	{
#if !defined(NO_D3D12_DEBUG_NAME) && ( defined(_DEBUG) || defined(PROFILE) )
		if (texture && *texture)
		{
			const wchar_t* pstrName = wcsrchr(fileName, '\\');
			if (!pstrName)
			{
				pstrName = fileName;
			}
			else
			{
				pstrName++;
			}

			(*texture)->SetName(pstrName);
		}
#else
		UNREFERENCED_PARAMETER(fileName);
		UNREFERENCED_PARAMETER(texture);
#endif
	}

	//---------------------------------------------------------------------------------

	DXGI_FORMAT GetPixelFormat(_In_ IWICBitmapFrameDecode* frame) noexcept
	{
		WICPixelFormatGUID pixelFormat;
		if (FAILED(frame->GetPixelFormat(&pixelFormat)))
			return DXGI_FORMAT_UNKNOWN;

		DXGI_FORMAT format = _WICToDXGI(pixelFormat);
		if (format == DXGI_FORMAT_UNKNOWN)
		{
			for (size_t i = 0; i < std::size(g_WICConvert); ++i)
			{
				if (memcmp(&g_WICConvert[i].source, &pixelFormat, sizeof(WICPixelFormatGUID)) == 0)
				{
					return _WICToDXGI(g_WICConvert[i].target);
				}
			}
		}

		return format;
	}

} // anonymous namespace


HRESULT CreateTextureFromWIC(
	IWICBitmapFrameDecode* frame,
	size_t maxsize,
	D3D12_RESOURCE_FLAGS resFlags,
	WIC_LOADER_FLAGS loadFlags,
	TTextureInfo& TextureInfo,
	std::vector<uint8_t>& decodedData,
	D3D12_SUBRESOURCE_DATA& subresource) noexcept
{
	UINT width, height;
	HRESULT hr = frame->GetSize(&width, &height);
	if (FAILED(hr))
		return hr;

	assert(width > 0 && height > 0);

	if (maxsize > UINT32_MAX)
		return E_INVALIDARG;

	if (!maxsize)
	{
		maxsize = size_t(D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION);
	}

	UINT twidth = width;
	UINT theight = height;
	if (loadFlags & WIC_LOADER_FIT_POW2)
	{
		LoaderHelpers::FitPowerOf2(width, height, twidth, theight, maxsize);
	}
	else if (width > maxsize || height > maxsize)
	{
		float ar = static_cast<float>(height) / static_cast<float>(width);
		if (width > height)
		{
			twidth = static_cast<UINT>(maxsize);
			theight = std::max<UINT>(1, static_cast<UINT>(static_cast<float>(maxsize) * ar));
		}
		else
		{
			theight = static_cast<UINT>(maxsize);
			twidth = std::max<UINT>(1, static_cast<UINT>(static_cast<float>(maxsize) / ar));
		}
		assert(twidth <= maxsize && theight <= maxsize);
	}

	if (loadFlags & WIC_LOADER_MAKE_SQUARE)
	{
		twidth = std::max<UINT>(twidth, theight);
		theight = twidth;
	}

	// Determine format
	WICPixelFormatGUID pixelFormat;
	hr = frame->GetPixelFormat(&pixelFormat);
	if (FAILED(hr))
		return hr;

	WICPixelFormatGUID convertGUID;
	memcpy_s(&convertGUID, sizeof(WICPixelFormatGUID), &pixelFormat, sizeof(GUID));

	size_t bpp = 0;

	DXGI_FORMAT format = _WICToDXGI(pixelFormat);
	if (format == DXGI_FORMAT_UNKNOWN)
	{
		for (size_t i = 0; i < std::size(g_WICConvert); ++i)
		{
			if (memcmp(&g_WICConvert[i].source, &pixelFormat, sizeof(WICPixelFormatGUID)) == 0)
			{
				memcpy_s(&convertGUID, sizeof(WICPixelFormatGUID), &g_WICConvert[i].target, sizeof(GUID));

				format = _WICToDXGI(g_WICConvert[i].target);
				assert(format != DXGI_FORMAT_UNKNOWN);
				bpp = _WICBitsPerPixel(convertGUID);
				break;
			}
		}

		if (format == DXGI_FORMAT_UNKNOWN)
		{
			DebugTrace("ERROR: WICTextureLoader does not support all DXGI formats (WIC GUID {%8.8lX-%4.4X-%4.4X-%2.2X%2.2X-%2.2X%2.2X%2.2X%2.2X%2.2X%2.2X}). Consider using DirectXTex.\n",
				pixelFormat.Data1, pixelFormat.Data2, pixelFormat.Data3,
				pixelFormat.Data4[0], pixelFormat.Data4[1], pixelFormat.Data4[2], pixelFormat.Data4[3],
				pixelFormat.Data4[4], pixelFormat.Data4[5], pixelFormat.Data4[6], pixelFormat.Data4[7]);
			return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
		}
	}
	else
	{
		bpp = _WICBitsPerPixel(pixelFormat);
	}

	if (loadFlags & WIC_LOADER_FORCE_RGBA32)
	{
		memcpy_s(&convertGUID, sizeof(WICPixelFormatGUID), &GUID_WICPixelFormat32bppRGBA, sizeof(GUID));
		format = DXGI_FORMAT_R8G8B8A8_UNORM;
		bpp = 32;
	}

	if (!bpp)
		return E_FAIL;

	// Handle sRGB formats
	if (loadFlags & WIC_LOADER_FORCE_SRGB)
	{
		format = LoaderHelpers::MakeSRGB(format);
	}
	else if (!(loadFlags & WIC_LOADER_IGNORE_SRGB))
	{
		ComPtr<IWICMetadataQueryReader> metareader;
		if (SUCCEEDED(frame->GetMetadataQueryReader(metareader.GetAddressOf())))
		{
			GUID containerFormat;
			if (SUCCEEDED(metareader->GetContainerFormat(&containerFormat)))
			{
				bool sRGB = false;

				PROPVARIANT value;
				PropVariantInit(&value);

				// Check for colorspace chunks
				if (memcmp(&containerFormat, &GUID_ContainerFormatPng, sizeof(GUID)) == 0)
				{
					// Check for sRGB chunk
					if (SUCCEEDED(metareader->GetMetadataByName(L"/sRGB/RenderingIntent", &value)) && value.vt == VT_UI1)
					{
						sRGB = true;
					}
					else if (SUCCEEDED(metareader->GetMetadataByName(L"/gAMA/ImageGamma", &value)) && value.vt == VT_UI4)
					{
						sRGB = (value.uintVal == 45455);
					}
					else
					{
						sRGB = (loadFlags & WIC_LOADER_SRGB_DEFAULT) != 0;
					}
				}
#if (defined(_XBOX_ONE) && defined(_TITLE)) || defined(_GAMING_XBOX)
				else if (memcmp(&containerFormat, &GUID_ContainerFormatJpeg, sizeof(GUID)) == 0)
				{
					if (SUCCEEDED(metareader->GetMetadataByName(L"/app1/ifd/exif/{ushort=40961}", &value)) && value.vt == VT_UI2)
					{
						sRGB = (value.uiVal == 1);
					}
					else
					{
						sRGB = (loadFlags & WIC_LOADER_SRGB_DEFAULT) != 0;
					}
				}
				else if (memcmp(&containerFormat, &GUID_ContainerFormatTiff, sizeof(GUID)) == 0)
				{
					if (SUCCEEDED(metareader->GetMetadataByName(L"/ifd/exif/{ushort=40961}", &value)) && value.vt == VT_UI2)
					{
						sRGB = (value.uiVal == 1);
					}
					else
					{
						sRGB = (loadFlags & WIC_LOADER_SRGB_DEFAULT) != 0;
					}
				}
#else
				else if (SUCCEEDED(metareader->GetMetadataByName(L"System.Image.ColorSpace", &value)) && value.vt == VT_UI2)
				{
					sRGB = (value.uiVal == 1);
				}
				else
				{
					sRGB = (loadFlags & WIC_LOADER_SRGB_DEFAULT) != 0;
				}
#endif

				(void)PropVariantClear(&value);

				if (sRGB)
					format = LoaderHelpers::MakeSRGB(format);
			}
		}
	}

	// Allocate memory for decoded image
	uint64_t rowBytes = (uint64_t(twidth) * uint64_t(bpp) + 7u) / 8u;
	uint64_t numBytes = rowBytes * uint64_t(theight);

	if (rowBytes > UINT32_MAX || numBytes > UINT32_MAX)
		return HRESULT_FROM_WIN32(ERROR_ARITHMETIC_OVERFLOW);

	auto rowPitch = static_cast<size_t>(rowBytes);
	auto imageSize = static_cast<size_t>(numBytes);

	decodedData.resize(imageSize);
	//decodedData.reset(new (std::nothrow) uint8_t[imageSize]);
	//if (!decodedData)
	//	return E_OUTOFMEMORY;

	// Load image data
	if (memcmp(&convertGUID, &pixelFormat, sizeof(GUID)) == 0
		&& twidth == width
		&& theight == height)
	{
		// No format conversion or resize needed
		hr = frame->CopyPixels(nullptr, static_cast<UINT>(rowPitch), static_cast<UINT>(imageSize), decodedData.data());
		if (FAILED(hr))
			return hr;
	}
	else if (twidth != width || theight != height)
	{
		// Resize
		auto pWIC = _GetWIC();
		if (!pWIC)
			return E_NOINTERFACE;

		ComPtr<IWICBitmapScaler> scaler;
		hr = pWIC->CreateBitmapScaler(scaler.GetAddressOf());
		if (FAILED(hr))
			return hr;

		hr = scaler->Initialize(frame, twidth, theight, WICBitmapInterpolationModeFant);
		if (FAILED(hr))
			return hr;

		WICPixelFormatGUID pfScaler;
		hr = scaler->GetPixelFormat(&pfScaler);
		if (FAILED(hr))
			return hr;

		if (memcmp(&convertGUID, &pfScaler, sizeof(GUID)) == 0)
		{
			// No format conversion needed
			hr = scaler->CopyPixels(nullptr, static_cast<UINT>(rowPitch), static_cast<UINT>(imageSize), decodedData.data());
			if (FAILED(hr))
				return hr;
		}
		else
		{
			ComPtr<IWICFormatConverter> FC;
			hr = pWIC->CreateFormatConverter(FC.GetAddressOf());
			if (FAILED(hr))
				return hr;

			BOOL canConvert = FALSE;
			hr = FC->CanConvert(pfScaler, convertGUID, &canConvert);
			if (FAILED(hr) || !canConvert)
			{
				return E_UNEXPECTED;
			}

			hr = FC->Initialize(scaler.Get(), convertGUID, WICBitmapDitherTypeErrorDiffusion, nullptr, 0, WICBitmapPaletteTypeMedianCut);
			if (FAILED(hr))
				return hr;

			hr = FC->CopyPixels(nullptr, static_cast<UINT>(rowPitch), static_cast<UINT>(imageSize), decodedData.data());
			if (FAILED(hr))
				return hr;
		}
	}
	else
	{
		// Format conversion but no resize
		auto pWIC = _GetWIC();
		if (!pWIC)
			return E_NOINTERFACE;

		ComPtr<IWICFormatConverter> FC;
		hr = pWIC->CreateFormatConverter(FC.GetAddressOf());
		if (FAILED(hr))
			return hr;

		BOOL canConvert = FALSE;
		hr = FC->CanConvert(pixelFormat, convertGUID, &canConvert);
		if (FAILED(hr) || !canConvert)
		{
			return E_UNEXPECTED;
		}

		hr = FC->Initialize(frame, convertGUID, WICBitmapDitherTypeErrorDiffusion, nullptr, 0, WICBitmapPaletteTypeMedianCut);
		if (FAILED(hr))
			return hr;

		hr = FC->CopyPixels(nullptr, static_cast<UINT>(rowPitch), static_cast<UINT>(imageSize), decodedData.data());
		if (FAILED(hr))
			return hr;
	}

	// Count the number of mips
	uint32_t mipCount = (loadFlags & (WIC_LOADER_MIP_AUTOGEN | WIC_LOADER_MIP_RESERVE))
		? LoaderHelpers::CountMips(twidth, theight) : 1u;


	TextureInfo.ArraySize = 1;
	TextureInfo.Format = format;
	TextureInfo.Width = twidth;
	TextureInfo.Height = theight;
	TextureInfo.Depth = 1;
	TextureInfo.MipCount = mipCount;
	TextureInfo.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;


	subresource.pData = decodedData.data();
	subresource.RowPitch = static_cast<LONG>(rowPitch);
	subresource.SlicePitch = static_cast<LONG>(imageSize);

	return hr;
}

HRESULT __cdecl DirectX::CreateWICTextureFromFile(
	_In_z_ const wchar_t* fileName,
	size_t maxsize,
	D3D12_RESOURCE_FLAGS resFlags,
	WIC_LOADER_FLAGS loadFlags,
	TTextureInfo& TextureInfo,
	D3D12_SUBRESOURCE_DATA& initData,
	std::vector<uint8_t>& decodedData)
{
	if (!fileName)
		return E_INVALIDARG;

	auto pWIC = _GetWIC();
	if (!pWIC)
		return E_NOINTERFACE;

	// Initialize WIC
	ComPtr<IWICBitmapDecoder> decoder;
	HRESULT hr = pWIC->CreateDecoderFromFilename(fileName,
		nullptr,
		GENERIC_READ,
		WICDecodeMetadataCacheOnDemand,
		decoder.GetAddressOf());
	if (FAILED(hr))
		return hr;

	ComPtr<IWICBitmapFrameDecode> frame;
	hr = decoder->GetFrame(0, frame.GetAddressOf());
	if (FAILED(hr))
		return hr;

	hr = CreateTextureFromWIC(frame.Get(), maxsize,
		resFlags, loadFlags, TextureInfo, decodedData, initData);

	return hr;
}