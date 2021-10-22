#pragma once

#include "Math/Math.h"
#include <string>

struct TText
{
	TText() {}
	TText(int InID, const std::string& InContent, const UIntPoint& InScreenPos, float InDuration)
		:ID(InID), Content(InContent), ScreenPos(InScreenPos), Duration(InDuration)
	{}

	std::string Content;
	UIntPoint ScreenPos;

	int ID = -1;
	float Duration = 0.0f;  // in seconds
};