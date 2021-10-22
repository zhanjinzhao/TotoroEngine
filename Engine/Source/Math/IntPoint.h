#pragma once

#include <DirectXMath.h>

// 2D Vector
// 32 bit signed integer components
struct IntPoint : public DirectX::XMINT2
{
	IntPoint() noexcept : DirectX::XMINT2(0, 0) {}
	constexpr explicit IntPoint(int32_t ix, int32_t iy) noexcept : DirectX::XMINT2(ix, iy) {}

	IntPoint(const IntPoint&) = default;
	IntPoint& operator=(const IntPoint&) = default;

	IntPoint(IntPoint&&) = default;
	IntPoint& operator=(IntPoint&&) = default;
};

// 2D Vector
// 32 bit unsigned integer components
struct UIntPoint : public DirectX::XMUINT2
{
	UIntPoint() noexcept : DirectX::XMUINT2(0, 0) {}
	constexpr explicit UIntPoint(uint32_t ix, uint32_t iy) noexcept : DirectX::XMUINT2(ix, iy) {}

	UIntPoint(const UIntPoint&) = default;
	UIntPoint& operator=(const UIntPoint&) = default;

	UIntPoint(UIntPoint&&) = default;
	UIntPoint& operator=(UIntPoint&&) = default;
};

inline UIntPoint operator* (const UIntPoint& U, float S) noexcept
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadUInt2(&U);
	XMVECTOR X = XMVectorScale(v1, S);
	UIntPoint R;
	XMStoreUInt2(&R, X);
	return R;
}

inline UIntPoint operator+ (const UIntPoint& U1, const UIntPoint& U2) noexcept
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadUInt2(&U1);
	XMVECTOR v2 = XMLoadUInt2(&U2);
	XMVECTOR X = XMVectorAdd(v1, v2);
	UIntPoint R;
	XMStoreUInt2(&R, X);
	return R;
}
