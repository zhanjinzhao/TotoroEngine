#pragma once

#include <cstdio>
#include <debugapi.h>

class TLogger
{
public:
	static void LogToOutput(char* Text)
	{
		OutputDebugStringA(Text);
	}
};
