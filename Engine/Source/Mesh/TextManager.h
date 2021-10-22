#pragma once

#include "Text.h"
#include <list>
#include <vector>

class TTextManager
{
public:
	void AddText(int ID, std::string Str, float Duration);

	void GetTexts(std::vector<TText>& OutTexts);

	void UpdateTexts(float DeltaTime);

private:
	std::list<TText> TextList;

	int MinGeneratedTextID = 10000;

	int CurrentGeneratedTextID = -1;

	UIntPoint TextStartPos = UIntPoint(30, 30);

	uint32_t TextHeightOffset = 20;
};
