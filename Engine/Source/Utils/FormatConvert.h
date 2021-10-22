#pragma once

#include <string>

class TFormatConvert
{
public:
	static std::string WStrToStr(const std::wstring& WStr)
	{
		if (WStr.empty())
		{
			return std::string();
		}

		int SizeNeeded = WideCharToMultiByte(CP_UTF8, 0, &WStr[0], (int)WStr.size(), NULL, 0, NULL, NULL);
		std::string Str(SizeNeeded, 0);
		WideCharToMultiByte(CP_UTF8, 0, &WStr[0], (int)WStr.size(), &Str[0], SizeNeeded, NULL, NULL);
		return Str;
	}

	static std::wstring StrToWStr(const std::string& Str)
	{
		if (Str.empty())
		{
			return std::wstring();
		}

		int SizeNeeded = MultiByteToWideChar(CP_UTF8, 0, &Str[0], (int)Str.size(), NULL, 0);
		std::wstring WStr(SizeNeeded, 0);
		MultiByteToWideChar(CP_UTF8, 0, &Str[0], (int)Str.size(), &WStr[0], SizeNeeded);
		return WStr;
	}
};

