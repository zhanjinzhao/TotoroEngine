#pragma once

struct TColor
{
public:
	TColor() = default;

	TColor(float Value)
		:R(Value), G(Value), B(Value), A(Value)
	{}

	TColor(float InR, float InG, float InB, float InA = 1.0f)
		:R(InR), G(InG), B(InB), A(InA)
	{}

public:
	static const TColor Black;
	static const TColor White;
	static const TColor Red;
	static const TColor Green;
	static const TColor Blue;
	static const TColor Yellow;
	static const TColor Cyan;
	static const TColor Magenta;

public:
	float R = 0;
	float G = 0;
	float B = 0;
	float A = 0;
};
