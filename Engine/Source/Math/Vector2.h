// Modified version of DirectXTK12's source file

//-------------------------------------------------------------------------------------
// SimpleMath.h -- Simplified C++ Math wrapper for DirectXMath
//
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// http://go.microsoft.com/fwlink/?LinkId=248929
// http://go.microsoft.com/fwlink/?LinkID=615561
//-------------------------------------------------------------------------------------

#pragma once

#include <DirectXMath.h>

// 2D vector
struct TVector2 : public DirectX::XMFLOAT2
{
	TVector2() noexcept : XMFLOAT2(0.f, 0.f) {}
	constexpr explicit TVector2(float ix) noexcept : XMFLOAT2(ix, ix) {}
	constexpr TVector2(float ix, float iy) noexcept : XMFLOAT2(ix, iy) {}
	explicit TVector2(_In_reads_(2) const float* pArray) noexcept : XMFLOAT2(pArray) {}
	TVector2(DirectX::FXMVECTOR V) noexcept { XMStoreFloat2(this, V); }
	TVector2(const XMFLOAT2& V) noexcept { this->x = V.x; this->y = V.y; }
	explicit TVector2(const DirectX::XMVECTORF32& F) noexcept { this->x = F.f[0]; this->y = F.f[1]; }

	TVector2(const TVector2&) = default;
	TVector2& operator=(const TVector2&) = default;

	TVector2(TVector2&&) = default;
	TVector2& operator=(TVector2&&) = default;

	operator DirectX::XMVECTOR() const noexcept { return XMLoadFloat2(this); }

	// Comparison operators
	bool operator == (const TVector2& V) const noexcept;
	bool operator != (const TVector2& V) const noexcept;

	// Assignment operators
	TVector2& operator= (const DirectX::XMVECTORF32& F) noexcept { x = F.f[0]; y = F.f[1]; return *this; }
	TVector2& operator+= (const TVector2& V) noexcept;
	TVector2& operator-= (const TVector2& V) noexcept;
	TVector2& operator*= (const TVector2& V) noexcept;
	TVector2& operator*= (float S) noexcept;
	TVector2& operator/= (float S) noexcept;

	// Unary operators
	TVector2 operator+ () const noexcept { return *this; }
	TVector2 operator- () const noexcept { return TVector2(-x, -y); }

	// Vector operations
	bool InBounds(const TVector2& Bounds) const noexcept;

	float Length() const noexcept;
	float LengthSquared() const noexcept;

	float Dot(const TVector2& V) const noexcept;
	void Cross(const TVector2& V, TVector2& result) const noexcept;
	TVector2 Cross(const TVector2& V) const noexcept;

	void Normalize() noexcept;
	void Normalize(TVector2& result) const noexcept;

	void Clamp(const TVector2& vmin, const TVector2& vmax) noexcept;
	void Clamp(const TVector2& vmin, const TVector2& vmax, TVector2& result) const noexcept;

	// Static functions
	static float Distance(const TVector2& v1, const TVector2& v2) noexcept;
	static float DistanceSquared(const TVector2& v1, const TVector2& v2) noexcept;

	static void Min(const TVector2& v1, const TVector2& v2, TVector2& result) noexcept;
	static TVector2 Min(const TVector2& v1, const TVector2& v2) noexcept;

	static void Max(const TVector2& v1, const TVector2& v2, TVector2& result) noexcept;
	static TVector2 Max(const TVector2& v1, const TVector2& v2) noexcept;

	static void Lerp(const TVector2& v1, const TVector2& v2, float t, TVector2& result) noexcept;
	static TVector2 Lerp(const TVector2& v1, const TVector2& v2, float t) noexcept;

	static void SmoothStep(const TVector2& v1, const TVector2& v2, float t, TVector2& result) noexcept;
	static TVector2 SmoothStep(const TVector2& v1, const TVector2& v2, float t) noexcept;

	static void Barycentric(const TVector2& v1, const TVector2& v2, const TVector2& v3, float f, float g, TVector2& result) noexcept;
	static TVector2 Barycentric(const TVector2& v1, const TVector2& v2, const TVector2& v3, float f, float g) noexcept;

	static void CatmullRom(const TVector2& v1, const TVector2& v2, const TVector2& v3, const TVector2& v4, float t, TVector2& result) noexcept;
	static TVector2 CatmullRom(const TVector2& v1, const TVector2& v2, const TVector2& v3, const TVector2& v4, float t) noexcept;

	static void Hermite(const TVector2& v1, const TVector2& t1, const TVector2& v2, const TVector2& t2, float t, TVector2& result) noexcept;
	static TVector2 Hermite(const TVector2& v1, const TVector2& t1, const TVector2& v2, const TVector2& t2, float t) noexcept;

	static void Reflect(const TVector2& ivec, const TVector2& nvec, TVector2& result) noexcept;
	static TVector2 Reflect(const TVector2& ivec, const TVector2& nvec) noexcept;

	static void Refract(const TVector2& ivec, const TVector2& nvec, float refractionIndex, TVector2& result) noexcept;
	static TVector2 Refract(const TVector2& ivec, const TVector2& nvec, float refractionIndex) noexcept;

	// Constants
	static const TVector2 Zero;
	static const TVector2 One;
	static const TVector2 UnitX;
	static const TVector2 UnitY;
};

//------------------------------------------------------------------------------
// Comparision operators
//------------------------------------------------------------------------------

inline bool TVector2::operator == (const TVector2& V) const noexcept
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat2(this);
	XMVECTOR v2 = XMLoadFloat2(&V);
	return XMVector2Equal(v1, v2);
}

inline bool TVector2::operator != (const TVector2& V) const noexcept
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat2(this);
	XMVECTOR v2 = XMLoadFloat2(&V);
	return XMVector2NotEqual(v1, v2);
}

//------------------------------------------------------------------------------
// Assignment operators
//------------------------------------------------------------------------------

inline TVector2& TVector2::operator+= (const TVector2& V) noexcept
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat2(this);
	XMVECTOR v2 = XMLoadFloat2(&V);
	XMVECTOR X = XMVectorAdd(v1, v2);
	XMStoreFloat2(this, X);
	return *this;
}

inline TVector2& TVector2::operator-= (const TVector2& V) noexcept
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat2(this);
	XMVECTOR v2 = XMLoadFloat2(&V);
	XMVECTOR X = XMVectorSubtract(v1, v2);
	XMStoreFloat2(this, X);
	return *this;
}

inline TVector2& TVector2::operator*= (const TVector2& V) noexcept
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat2(this);
	XMVECTOR v2 = XMLoadFloat2(&V);
	XMVECTOR X = XMVectorMultiply(v1, v2);
	XMStoreFloat2(this, X);
	return *this;
}

inline TVector2& TVector2::operator*= (float S) noexcept
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat2(this);
	XMVECTOR X = XMVectorScale(v1, S);
	XMStoreFloat2(this, X);
	return *this;
}

inline TVector2& TVector2::operator/= (float S) noexcept
{
	using namespace DirectX;
	assert(S != 0.0f);
	XMVECTOR v1 = XMLoadFloat2(this);
	XMVECTOR X = XMVectorScale(v1, 1.f / S);
	XMStoreFloat2(this, X);
	return *this;
}

//------------------------------------------------------------------------------
// Binary operators
//------------------------------------------------------------------------------

inline TVector2 operator+ (const TVector2& V1, const TVector2& V2) noexcept
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat2(&V1);
	XMVECTOR v2 = XMLoadFloat2(&V2);
	XMVECTOR X = XMVectorAdd(v1, v2);
	TVector2 R;
	XMStoreFloat2(&R, X);
	return R;
}

inline TVector2 operator- (const TVector2& V1, const TVector2& V2) noexcept
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat2(&V1);
	XMVECTOR v2 = XMLoadFloat2(&V2);
	XMVECTOR X = XMVectorSubtract(v1, v2);
	TVector2 R;
	XMStoreFloat2(&R, X);
	return R;
}

inline TVector2 operator* (const TVector2& V1, const TVector2& V2) noexcept
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat2(&V1);
	XMVECTOR v2 = XMLoadFloat2(&V2);
	XMVECTOR X = XMVectorMultiply(v1, v2);
	TVector2 R;
	XMStoreFloat2(&R, X);
	return R;
}

inline TVector2 operator* (const TVector2& V, float S) noexcept
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat2(&V);
	XMVECTOR X = XMVectorScale(v1, S);
	TVector2 R;
	XMStoreFloat2(&R, X);
	return R;
}

inline TVector2 operator/ (const TVector2& V1, const TVector2& V2) noexcept
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat2(&V1);
	XMVECTOR v2 = XMLoadFloat2(&V2);
	XMVECTOR X = XMVectorDivide(v1, v2);
	TVector2 R;
	XMStoreFloat2(&R, X);
	return R;
}

inline TVector2 operator/ (const TVector2& V, float S) noexcept
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat2(&V);
	XMVECTOR X = XMVectorScale(v1, 1.f / S);
	TVector2 R;
	XMStoreFloat2(&R, X);
	return R;
}

inline TVector2 operator* (float S, const TVector2& V) noexcept
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat2(&V);
	XMVECTOR X = XMVectorScale(v1, S);
	TVector2 R;
	XMStoreFloat2(&R, X);
	return R;
}

//------------------------------------------------------------------------------
// Vector operations
//------------------------------------------------------------------------------

inline bool TVector2::InBounds(const TVector2& Bounds) const noexcept
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat2(this);
	XMVECTOR v2 = XMLoadFloat2(&Bounds);
	return XMVector2InBounds(v1, v2);
}

inline float TVector2::Length() const noexcept
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat2(this);
	XMVECTOR X = XMVector2Length(v1);
	return XMVectorGetX(X);
}

inline float TVector2::LengthSquared() const noexcept
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat2(this);
	XMVECTOR X = XMVector2LengthSq(v1);
	return XMVectorGetX(X);
}

inline float TVector2::Dot(const TVector2& V) const noexcept
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat2(this);
	XMVECTOR v2 = XMLoadFloat2(&V);
	XMVECTOR X = XMVector2Dot(v1, v2);
	return XMVectorGetX(X);
}

inline void TVector2::Cross(const TVector2& V, TVector2& result) const noexcept
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat2(this);
	XMVECTOR v2 = XMLoadFloat2(&V);
	XMVECTOR R = XMVector2Cross(v1, v2);
	XMStoreFloat2(&result, R);
}

inline TVector2 TVector2::Cross(const TVector2& V) const noexcept
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat2(this);
	XMVECTOR v2 = XMLoadFloat2(&V);
	XMVECTOR R = XMVector2Cross(v1, v2);

	TVector2 result;
	XMStoreFloat2(&result, R);
	return result;
}

inline void TVector2::Normalize() noexcept
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat2(this);
	XMVECTOR X = XMVector2Normalize(v1);
	XMStoreFloat2(this, X);
}

inline void TVector2::Normalize(TVector2& result) const noexcept
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat2(this);
	XMVECTOR X = XMVector2Normalize(v1);
	XMStoreFloat2(&result, X);
}

inline void TVector2::Clamp(const TVector2& vmin, const TVector2& vmax) noexcept
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat2(this);
	XMVECTOR v2 = XMLoadFloat2(&vmin);
	XMVECTOR v3 = XMLoadFloat2(&vmax);
	XMVECTOR X = XMVectorClamp(v1, v2, v3);
	XMStoreFloat2(this, X);
}

inline void TVector2::Clamp(const TVector2& vmin, const TVector2& vmax, TVector2& result) const noexcept
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat2(this);
	XMVECTOR v2 = XMLoadFloat2(&vmin);
	XMVECTOR v3 = XMLoadFloat2(&vmax);
	XMVECTOR X = XMVectorClamp(v1, v2, v3);
	XMStoreFloat2(&result, X);
}

//------------------------------------------------------------------------------
// Static functions
//------------------------------------------------------------------------------

inline float TVector2::Distance(const TVector2& v1, const TVector2& v2) noexcept
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat2(&v1);
	XMVECTOR x2 = XMLoadFloat2(&v2);
	XMVECTOR V = XMVectorSubtract(x2, x1);
	XMVECTOR X = XMVector2Length(V);
	return XMVectorGetX(X);
}

inline float TVector2::DistanceSquared(const TVector2& v1, const TVector2& v2) noexcept
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat2(&v1);
	XMVECTOR x2 = XMLoadFloat2(&v2);
	XMVECTOR V = XMVectorSubtract(x2, x1);
	XMVECTOR X = XMVector2LengthSq(V);
	return XMVectorGetX(X);
}

inline void TVector2::Min(const TVector2& v1, const TVector2& v2, TVector2& result) noexcept
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat2(&v1);
	XMVECTOR x2 = XMLoadFloat2(&v2);
	XMVECTOR X = XMVectorMin(x1, x2);
	XMStoreFloat2(&result, X);
}

inline TVector2 TVector2::Min(const TVector2& v1, const TVector2& v2) noexcept
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat2(&v1);
	XMVECTOR x2 = XMLoadFloat2(&v2);
	XMVECTOR X = XMVectorMin(x1, x2);

	TVector2 result;
	XMStoreFloat2(&result, X);
	return result;
}

inline void TVector2::Max(const TVector2& v1, const TVector2& v2, TVector2& result) noexcept
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat2(&v1);
	XMVECTOR x2 = XMLoadFloat2(&v2);
	XMVECTOR X = XMVectorMax(x1, x2);
	XMStoreFloat2(&result, X);
}

inline TVector2 TVector2::Max(const TVector2& v1, const TVector2& v2) noexcept
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat2(&v1);
	XMVECTOR x2 = XMLoadFloat2(&v2);
	XMVECTOR X = XMVectorMax(x1, x2);

	TVector2 result;
	XMStoreFloat2(&result, X);
	return result;
}

inline void TVector2::Lerp(const TVector2& v1, const TVector2& v2, float t, TVector2& result) noexcept
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat2(&v1);
	XMVECTOR x2 = XMLoadFloat2(&v2);
	XMVECTOR X = XMVectorLerp(x1, x2, t);
	XMStoreFloat2(&result, X);
}

inline TVector2 TVector2::Lerp(const TVector2& v1, const TVector2& v2, float t) noexcept
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat2(&v1);
	XMVECTOR x2 = XMLoadFloat2(&v2);
	XMVECTOR X = XMVectorLerp(x1, x2, t);

	TVector2 result;
	XMStoreFloat2(&result, X);
	return result;
}

inline void TVector2::SmoothStep(const TVector2& v1, const TVector2& v2, float t, TVector2& result) noexcept
{
	using namespace DirectX;
	t = (t > 1.0f) ? 1.0f : ((t < 0.0f) ? 0.0f : t);  // Clamp value to 0 to 1
	t = t * t * (3.f - 2.f * t);
	XMVECTOR x1 = XMLoadFloat2(&v1);
	XMVECTOR x2 = XMLoadFloat2(&v2);
	XMVECTOR X = XMVectorLerp(x1, x2, t);
	XMStoreFloat2(&result, X);
}

inline TVector2 TVector2::SmoothStep(const TVector2& v1, const TVector2& v2, float t) noexcept
{
	using namespace DirectX;
	t = (t > 1.0f) ? 1.0f : ((t < 0.0f) ? 0.0f : t);  // Clamp value to 0 to 1
	t = t * t * (3.f - 2.f * t);
	XMVECTOR x1 = XMLoadFloat2(&v1);
	XMVECTOR x2 = XMLoadFloat2(&v2);
	XMVECTOR X = XMVectorLerp(x1, x2, t);

	TVector2 result;
	XMStoreFloat2(&result, X);
	return result;
}

inline void TVector2::Barycentric(const TVector2& v1, const TVector2& v2, const TVector2& v3, float f, float g, TVector2& result) noexcept
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat2(&v1);
	XMVECTOR x2 = XMLoadFloat2(&v2);
	XMVECTOR x3 = XMLoadFloat2(&v3);
	XMVECTOR X = XMVectorBaryCentric(x1, x2, x3, f, g);
	XMStoreFloat2(&result, X);
}

inline TVector2 TVector2::Barycentric(const TVector2& v1, const TVector2& v2, const TVector2& v3, float f, float g) noexcept
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat2(&v1);
	XMVECTOR x2 = XMLoadFloat2(&v2);
	XMVECTOR x3 = XMLoadFloat2(&v3);
	XMVECTOR X = XMVectorBaryCentric(x1, x2, x3, f, g);

	TVector2 result;
	XMStoreFloat2(&result, X);
	return result;
}

inline void TVector2::CatmullRom(const TVector2& v1, const TVector2& v2, const TVector2& v3, const TVector2& v4, float t, TVector2& result) noexcept
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat2(&v1);
	XMVECTOR x2 = XMLoadFloat2(&v2);
	XMVECTOR x3 = XMLoadFloat2(&v3);
	XMVECTOR x4 = XMLoadFloat2(&v4);
	XMVECTOR X = XMVectorCatmullRom(x1, x2, x3, x4, t);
	XMStoreFloat2(&result, X);
}

inline TVector2 TVector2::CatmullRom(const TVector2& v1, const TVector2& v2, const TVector2& v3, const TVector2& v4, float t) noexcept
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat2(&v1);
	XMVECTOR x2 = XMLoadFloat2(&v2);
	XMVECTOR x3 = XMLoadFloat2(&v3);
	XMVECTOR x4 = XMLoadFloat2(&v4);
	XMVECTOR X = XMVectorCatmullRom(x1, x2, x3, x4, t);

	TVector2 result;
	XMStoreFloat2(&result, X);
	return result;
}

inline void TVector2::Hermite(const TVector2& v1, const TVector2& t1, const TVector2& v2, const TVector2& t2, float t, TVector2& result) noexcept
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat2(&v1);
	XMVECTOR x2 = XMLoadFloat2(&t1);
	XMVECTOR x3 = XMLoadFloat2(&v2);
	XMVECTOR x4 = XMLoadFloat2(&t2);
	XMVECTOR X = XMVectorHermite(x1, x2, x3, x4, t);
	XMStoreFloat2(&result, X);
}

inline TVector2 TVector2::Hermite(const TVector2& v1, const TVector2& t1, const TVector2& v2, const TVector2& t2, float t) noexcept
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat2(&v1);
	XMVECTOR x2 = XMLoadFloat2(&t1);
	XMVECTOR x3 = XMLoadFloat2(&v2);
	XMVECTOR x4 = XMLoadFloat2(&t2);
	XMVECTOR X = XMVectorHermite(x1, x2, x3, x4, t);

	TVector2 result;
	XMStoreFloat2(&result, X);
	return result;
}

inline void TVector2::Reflect(const TVector2& ivec, const TVector2& nvec, TVector2& result) noexcept
{
	using namespace DirectX;
	XMVECTOR i = XMLoadFloat2(&ivec);
	XMVECTOR n = XMLoadFloat2(&nvec);
	XMVECTOR X = XMVector2Reflect(i, n);
	XMStoreFloat2(&result, X);
}

inline TVector2 TVector2::Reflect(const TVector2& ivec, const TVector2& nvec) noexcept
{
	using namespace DirectX;
	XMVECTOR i = XMLoadFloat2(&ivec);
	XMVECTOR n = XMLoadFloat2(&nvec);
	XMVECTOR X = XMVector2Reflect(i, n);

	TVector2 result;
	XMStoreFloat2(&result, X);
	return result;
}

inline void TVector2::Refract(const TVector2& ivec, const TVector2& nvec, float refractionIndex, TVector2& result) noexcept
{
	using namespace DirectX;
	XMVECTOR i = XMLoadFloat2(&ivec);
	XMVECTOR n = XMLoadFloat2(&nvec);
	XMVECTOR X = XMVector2Refract(i, n, refractionIndex);
	XMStoreFloat2(&result, X);
}

inline TVector2 TVector2::Refract(const TVector2& ivec, const TVector2& nvec, float refractionIndex) noexcept
{
	using namespace DirectX;
	XMVECTOR i = XMLoadFloat2(&ivec);
	XMVECTOR n = XMLoadFloat2(&nvec);
	XMVECTOR X = XMVector2Refract(i, n, refractionIndex);

	TVector2 result;
	XMStoreFloat2(&result, X);
	return result;
}