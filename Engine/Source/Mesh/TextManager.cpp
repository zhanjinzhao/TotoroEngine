#include "TextManager.h"



void TTextManager::AddText(int ID, std::string Str, float Duration)
{
	assert(ID < MinGeneratedTextID);

	if (ID == -1) // Add new text with auto generated ID
	{
		if (CurrentGeneratedTextID == -1)
		{
			CurrentGeneratedTextID = MinGeneratedTextID;
		}

		ID = CurrentGeneratedTextID;
		CurrentGeneratedTextID++;

		TextList.push_back(TText(ID, Str, UIntPoint(), Duration));
	}
	else
	{
		bool bHasID = false;

		for (auto& Text : TextList)
		{
			if (ID == Text.ID) // Assign new value
			{
				Text.Content = Str;
				Text.Duration = Duration;

				bHasID = true;
				break;
			}
		}

		if (!bHasID) // Add new text with the given ID
		{
			TextList.push_back(TText(ID, Str, UIntPoint(), Duration));
		}
	}
}

void TTextManager::GetTexts(std::vector<TText>& OutTexts)
{
	UIntPoint TextPos = TextStartPos;

	for (auto& Text : TextList)
	{
		Text.ScreenPos = TextPos;

		TextPos.y += TextHeightOffset;

		OutTexts.push_back(Text);
	}
}

void TTextManager::UpdateTexts(float DeltaTime)
{
	auto Iter = TextList.begin();
	while (Iter != TextList.end())
	{
		Iter->Duration -= DeltaTime;

		if (Iter->Duration <= 0.0f) // Remove expired texts
		{
			Iter = TextList.erase(Iter);
		}
		else
		{
			Iter++;
		}
	}
}