//--------------------------------------------------------------------------------------
// File: LoaderHelpers.h
//
// Helper functions for texture loaders and screen grabber
//
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// http://go.microsoft.com/fwlink/?LinkId=248929
// http://go.microsoft.com/fwlink/?LinkID=615561
//--------------------------------------------------------------------------------------

#pragma once

//#include "DDS.h"
//#include "DDSTextureLoader.h"
//#include "PlatformHelpers.h"

#include <stdint.h>
#include "D3D12/d3dx12.h"


namespace DirectX
{
    namespace LoaderHelpers
    {
		//--------------------------------------------------------------------------------------
		inline bool IsCompressed(_In_ DXGI_FORMAT fmt) noexcept
		{
			switch (fmt)
			{
			case DXGI_FORMAT_BC1_TYPELESS:
			case DXGI_FORMAT_BC1_UNORM:
			case DXGI_FORMAT_BC1_UNORM_SRGB:
			case DXGI_FORMAT_BC2_TYPELESS:
			case DXGI_FORMAT_BC2_UNORM:
			case DXGI_FORMAT_BC2_UNORM_SRGB:
			case DXGI_FORMAT_BC3_TYPELESS:
			case DXGI_FORMAT_BC3_UNORM:
			case DXGI_FORMAT_BC3_UNORM_SRGB:
			case DXGI_FORMAT_BC4_TYPELESS:
			case DXGI_FORMAT_BC4_UNORM:
			case DXGI_FORMAT_BC4_SNORM:
			case DXGI_FORMAT_BC5_TYPELESS:
			case DXGI_FORMAT_BC5_UNORM:
			case DXGI_FORMAT_BC5_SNORM:
			case DXGI_FORMAT_BC6H_TYPELESS:
			case DXGI_FORMAT_BC6H_UF16:
			case DXGI_FORMAT_BC6H_SF16:
			case DXGI_FORMAT_BC7_TYPELESS:
			case DXGI_FORMAT_BC7_UNORM:
			case DXGI_FORMAT_BC7_UNORM_SRGB:
				return true;

			default:
				return false;
			}
		}

		//--------------------------------------------------------------------------------------
		// Return the BPP for a particular format
		//--------------------------------------------------------------------------------------
		inline size_t BitsPerPixel(_In_ DXGI_FORMAT fmt) noexcept
		{
			switch (fmt)
			{
			case DXGI_FORMAT_R32G32B32A32_TYPELESS:
			case DXGI_FORMAT_R32G32B32A32_FLOAT:
			case DXGI_FORMAT_R32G32B32A32_UINT:
			case DXGI_FORMAT_R32G32B32A32_SINT:
				return 128;

			case DXGI_FORMAT_R32G32B32_TYPELESS:
			case DXGI_FORMAT_R32G32B32_FLOAT:
			case DXGI_FORMAT_R32G32B32_UINT:
			case DXGI_FORMAT_R32G32B32_SINT:
				return 96;

			case DXGI_FORMAT_R16G16B16A16_TYPELESS:
			case DXGI_FORMAT_R16G16B16A16_FLOAT:
			case DXGI_FORMAT_R16G16B16A16_UNORM:
			case DXGI_FORMAT_R16G16B16A16_UINT:
			case DXGI_FORMAT_R16G16B16A16_SNORM:
			case DXGI_FORMAT_R16G16B16A16_SINT:
			case DXGI_FORMAT_R32G32_TYPELESS:
			case DXGI_FORMAT_R32G32_FLOAT:
			case DXGI_FORMAT_R32G32_UINT:
			case DXGI_FORMAT_R32G32_SINT:
			case DXGI_FORMAT_R32G8X24_TYPELESS:
			case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
			case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
			case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
			case DXGI_FORMAT_Y416:
			case DXGI_FORMAT_Y210:
			case DXGI_FORMAT_Y216:
				return 64;

			case DXGI_FORMAT_R10G10B10A2_TYPELESS:
			case DXGI_FORMAT_R10G10B10A2_UNORM:
			case DXGI_FORMAT_R10G10B10A2_UINT:
			case DXGI_FORMAT_R11G11B10_FLOAT:
			case DXGI_FORMAT_R8G8B8A8_TYPELESS:
			case DXGI_FORMAT_R8G8B8A8_UNORM:
			case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
			case DXGI_FORMAT_R8G8B8A8_UINT:
			case DXGI_FORMAT_R8G8B8A8_SNORM:
			case DXGI_FORMAT_R8G8B8A8_SINT:
			case DXGI_FORMAT_R16G16_TYPELESS:
			case DXGI_FORMAT_R16G16_FLOAT:
			case DXGI_FORMAT_R16G16_UNORM:
			case DXGI_FORMAT_R16G16_UINT:
			case DXGI_FORMAT_R16G16_SNORM:
			case DXGI_FORMAT_R16G16_SINT:
			case DXGI_FORMAT_R32_TYPELESS:
			case DXGI_FORMAT_D32_FLOAT:
			case DXGI_FORMAT_R32_FLOAT:
			case DXGI_FORMAT_R32_UINT:
			case DXGI_FORMAT_R32_SINT:
			case DXGI_FORMAT_R24G8_TYPELESS:
			case DXGI_FORMAT_D24_UNORM_S8_UINT:
			case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
			case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
			case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
			case DXGI_FORMAT_R8G8_B8G8_UNORM:
			case DXGI_FORMAT_G8R8_G8B8_UNORM:
			case DXGI_FORMAT_B8G8R8A8_UNORM:
			case DXGI_FORMAT_B8G8R8X8_UNORM:
			case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
			case DXGI_FORMAT_B8G8R8A8_TYPELESS:
			case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
			case DXGI_FORMAT_B8G8R8X8_TYPELESS:
			case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
			case DXGI_FORMAT_AYUV:
			case DXGI_FORMAT_Y410:
			case DXGI_FORMAT_YUY2:
#if (defined(_XBOX_ONE) && defined(_TITLE)) || defined(_GAMING_XBOX)
			case DXGI_FORMAT_R10G10B10_7E3_A2_FLOAT:
			case DXGI_FORMAT_R10G10B10_6E4_A2_FLOAT:
			case DXGI_FORMAT_R10G10B10_SNORM_A2_UNORM:
#endif
				return 32;

			case DXGI_FORMAT_P010:
			case DXGI_FORMAT_P016:
#if (_WIN32_WINNT >= _WIN32_WINNT_WIN10)
			case DXGI_FORMAT_V408:
#endif
#if (defined(_XBOX_ONE) && defined(_TITLE)) || defined(_GAMING_XBOX)
			case DXGI_FORMAT_D16_UNORM_S8_UINT:
			case DXGI_FORMAT_R16_UNORM_X8_TYPELESS:
			case DXGI_FORMAT_X16_TYPELESS_G8_UINT:
#endif
				return 24;

			case DXGI_FORMAT_R8G8_TYPELESS:
			case DXGI_FORMAT_R8G8_UNORM:
			case DXGI_FORMAT_R8G8_UINT:
			case DXGI_FORMAT_R8G8_SNORM:
			case DXGI_FORMAT_R8G8_SINT:
			case DXGI_FORMAT_R16_TYPELESS:
			case DXGI_FORMAT_R16_FLOAT:
			case DXGI_FORMAT_D16_UNORM:
			case DXGI_FORMAT_R16_UNORM:
			case DXGI_FORMAT_R16_UINT:
			case DXGI_FORMAT_R16_SNORM:
			case DXGI_FORMAT_R16_SINT:
			case DXGI_FORMAT_B5G6R5_UNORM:
			case DXGI_FORMAT_B5G5R5A1_UNORM:
			case DXGI_FORMAT_A8P8:
			case DXGI_FORMAT_B4G4R4A4_UNORM:
#if (_WIN32_WINNT >= _WIN32_WINNT_WIN10)
			case DXGI_FORMAT_P208:
			case DXGI_FORMAT_V208:
#endif
				return 16;

			case DXGI_FORMAT_NV12:
			case DXGI_FORMAT_420_OPAQUE:
			case DXGI_FORMAT_NV11:
				return 12;

			case DXGI_FORMAT_R8_TYPELESS:
			case DXGI_FORMAT_R8_UNORM:
			case DXGI_FORMAT_R8_UINT:
			case DXGI_FORMAT_R8_SNORM:
			case DXGI_FORMAT_R8_SINT:
			case DXGI_FORMAT_A8_UNORM:
			case DXGI_FORMAT_BC2_TYPELESS:
			case DXGI_FORMAT_BC2_UNORM:
			case DXGI_FORMAT_BC2_UNORM_SRGB:
			case DXGI_FORMAT_BC3_TYPELESS:
			case DXGI_FORMAT_BC3_UNORM:
			case DXGI_FORMAT_BC3_UNORM_SRGB:
			case DXGI_FORMAT_BC5_TYPELESS:
			case DXGI_FORMAT_BC5_UNORM:
			case DXGI_FORMAT_BC5_SNORM:
			case DXGI_FORMAT_BC6H_TYPELESS:
			case DXGI_FORMAT_BC6H_UF16:
			case DXGI_FORMAT_BC6H_SF16:
			case DXGI_FORMAT_BC7_TYPELESS:
			case DXGI_FORMAT_BC7_UNORM:
			case DXGI_FORMAT_BC7_UNORM_SRGB:
			case DXGI_FORMAT_AI44:
			case DXGI_FORMAT_IA44:
			case DXGI_FORMAT_P8:
#if (defined(_XBOX_ONE) && defined(_TITLE)) || defined(_GAMING_XBOX)
			case DXGI_FORMAT_R4G4_UNORM:
#endif
				return 8;

			case DXGI_FORMAT_R1_UNORM:
				return 1;

			case DXGI_FORMAT_BC1_TYPELESS:
			case DXGI_FORMAT_BC1_UNORM:
			case DXGI_FORMAT_BC1_UNORM_SRGB:
			case DXGI_FORMAT_BC4_TYPELESS:
			case DXGI_FORMAT_BC4_UNORM:
			case DXGI_FORMAT_BC4_SNORM:
				return 4;

			case DXGI_FORMAT_UNKNOWN:
			case DXGI_FORMAT_FORCE_UINT:
			default:
				return 0;
			}
		}


		//--------------------------------------------------------------------------------------
		// Get surface information for a particular format
		//--------------------------------------------------------------------------------------
		inline HRESULT GetSurfaceInfo(
			_In_ size_t width,
			_In_ size_t height,
			_In_ DXGI_FORMAT fmt,
			_Out_opt_ size_t* outNumBytes,
			_Out_opt_ size_t* outRowBytes,
			_Out_opt_ size_t* outNumRows) noexcept
		{
			uint64_t numBytes = 0;
			uint64_t rowBytes = 0;
			uint64_t numRows = 0;

			bool bc = false;
			bool packed = false;
			bool planar = false;
			size_t bpe = 0;
			switch (fmt)
			{
			case DXGI_FORMAT_BC1_TYPELESS:
			case DXGI_FORMAT_BC1_UNORM:
			case DXGI_FORMAT_BC1_UNORM_SRGB:
			case DXGI_FORMAT_BC4_TYPELESS:
			case DXGI_FORMAT_BC4_UNORM:
			case DXGI_FORMAT_BC4_SNORM:
				bc = true;
				bpe = 8;
				break;

			case DXGI_FORMAT_BC2_TYPELESS:
			case DXGI_FORMAT_BC2_UNORM:
			case DXGI_FORMAT_BC2_UNORM_SRGB:
			case DXGI_FORMAT_BC3_TYPELESS:
			case DXGI_FORMAT_BC3_UNORM:
			case DXGI_FORMAT_BC3_UNORM_SRGB:
			case DXGI_FORMAT_BC5_TYPELESS:
			case DXGI_FORMAT_BC5_UNORM:
			case DXGI_FORMAT_BC5_SNORM:
			case DXGI_FORMAT_BC6H_TYPELESS:
			case DXGI_FORMAT_BC6H_UF16:
			case DXGI_FORMAT_BC6H_SF16:
			case DXGI_FORMAT_BC7_TYPELESS:
			case DXGI_FORMAT_BC7_UNORM:
			case DXGI_FORMAT_BC7_UNORM_SRGB:
				bc = true;
				bpe = 16;
				break;

			case DXGI_FORMAT_R8G8_B8G8_UNORM:
			case DXGI_FORMAT_G8R8_G8B8_UNORM:
			case DXGI_FORMAT_YUY2:
				packed = true;
				bpe = 4;
				break;

			case DXGI_FORMAT_Y210:
			case DXGI_FORMAT_Y216:
				packed = true;
				bpe = 8;
				break;

			case DXGI_FORMAT_NV12:
			case DXGI_FORMAT_420_OPAQUE:
#if (_WIN32_WINNT >= _WIN32_WINNT_WIN10)
			case DXGI_FORMAT_P208:
#endif
				planar = true;
				bpe = 2;
				break;

			case DXGI_FORMAT_P010:
			case DXGI_FORMAT_P016:
				planar = true;
				bpe = 4;
				break;

#if (defined(_XBOX_ONE) && defined(_TITLE)) || defined(_GAMING_XBOX)

			case DXGI_FORMAT_D16_UNORM_S8_UINT:
			case DXGI_FORMAT_R16_UNORM_X8_TYPELESS:
			case DXGI_FORMAT_X16_TYPELESS_G8_UINT:
				planar = true;
				bpe = 4;
				break;

#endif

			default:
				break;
			}

			if (bc)
			{
				uint64_t numBlocksWide = 0;
				if (width > 0)
				{
					numBlocksWide = std::max<uint64_t>(1u, (uint64_t(width) + 3u) / 4u);
				}
				uint64_t numBlocksHigh = 0;
				if (height > 0)
				{
					numBlocksHigh = std::max<uint64_t>(1u, (uint64_t(height) + 3u) / 4u);
				}
				rowBytes = numBlocksWide * bpe;
				numRows = numBlocksHigh;
				numBytes = rowBytes * numBlocksHigh;
			}
			else if (packed)
			{
				rowBytes = ((uint64_t(width) + 1u) >> 1) * bpe;
				numRows = uint64_t(height);
				numBytes = rowBytes * height;
			}
			else if (fmt == DXGI_FORMAT_NV11)
			{
				rowBytes = ((uint64_t(width) + 3u) >> 2) * 4u;
				numRows = uint64_t(height) * 2u; // Direct3D makes this simplifying assumption, although it is larger than the 4:1:1 data
				numBytes = rowBytes * numRows;
			}
			else if (planar)
			{
				rowBytes = ((uint64_t(width) + 1u) >> 1) * bpe;
				numBytes = (rowBytes * uint64_t(height)) + ((rowBytes * uint64_t(height) + 1u) >> 1);
				numRows = height + ((uint64_t(height) + 1u) >> 1);
			}
			else
			{
				size_t bpp = BitsPerPixel(fmt);
				if (!bpp)
					return E_INVALIDARG;

				rowBytes = (uint64_t(width) * bpp + 7u) / 8u; // round up to nearest byte
				numRows = uint64_t(height);
				numBytes = rowBytes * height;
			}

#if defined(_M_IX86) || defined(_M_ARM) || defined(_M_HYBRID_X86_ARM64)
			static_assert(sizeof(size_t) == 4, "Not a 32-bit platform!");
			if (numBytes > UINT32_MAX || rowBytes > UINT32_MAX || numRows > UINT32_MAX)
				return HRESULT_FROM_WIN32(ERROR_ARITHMETIC_OVERFLOW);
#else
			static_assert(sizeof(size_t) == 8, "Not a 64-bit platform!");
#endif

			if (outNumBytes)
			{
				*outNumBytes = static_cast<size_t>(numBytes);
			}
			if (outRowBytes)
			{
				*outRowBytes = static_cast<size_t>(rowBytes);
			}
			if (outNumRows)
			{
				*outNumRows = static_cast<size_t>(numRows);
			}

			return S_OK;
		}

		//--------------------------------------------------------------------------------------
		inline DXGI_FORMAT EnsureNotTypeless(DXGI_FORMAT fmt) noexcept
		{
			// Assumes UNORM or FLOAT; doesn't use UINT or SINT
			switch (fmt)
			{
			case DXGI_FORMAT_R32G32B32A32_TYPELESS: return DXGI_FORMAT_R32G32B32A32_FLOAT;
			case DXGI_FORMAT_R32G32B32_TYPELESS:    return DXGI_FORMAT_R32G32B32_FLOAT;
			case DXGI_FORMAT_R16G16B16A16_TYPELESS: return DXGI_FORMAT_R16G16B16A16_UNORM;
			case DXGI_FORMAT_R32G32_TYPELESS:       return DXGI_FORMAT_R32G32_FLOAT;
			case DXGI_FORMAT_R10G10B10A2_TYPELESS:  return DXGI_FORMAT_R10G10B10A2_UNORM;
			case DXGI_FORMAT_R8G8B8A8_TYPELESS:     return DXGI_FORMAT_R8G8B8A8_UNORM;
			case DXGI_FORMAT_R16G16_TYPELESS:       return DXGI_FORMAT_R16G16_UNORM;
			case DXGI_FORMAT_R32_TYPELESS:          return DXGI_FORMAT_R32_FLOAT;
			case DXGI_FORMAT_R8G8_TYPELESS:         return DXGI_FORMAT_R8G8_UNORM;
			case DXGI_FORMAT_R16_TYPELESS:          return DXGI_FORMAT_R16_UNORM;
			case DXGI_FORMAT_R8_TYPELESS:           return DXGI_FORMAT_R8_UNORM;
			case DXGI_FORMAT_BC1_TYPELESS:          return DXGI_FORMAT_BC1_UNORM;
			case DXGI_FORMAT_BC2_TYPELESS:          return DXGI_FORMAT_BC2_UNORM;
			case DXGI_FORMAT_BC3_TYPELESS:          return DXGI_FORMAT_BC3_UNORM;
			case DXGI_FORMAT_BC4_TYPELESS:          return DXGI_FORMAT_BC4_UNORM;
			case DXGI_FORMAT_BC5_TYPELESS:          return DXGI_FORMAT_BC5_UNORM;
			case DXGI_FORMAT_B8G8R8A8_TYPELESS:     return DXGI_FORMAT_B8G8R8A8_UNORM;
			case DXGI_FORMAT_B8G8R8X8_TYPELESS:     return DXGI_FORMAT_B8G8R8X8_UNORM;
			case DXGI_FORMAT_BC7_TYPELESS:          return DXGI_FORMAT_BC7_UNORM;
			default:                                return fmt;
			}
		}

		//--------------------------------------------------------------------------------------
		class auto_delete_file
		{
		public:
			auto_delete_file(HANDLE hFile) noexcept : m_handle(hFile) {}

			auto_delete_file(const auto_delete_file&) = delete;
			auto_delete_file& operator=(const auto_delete_file&) = delete;

			auto_delete_file(const auto_delete_file&&) = delete;
			auto_delete_file& operator=(const auto_delete_file&&) = delete;

			~auto_delete_file()
			{
				if (m_handle)
				{
					FILE_DISPOSITION_INFO info = {};
					info.DeleteFile = TRUE;
					(void)SetFileInformationByHandle(m_handle, FileDispositionInfo, &info, sizeof(info));
				}
			}

			void clear() noexcept { m_handle = nullptr; }

		private:
			HANDLE m_handle;
		};

		//--------------------------------------------------------------------------------------
		inline DXGI_FORMAT MakeSRGB(_In_ DXGI_FORMAT format) noexcept
		{
			switch (format)
			{
			case DXGI_FORMAT_R8G8B8A8_UNORM:
				return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

			case DXGI_FORMAT_BC1_UNORM:
				return DXGI_FORMAT_BC1_UNORM_SRGB;

			case DXGI_FORMAT_BC2_UNORM:
				return DXGI_FORMAT_BC2_UNORM_SRGB;

			case DXGI_FORMAT_BC3_UNORM:
				return DXGI_FORMAT_BC3_UNORM_SRGB;

			case DXGI_FORMAT_B8G8R8A8_UNORM:
				return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;

			case DXGI_FORMAT_B8G8R8X8_UNORM:
				return DXGI_FORMAT_B8G8R8X8_UNORM_SRGB;

			case DXGI_FORMAT_BC7_UNORM:
				return DXGI_FORMAT_BC7_UNORM_SRGB;

			default:
				return format;
			}
		}

		inline uint32_t CountMips(uint32_t width, uint32_t height) noexcept
		{
			if (width == 0 || height == 0)
				return 0;

			uint32_t count = 1;
			while (width > 1 || height > 1)
			{
				width >>= 1;
				height >>= 1;
				count++;
			}
			return count;
		}

        inline void FitPowerOf2(UINT origx, UINT origy, UINT& targetx, UINT& targety, size_t maxsize)
        {
            float origAR = float(origx) / float(origy);

            if (origx > origy)
            {
                size_t x;
                for (x = maxsize; x > 1; x >>= 1) { if (x <= targetx) break; }
                targetx = UINT(x);

                float bestScore = FLT_MAX;
                for (size_t y = maxsize; y > 0; y >>= 1)
                {
                    float score = fabsf((float(x) / float(y)) - origAR);
                    if (score < bestScore)
                    {
                        bestScore = score;
                        targety = UINT(y);
                    }
                }
            }
            else
            {
                size_t y;
                for (y = maxsize; y > 1; y >>= 1) { if (y <= targety) break; }
                targety = UINT(y);

                float bestScore = FLT_MAX;
                for (size_t x = maxsize; x > 0; x >>= 1)
                {
                    float score = fabsf((float(x) / float(y)) - origAR);
                    if (score < bestScore)
                    {
                        bestScore = score;
                        targetx = UINT(x);
                    }
                }
            }
        }
    }
}
