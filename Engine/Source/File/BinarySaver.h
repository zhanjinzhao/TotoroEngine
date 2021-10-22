#pragma once

#include <string>
#include <vector>
#include <cstdio>

class TBinarySaver
{
public:
	TBinarySaver(const std::wstring& InFilePath);

	template<typename T>
	bool Save(T Data)
	{
		return SaveArray<T>(&Data, 1);
	}

	template<typename T>
	bool SaveArray(T* Data, size_t ElementCount)
	{
		auto File = std::fstream(FilePath, std::ios::out | std::ios::binary | std::ios::app);
		if (File.is_open())
		{
			File.write((char*)Data, ElementCount * sizeof(T));
			File.close();

			return true;
		}

		return false;
	}

private:
	std::wstring FilePath;
};
