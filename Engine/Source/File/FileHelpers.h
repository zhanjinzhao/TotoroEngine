#pragma once

#include <string>
#include <filesystem>
#include <Utils/FormatConvert.h>

class TFileHelpers
{
public:
	static bool IsFileExit(const std::wstring& FileName)
	{
		return std::filesystem::exists(FileName);
	}

	static std::wstring EngineDir()
	{
		std::wstring EngineDir = TFormatConvert::StrToWStr(std::string(SOLUTION_DIR)) + L"Engine/";

		return EngineDir;
	}
};
