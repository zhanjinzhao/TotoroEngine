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

// 4D vector
struct TVector4 : public DirectX::XMFLOAT4
{
	TVector4() noexcept : XMFLOAT4(0.f, 0.f, 0.f, 0.f) {}
	constexpr explicit TVector4(float ix) noexcept : XMFLOAT4(ix, ix, ix, ix) {}
	constexpr TVector4(float ix, float iy, float iz, float iw) noexcept : XMFLOAT4(ix, iy, iz, iw) {}
	explicit TVector4(_In_reads_(4) const float* pArray) noexcept : XMFLOAT4(pArray) {}
	TVector4(DirectX::FXMVECTOR V) noexcept { XMStoreFloat4(this, V); }
	TVector4(const XMFLOAT4& V) noexcept { this->x = V.x; this->y = V.y; this->z = V.z; this->w = V.w; }
	explicit TVector4(const DirectX::XMVECTORF32& F) noexcept { this->x = F.f[0]; this->y = F.f[1]; this->z = F.f[2]; this->w = F.f[3]; }

	TVector4(const TVector4&) = default;
	TVector4& operator=(const TVector4&) = default;

	TVector4(TVector4&&) = default;
	TVector4& operator=(TVector4&&) = default;

	operator DirectX::XMVECTOR() const  noexcept { return XMLoadFloat4(this); }

	// Comparison operators
	bool operator == (const TVector4& V) const noexcept;
	bool operator != (const TVector4& V) const noexcept;

	// Assignment operators
	TVector4& operator= (const DirectX::XMVECTORF32& F) noexcept { x = F.f[0]; y = F.f[1]; z = F.f[2]; w = F.f[3]; return *this; }
	TVector4& operator+= (const TVector4& V) noexcept;
	TVector4& operator-= (const TVector4& V) noexcept;
	TVector4& operator*= (const TVector4& V) noexcept;
	TVector4& operator*= (float S) noexcept;
	TVector4& operator/= (float S) noexcept;

	// Unary operators
	TVector4 operator+ () const noexcept { return *this; }
	TVector4 operator- () const noexcept;

	// Vector operations
	bool InBounds(const TVector4& Bounds) const noexcept;

	float Length() const noexcept;
	float LengthSquared() const noexcept;

	float Dot(const TVector4& V) const noexcept;
	void Cross(const TVector4& v1, const TVector4& v2, TVector4& result) const noexcept;
	TVector4 Cross(const TVector4& v1, const TVector4& v2) const noexcept;

	void Normalize() noexcept;
	void Normalize(TVector4& result) const noexcept;

	void Clamp(const TVector4& vmin, const TVector4& vmax) noexcept;
	void Clamp(const TVector4& vmin, const TVector4& vmax, TVector4& result) const noexcept;

	// Static functions
	static float Distance(const TVector4& v1, const TVector4& v2) noexcept;
	static float DistanceSquared(const TVector4& v1, const TVector4& v2) noexcept;

	static void Min(const TVector4& v1, const TVector4& v2, TVector4& result) noexcept;
	static TVector4 Min(const TVector4& v1, const TVector4& v2) noexcept;

	static void Max(const TVector4& v1, const TVector4& v2, TVector4& result) noexcept;
	static TVector4 Max(const TVector4& v1, const TVector4& v2) noexcept;

	static void Lerp(const TVector4& v1, const TVector4& v2, float t, TVector4& result) noexcept;
	static TVector4 Lerp(const TVector4& v1, const TVector4& v2, float t) noexcept;

	static void SmoothStep(const TVector4& v1, const TVector4& v2, float t, TVector4& result) noexcept;
	static TVector4 SmoothStep(const TVector4& v1, const TVector4& v2, float t) noexcept;

	static void Barycentric(const TVector4& v1, const TVector4& v2, const TVector4& v3, float f, float g, TVector4& result) noexcept;
	static TVector4 Barycentric(const TVector4& v1, const TVector4& v2, const TVector4& v3, float f, float g) noexcept;

	static void CatmullRom(const TVector4& v1, const TVector4& v2, const TVector4& v3, const TVector4& v4, float t, TVector4& result) noexcept;
	static TVector4 CatmullRom(const TVector4& v1, const TVector4& v2, const TVector4& v3, const TVector4& v4, float t) noexcept;

	static void Hermite(const TVector4& v1, const TVector4& t1, const TVector4& v2, const TVector4& t2, float t, TVector4& result) noexcept;
	static TVector4 Hermite(const TVector4& v1, const TVector4& t1, const TVector4& v2, const TVector4& t2, float t) noexcept;

	static void Reflect(const TVector4& ivec, const TVector4& nvec, TVector4& result) noexcept;
	static TVector4 Reflect(const TVector4& ivec, const TVector4& nvec) noexcept;

	static void Refract(const TVector4& ivec, const TVector4& nvec, float refractionIndex, TVector4& result) noexcept;
	static TVector4 Refract(const TVector4& ivec, const TVector4& nvec, float refractionIndex) noexcept;

	// Constants
	static const TVector4 Zero;
	static const TVector4 One;
	static const TVector4 UnitX;
	static const TVector4 UnitY;
	static const TVector4 UnitZ;
	static const TVector4 UnitW;
};


//------------------------------------------------------------------------------
// Comparision operators
//------------------------------------------------------------------------------

inline bool TVector4::operator == (const TVector4& V) const noexcept
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat4(this);
	XMVECTOR v2 = XMLoadFloat4(&V);
	return XMVector4Equal(v1, v2);
}

inline bool TVector4::operator != (const TVector4& V) const noexcept
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat4(this);
	XMVECTOR v2 = XMLoadFloat4(&V);
	return XMVector4NotEqual(v1, v2);
}

//------------------------------------------------------------------------------
// Assignment operators
//------------------------------------------------------------------------------

inline TVector4& TVector4::operator+= (const TVector4& V) noexcept
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat4(this);
	XMVECTOR v2 = XMLoadFloat4(&V);
	XMVECTOR X = XMVectorAdd(v1, v2);
	XMStoreFloat4(this, X);
	return *this;
}

inline TVector4& TVector4::operator-= (const TVector4& V) noexcept
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat4(this);
	XMVECTOR v2 = XMLoadFloat4(&V);
	XMVECTOR X = XMVectorSubtract(v1, v2);
	XMStoreFloat4(this, X);
	return *this;
}

inline TVector4& TVector4::operator*= (const TVector4& V) noexcept
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat4(this);
	XMVECTOR v2 = XMLoadFloat4(&V);
	XMVECTOR X = XMVectorMultiply(v1, v2);
	XMStoreFloat4(this, X);
	return *this;
}

inline TVector4& TVector4::operator*= (float S) noexcept
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat4(this);
	XMVECTOR X = XMVectorScale(v1, S);
	XMStoreFloat4(this, X);
	return *this;
}

inline TVector4& TVector4::operator/= (float S) noexcept
{
	using namespace DirectX;
	assert(S != 0.0f);
	XMVECTOR v1 = XMLoadFloat4(this);
	XMVECTOR X = XMVectorScale(v1, 1.f / S);
	XMStoreFloat4(this, X);
	return *this;
}

//------------------------------------------------------------------------------
// Urnary operators
//------------------------------------------------------------------------------

inline TVector4 TVector4::operator- () const noexcept
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat4(this);
	XMVECTOR X = XMVectorNegate(v1);
	TVector4 R;
	XMStoreFloat4(&R, X);
	return R;
}

//------------------------------------------------------------------------------
// Binary operators
//------------------------------------------------------------------------------

inline TVector4 operator+ (const TVector4& V1, const TVector4& V2) noexcept
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat4(&V1);
	XMVECTOR v2 = XMLoadFloat4(&V2);
	XMVECTOR X = XMVectorAdd(v1, v2);
	TVector4 R;
	XMStoreFloat4(&R, X);
	return R;
}

inline TVector4 operator- (const TVector4& V1, const TVector4& V2) noexcept
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat4(&V1);
	XMVECTOR v2 = XMLoadFloat4(&V2);
	XMVECTOR X = XMVectorSubtract(v1, v2);
	TVector4 R;
	XMStoreFloat4(&R, X);
	return R;
}

inline TVector4 operator* (const TVector4& V1, const TVector4& V2) noexcept
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat4(&V1);
	XMVECTOR v2 = XMLoadFloat4(&V2);
	XMVECTOR X = XMVectorMultiply(v1, v2);
	TVector4 R;
	XMStoreFloat4(&R, X);
	return R;
}

inline TVector4 operator* (const TVector4& V, float S) noexcept
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat4(&V);
	XMVECTOR X = XMVectorScale(v1, S);
	TVector4 R;
	XMStoreFloat4(&R, X);
	return R;
}

inline TVector4 operator/ (const TVector4& V1, const TVector4& V2) noexcept
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat4(&V1);
	XMVECTOR v2 = XMLoadFloat4(&V2);
	XMVECTOR X = XMVectorDivide(v1, v2);
	TVector4 R;
	XMStoreFloat4(&R, X);
	return R;
}

inline TVector4 operator/ (const TVector4& V, float S) noexcept
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat4(&V);
	XMVECTOR X = XMVectorScale(v1, 1.f / S);
	TVector4 R;
	XMStoreFloat4(&R, X);
	return R;
}

inline TVector4 operator* (float S, const TVector4& V) noexcept
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat4(&V);
	XMVECTOR X = XMVectorScale(v1, S);
	TVector4 R;
	XMStoreFloat4(&R, X);
	return R;
}

//------------------------------------------------------------------------------
// Vector operations
//------------------------------------------------------------------------------

inline bool TVector4::InBounds(const TVector4& Bounds) const noexcept
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat4(this);
	XMVECTOR v2 = XMLoadFloat4(&Bounds);
	return XMVector4InBounds(v1, v2);
}

inline float TVector4::Length() const noexcept
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat4(this);
	XMVECTOR X = XMVector4Length(v1);
	return XMVectorGetX(X);
}

inline float TVector4::LengthSquared() const noexcept
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat4(this);
	XMVECTOR X = XMVector4LengthSq(v1);
	return XMVectorGetX(X);
}

inline float TVector4::Dot(const TVector4& V) const noexcept
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat4(this);
	XMVECTOR v2 = XMLoadFloat4(&V);
	XMVECTOR X = XMVector4Dot(v1, v2);
	return XMVectorGetX(X);
}

inline void TVector4::Cross(const TVector4& v1, const TVector4& v2, TVector4& result) const noexcept
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat4(this);
	XMVECTOR x2 = XMLoadFloat4(&v1);
	XMVECTOR x3 = XMLoadFloat4(&v2);
	XMVECTOR R = XMVector4Cross(x1, x2, x3);
	XMStoreFloat4(&result, R);
}

inline TVector4 TVector4::Cross(const TVector4& v1, const TVector4& v2) const noexcept
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat4(this);
	XMVECTOR x2 = XMLoadFloat4(&v1);
	XMVECTOR x3 = XMLoadFloat4(&v2);
	XMVECTOR R = XMVector4Cross(x1, x2, x3);

	TVector4 result;
	XMStoreFloat4(&result, R);
	return result;
}

inline void TVector4::Normalize() noexcept
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat4(this);
	XMVECTOR X = XMVector4Normalize(v1);
	XMStoreFloat4(this, X);
}

inline void TVector4::Normalize(TVector4& result) const noexcept
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat4(this);
	XMVECTOR X = XMVector4Normalize(v1);
	XMStoreFloat4(&result, X);
}

inline void TVector4::Clamp(const TVector4& vmin, const TVector4& vmax) noexcept
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat4(this);
	XMVECTOR v2 = XMLoadFloat4(&vmin);
	XMVECTOR v3 = XMLoadFloat4(&vmax);
	XMVECTOR X = XMVectorClamp(v1, v2, v3);
	XMStoreFloat4(this, X);
}

inline void TVector4::Clamp(const TVector4& vmin, const TVector4& vmax, TVector4& result) const noexcept
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat4(this);
	XMVECTOR v2 = XMLoadFloat4(&vmin);
	XMVECTOR v3 = XMLoadFloat4(&vmax);
	XMVECTOR X = XMVectorClamp(v1, v2, v3);
	XMStoreFloat4(&result, X);
}

//------------------------------------------------------------------------------
// Static functions
//------------------------------------------------------------------------------

inline float TVector4::Distance(const TVector4& v1, const TVector4& v2) noexcept
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat4(&v1);
	XMVECTOR x2 = XMLoadFloat4(&v2);
	XMVECTOR V = XMVectorSubtract(x2, x1);
	XMVECTOR X = XMVector4Length(V);
	return XMVectorGetX(X);
}

inline float TVector4::DistanceSquared(const TVector4& v1, const TVector4& v2) noexcept
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat4(&v1);
	XMVECTOR x2 = XMLoadFloat4(&v2);
	XMVECTOR V = XMVectorSubtract(x2, x1);
	XMVECTOR X = XMVector4LengthSq(V);
	return XMVectorGetX(X);
}

inline void TVector4::Min(const TVector4& v1, const TVector4& v2, TVector4& result) noexcept
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat4(&v1);
	XMVECTOR x2 = XMLoadFloat4(&v2);
	XMVECTOR X = XMVectorMin(x1, x2);
	XMStoreFloat4(&result, X);
}

inline TVector4 TVector4::Min(const TVector4& v1, const TVector4& v2) noexcept
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat4(&v1);
	XMVECTOR x2 = XMLoadFloat4(&v2);
	XMVECTOR X = XMVectorMin(x1, x2);

	TVector4 result;
	XMStoreFloat4(&result, X);
	return result;
}

inline void TVector4::Max(const TVector4& v1, const TVector4& v2, TVector4& result) noexcept
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat4(&v1);
	XMVECTOR x2 = XMLoadFloat4(&v2);
	XMVECTOR X = XMVectorMax(x1, x2);
	XMStoreFloat4(&result, X);
}

inline TVector4 TVector4::Max(const TVector4& v1, const TVector4& v2) noexcept
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat4(&v1);
	XMVECTOR x2 = XMLoadFloat4(&v2);
	XMVECTOR X = XMVectorMax(x1, x2);

	TVector4 result;
	XMStoreFloat4(&result, X);
	return result;
}

inline void TVector4::Lerp(const TVector4& v1, const TVector4& v2, float t, TVector4& result) noexcept
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat4(&v1);
	XMVECTOR x2 = XMLoadFloat4(&v2);
	XMVECTOR X = XMVectorLerp(x1, x2, t);
	XMStoreFloat4(&result, X);
}

inline TVector4 TVector4::Lerp(const TVector4& v1, const TVector4& v2, float t) noexcept
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat4(&v1);
	XMVECTOR x2 = XMLoadFloat4(&v2);
	XMVECTOR X = XMVectorLerp(x1, x2, t);

	TVector4 result;
	XMStoreFloat4(&result, X);
	return result;
}

inline void TVector4::SmoothStep(const TVector4& v1, const TVector4& v2, float t, TVector4& result) noexcept
{
	using namespace DirectX;
	t = (t > 1.0f) ? 1.0f : ((t < 0.0f) ? 0.0f : t);  // Clamp value to 0 to 1
	t = t * t * (3.f - 2.f * t);
	XMVECTOR x1 = XMLoadFloat4(&v1);
	XMVECTOR x2 = XMLoadFloat4(&v2);
	XMVECTOR X = XMVectorLerp(x1, x2, t);
	XMStoreFloat4(&result, X);
}

inline TVector4 TVector4::SmoothStep(const TVector4& v1, const TVector4& v2, float t) noexcept
{
	using namespace DirectX;
	t = (t > 1.0f) ? 1.0f : ((t < 0.0f) ? 0.0f : t);  // Clamp value to 0 to 1
	t = t * t * (3.f - 2.f * t);
	XMVECTOR x1 = XMLoadFloat4(&v1);
	XMVECTOR x2 = XMLoadFloat4(&v2);
	XMVECTOR X = XMVectorLerp(x1, x2, t);

	TVector4 result;
	XMStoreFloat4(&result, X);
	return result;
}

inline void TVector4::Barycentric(const TVector4& v1, const TVector4& v2, const TVector4& v3, float f, float g, TVector4& result) noexcept
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat4(&v1);
	XMVECTOR x2 = XMLoadFloat4(&v2);
	XMVECTOR x3 = XMLoadFloat4(&v3);
	XMVECTOR X = XMVectorBaryCentric(x1, x2, x3, f, g);
	XMStoreFloat4(&result, X);
}

inline TVector4 TVector4::Barycentric(const TVector4& v1, const TVector4& v2, const TVector4& v3, float f, float g) noexcept
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat4(&v1);
	XMVECTOR x2 = XMLoadFloat4(&v2);
	XMVECTOR x3 = XMLoadFloat4(&v3);
	XMVECTOR X = XMVectorBaryCentric(x1, x2, x3, f, g);

	TVector4 result;
	XMStoreFloat4(&result, X);
	return result;
}

inline void TVector4::CatmullRom(const TVector4& v1, const TVector4& v2, const TVector4& v3, const TVector4& v4, float t, TVector4& result) noexcept
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat4(&v1);
	XMVECTOR x2 = XMLoadFloat4(&v2);
	XMVECTOR x3 = XMLoadFloat4(&v3);
	XMVECTOR x4 = XMLoadFloat4(&v4);
	XMVECTOR X = XMVectorCatmullRom(x1, x2, x3, x4, t);
	XMStoreFloat4(&result, X);
}

inline TVector4 TVector4::CatmullRom(const TVector4& v1, const TVector4& v2, const TVector4& v3, const TVector4& v4, float t) noexcept
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat4(&v1);
	XMVECTOR x2 = XMLoadFloat4(&v2);
	XMVECTOR x3 = XMLoadFloat4(&v3);
	XMVECTOR x4 = XMLoadFloat4(&v4);
	XMVECTOR X = XMVectorCatmullRom(x1, x2, x3, x4, t);

	TVector4 result;
	XMStoreFloat4(&result, X);
	return result;
}

inline void TVector4::Hermite(const TVector4& v1, const TVector4& t1, const TVector4& v2, const TVector4& t2, float t, TVector4& result) noexcept
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat4(&v1);
	XMVECTOR x2 = XMLoadFloat4(&t1);
	XMVECTOR x3 = XMLoadFloat4(&v2);
	XMVECTOR x4 = XMLoadFloat4(&t2);
	XMVECTOR X = XMVectorHermite(x1, x2, x3, x4, t);
	XMStoreFloat4(&result, X);
}

inline TVector4 TVector4::Hermite(const TVector4& v1, const TVector4& t1, const TVector4& v2, const TVector4& t2, float t) noexcept
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat4(&v1);
	XMVECTOR x2 = XMLoadFloat4(&t1);
	XMVECTOR x3 = XMLoadFloat4(&v2);
	XMVECTOR x4 = XMLoadFloat4(&t2);
	XMVECTOR X = XMVectorHermite(x1, x2, x3, x4, t);

	TVector4 result;
	XMStoreFloat4(&result, X);
	return result;
}

inline void TVector4::Reflect(const TVector4& ivec, const TVector4& nvec, TVector4& result) noexcept
{
	using namespace DirectX;
	XMVECTOR i = XMLoadFloat4(&ivec);
	XMVECTOR n = XMLoadFloat4(&nvec);
	XMVECTOR X = XMVector4Reflect(i, n);
	XMStoreFloat4(&result, X);
}

inline TVector4 TVector4::Reflect(const TVector4& ivec, const TVector4& nvec) noexcept
{
	using namespace DirectX;
	XMVECTOR i = XMLoadFloat4(&ivec);
	XMVECTOR n = XMLoadFloat4(&nvec);
	XMVECTOR X = XMVector4Reflect(i, n);

	TVector4 result;
	XMStoreFloat4(&result, X);
	return result;
}

inline void TVector4::Refract(const TVector4& ivec, const TVector4& nvec, float refractionIndex, TVector4& result) noexcept
{
	using namespace DirectX;
	XMVECTOR i = XMLoadFloat4(&ivec);
	XMVECTOR n = XMLoadFloat4(&nvec);
	XMVECTOR X = XMVector4Refract(i, n, refractionIndex);
	XMStoreFloat4(&result, X);
}

inline TVector4 TVector4::Refract(const TVector4& ivec, const TVector4& nvec, float refractionIndex) noexcept
{
	using namespace DirectX;
	XMVECTOR i = XMLoadFloat4(&ivec);
	XMVECTOR n = XMLoadFloat4(&nvec);
	XMVECTOR X = XMVector4Refract(i, n, refractionIndex);

	TVector4 result;
	XMStoreFloat4(&result, X);
	return result;
}