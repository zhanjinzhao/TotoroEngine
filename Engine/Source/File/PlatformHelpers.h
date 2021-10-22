// Modified version of DirectXTK12's source file

//--------------------------------------------------------------------------------------
// File: PlatformHelpers.h
//
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// http://go.microsoft.com/fwlink/?LinkId=248929
// http://go.microsoft.com/fwlink/?LinkID=615561
//--------------------------------------------------------------------------------------

#pragma once

#include <Windows.h>
#include <exception>
#include <stdexcept>
#include <memory>


// Helper for output debug tracing
inline void DebugTrace(_In_z_ _Printf_format_string_ const char* format, ...) noexcept
{
#ifdef _DEBUG
	va_list args;
	va_start(args, format);

	char buff[1024] = {};
	vsprintf_s(buff, format, args);
	OutputDebugStringA(buff);
	va_end(args);
#else
	UNREFERENCED_PARAMETER(format);
#endif
}

struct handle_closer { void operator()(HANDLE h) noexcept { if (h) CloseHandle(h); } };

using ScopedHandle = std::unique_ptr<void, handle_closer>;

inline HANDLE safe_handle(HANDLE h) noexcept { return (h == INVALID_HANDLE_VALUE) ? nullptr : h; }
