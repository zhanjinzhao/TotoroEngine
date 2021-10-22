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

// 3D vector
struct TVector3 : public DirectX::XMFLOAT3
{
	TVector3() noexcept : XMFLOAT3(0.f, 0.f, 0.f) {}
	constexpr explicit TVector3(float ix) noexcept : XMFLOAT3(ix, ix, ix) {}
	constexpr TVector3(float ix, float iy, float iz) noexcept : XMFLOAT3(ix, iy, iz) {}
	explicit TVector3(_In_reads_(3) const float* pArray) noexcept : XMFLOAT3(pArray) {}
	TVector3(DirectX::FXMVECTOR V) noexcept { XMStoreFloat3(this, V); }
	TVector3(const XMFLOAT3& V) noexcept { this->x = V.x; this->y = V.y; this->z = V.z; }
	explicit TVector3(const DirectX::XMVECTORF32& F) noexcept { this->x = F.f[0]; this->y = F.f[1]; this->z = F.f[2]; }

	TVector3(const TVector3&) = default;
	TVector3& operator=(const TVector3&) = default;

	TVector3(TVector3&&) = default;
	TVector3& operator=(TVector3&&) = default;

	operator DirectX::XMVECTOR() const noexcept { return XMLoadFloat3(this); }

	// Access operators
	float operator [](int i) const noexcept
	{
		if (i == 0) return x;
		if (i == 1) return y;
		return z;
	}

	float& operator [](int i) noexcept
	{
		if (i == 0) return x;
		if (i == 1) return y;
		return z;
	}

	// Comparison operators
	bool operator == (const TVector3& V) const noexcept;
	bool operator != (const TVector3& V) const noexcept;

	// Assignment operators
	TVector3& operator= (const DirectX::XMVECTORF32& F) noexcept { x = F.f[0]; y = F.f[1]; z = F.f[2]; return *this; }
	TVector3& operator+= (const TVector3& V) noexcept;
	TVector3& operator-= (const TVector3& V) noexcept;
	TVector3& operator*= (const TVector3& V) noexcept;
	TVector3& operator*= (float S) noexcept;
	TVector3& operator/= (float S) noexcept;

	// Unary operators
	TVector3 operator+ () const noexcept { return *this; }
	TVector3 operator- () const noexcept;

	// Vector operations
	bool InBounds(const TVector3& Bounds) const noexcept;

	float Length() const noexcept;
	float LengthSquared() const noexcept;

	float Dot(const TVector3& V) const noexcept;
	void Cross(const TVector3& V, TVector3& result) const noexcept;
	TVector3 Cross(const TVector3& V) const noexcept;

	void Normalize() noexcept;
	void Normalize(TVector3& result) const noexcept;

	void Clamp(const TVector3& vmin, const TVector3& vmax) noexcept;
	void Clamp(const TVector3& vmin, const TVector3& vmax, TVector3& result) const noexcept;

	// Static functions
	static float Distance(const TVector3& v1, const TVector3& v2) noexcept;
	static float DistanceSquared(const TVector3& v1, const TVector3& v2) noexcept;

	static void Min(const TVector3& v1, const TVector3& v2, TVector3& result) noexcept;
	static TVector3 Min(const TVector3& v1, const TVector3& v2) noexcept;

	static void Max(const TVector3& v1, const TVector3& v2, TVector3& result) noexcept;
	static TVector3 Max(const TVector3& v1, const TVector3& v2) noexcept;

	static void Lerp(const TVector3& v1, const TVector3& v2, float t, TVector3& result) noexcept;
	static TVector3 Lerp(const TVector3& v1, const TVector3& v2, float t) noexcept;

	static void SmoothStep(const TVector3& v1, const TVector3& v2, float t, TVector3& result) noexcept;
	static TVector3 SmoothStep(const TVector3& v1, const TVector3& v2, float t) noexcept;

	static void Barycentric(const TVector3& v1, const TVector3& v2, const TVector3& v3, float f, float g, TVector3& result) noexcept;
	static TVector3 Barycentric(const TVector3& v1, const TVector3& v2, const TVector3& v3, float f, float g) noexcept;

	static void CatmullRom(const TVector3& v1, const TVector3& v2, const TVector3& v3, const TVector3& v4, float t, TVector3& result) noexcept;
	static TVector3 CatmullRom(const TVector3& v1, const TVector3& v2, const TVector3& v3, const TVector3& v4, float t) noexcept;

	static void Hermite(const TVector3& v1, const TVector3& t1, const TVector3& v2, const TVector3& t2, float t, TVector3& result) noexcept;
	static TVector3 Hermite(const TVector3& v1, const TVector3& t1, const TVector3& v2, const TVector3& t2, float t) noexcept;

	static void Reflect(const TVector3& ivec, const TVector3& nvec, TVector3& result) noexcept;
	static TVector3 Reflect(const TVector3& ivec, const TVector3& nvec) noexcept;

	static void Refract(const TVector3& ivec, const TVector3& nvec, float refractionIndex, TVector3& result) noexcept;
	static TVector3 Refract(const TVector3& ivec, const TVector3& nvec, float refractionIndex) noexcept;

	// Constants
	static const TVector3 Zero;
	static const TVector3 One;
	static const TVector3 UnitX;
	static const TVector3 UnitY;
	static const TVector3 UnitZ;
	static const TVector3 Up;
	static const TVector3 Down;
	static const TVector3 Right;
	static const TVector3 Left;
	static const TVector3 Forward;
	static const TVector3 Backward;
};

//------------------------------------------------------------------------------
// Comparision operators
//------------------------------------------------------------------------------

inline bool TVector3::operator == (const TVector3& V) const noexcept
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat3(this);
	XMVECTOR v2 = XMLoadFloat3(&V);
	return XMVector3Equal(v1, v2);
}

inline bool TVector3::operator != (const TVector3& V) const noexcept
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat3(this);
	XMVECTOR v2 = XMLoadFloat3(&V);
	return XMVector3NotEqual(v1, v2);
}

//------------------------------------------------------------------------------
// Assignment operators
//------------------------------------------------------------------------------

inline TVector3& TVector3::operator+= (const TVector3& V) noexcept
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat3(this);
	XMVECTOR v2 = XMLoadFloat3(&V);
	XMVECTOR X = XMVectorAdd(v1, v2);
	XMStoreFloat3(this, X);
	return *this;
}

inline TVector3& TVector3::operator-= (const TVector3& V) noexcept
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat3(this);
	XMVECTOR v2 = XMLoadFloat3(&V);
	XMVECTOR X = XMVectorSubtract(v1, v2);
	XMStoreFloat3(this, X);
	return *this;
}

inline TVector3& TVector3::operator*= (const TVector3& V) noexcept
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat3(this);
	XMVECTOR v2 = XMLoadFloat3(&V);
	XMVECTOR X = XMVectorMultiply(v1, v2);
	XMStoreFloat3(this, X);
	return *this;
}

inline TVector3& TVector3::operator*= (float S) noexcept
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat3(this);
	XMVECTOR X = XMVectorScale(v1, S);
	XMStoreFloat3(this, X);
	return *this;
}

inline TVector3& TVector3::operator/= (float S) noexcept
{
	using namespace DirectX;
	assert(S != 0.0f);
	XMVECTOR v1 = XMLoadFloat3(this);
	XMVECTOR X = XMVectorScale(v1, 1.f / S);
	XMStoreFloat3(this, X);
	return *this;
}

//------------------------------------------------------------------------------
// Urnary operators
//------------------------------------------------------------------------------

inline TVector3 TVector3::operator- () const noexcept
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat3(this);
	XMVECTOR X = XMVectorNegate(v1);
	TVector3 R;
	XMStoreFloat3(&R, X);
	return R;
}

//------------------------------------------------------------------------------
// Binary operators
//------------------------------------------------------------------------------

inline TVector3 operator+ (const TVector3& V1, const TVector3& V2) noexcept
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat3(&V1);
	XMVECTOR v2 = XMLoadFloat3(&V2);
	XMVECTOR X = XMVectorAdd(v1, v2);
	TVector3 R;
	XMStoreFloat3(&R, X);
	return R;
}

inline TVector3 operator- (const TVector3& V1, const TVector3& V2) noexcept
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat3(&V1);
	XMVECTOR v2 = XMLoadFloat3(&V2);
	XMVECTOR X = XMVectorSubtract(v1, v2);
	TVector3 R;
	XMStoreFloat3(&R, X);
	return R;
}

inline TVector3 operator* (const TVector3& V1, const TVector3& V2) noexcept
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat3(&V1);
	XMVECTOR v2 = XMLoadFloat3(&V2);
	XMVECTOR X = XMVectorMultiply(v1, v2);
	TVector3 R;
	XMStoreFloat3(&R, X);
	return R;
}

inline TVector3 operator* (const TVector3& V, float S) noexcept
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat3(&V);
	XMVECTOR X = XMVectorScale(v1, S);
	TVector3 R;
	XMStoreFloat3(&R, X);
	return R;
}

inline TVector3 operator/ (const TVector3& V1, const TVector3& V2) noexcept
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat3(&V1);
	XMVECTOR v2 = XMLoadFloat3(&V2);
	XMVECTOR X = XMVectorDivide(v1, v2);
	TVector3 R;
	XMStoreFloat3(&R, X);
	return R;
}

inline TVector3 operator/ (const TVector3& V, float S) noexcept
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat3(&V);
	XMVECTOR X = XMVectorScale(v1, 1.f / S);
	TVector3 R;
	XMStoreFloat3(&R, X);
	return R;
}

inline TVector3 operator* (float S, const TVector3& V) noexcept
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat3(&V);
	XMVECTOR X = XMVectorScale(v1, S);
	TVector3 R;
	XMStoreFloat3(&R, X);
	return R;
}

//------------------------------------------------------------------------------
// Vector operations
//------------------------------------------------------------------------------

inline bool TVector3::InBounds(const TVector3& Bounds) const noexcept
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat3(this);
	XMVECTOR v2 = XMLoadFloat3(&Bounds);
	return XMVector3InBounds(v1, v2);
}

inline float TVector3::Length() const noexcept
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat3(this);
	XMVECTOR X = XMVector3Length(v1);
	return XMVectorGetX(X);
}

inline float TVector3::LengthSquared() const noexcept
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat3(this);
	XMVECTOR X = XMVector3LengthSq(v1);
	return XMVectorGetX(X);
}

inline float TVector3::Dot(const TVector3& V) const noexcept
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat3(this);
	XMVECTOR v2 = XMLoadFloat3(&V);
	XMVECTOR X = XMVector3Dot(v1, v2);
	return XMVectorGetX(X);
}

inline void TVector3::Cross(const TVector3& V, TVector3& result) const noexcept
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat3(this);
	XMVECTOR v2 = XMLoadFloat3(&V);
	XMVECTOR R = XMVector3Cross(v1, v2);
	XMStoreFloat3(&result, R);
}

inline TVector3 TVector3::Cross(const TVector3& V) const noexcept
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat3(this);
	XMVECTOR v2 = XMLoadFloat3(&V);
	XMVECTOR R = XMVector3Cross(v1, v2);

	TVector3 result;
	XMStoreFloat3(&result, R);
	return result;
}

inline void TVector3::Normalize() noexcept
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat3(this);
	XMVECTOR X = XMVector3Normalize(v1);
	XMStoreFloat3(this, X);
}

inline void TVector3::Normalize(TVector3& result) const noexcept
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat3(this);
	XMVECTOR X = XMVector3Normalize(v1);
	XMStoreFloat3(&result, X);
}

inline void TVector3::Clamp(const TVector3& vmin, const TVector3& vmax) noexcept
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat3(this);
	XMVECTOR v2 = XMLoadFloat3(&vmin);
	XMVECTOR v3 = XMLoadFloat3(&vmax);
	XMVECTOR X = XMVectorClamp(v1, v2, v3);
	XMStoreFloat3(this, X);
}

inline void TVector3::Clamp(const TVector3& vmin, const TVector3& vmax, TVector3& result) const noexcept
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat3(this);
	XMVECTOR v2 = XMLoadFloat3(&vmin);
	XMVECTOR v3 = XMLoadFloat3(&vmax);
	XMVECTOR X = XMVectorClamp(v1, v2, v3);
	XMStoreFloat3(&result, X);
}

//------------------------------------------------------------------------------
// Static functions
//------------------------------------------------------------------------------

inline float TVector3::Distance(const TVector3& v1, const TVector3& v2) noexcept
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat3(&v1);
	XMVECTOR x2 = XMLoadFloat3(&v2);
	XMVECTOR V = XMVectorSubtract(x2, x1);
	XMVECTOR X = XMVector3Length(V);
	return XMVectorGetX(X);
}

inline float TVector3::DistanceSquared(const TVector3& v1, const TVector3& v2) noexcept
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat3(&v1);
	XMVECTOR x2 = XMLoadFloat3(&v2);
	XMVECTOR V = XMVectorSubtract(x2, x1);
	XMVECTOR X = XMVector3LengthSq(V);
	return XMVectorGetX(X);
}

inline void TVector3::Min(const TVector3& v1, const TVector3& v2, TVector3& result) noexcept
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat3(&v1);
	XMVECTOR x2 = XMLoadFloat3(&v2);
	XMVECTOR X = XMVectorMin(x1, x2);
	XMStoreFloat3(&result, X);
}

inline TVector3 TVector3::Min(const TVector3& v1, const TVector3& v2) noexcept
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat3(&v1);
	XMVECTOR x2 = XMLoadFloat3(&v2);
	XMVECTOR X = XMVectorMin(x1, x2);

	TVector3 result;
	XMStoreFloat3(&result, X);
	return result;
}

inline void TVector3::Max(const TVector3& v1, const TVector3& v2, TVector3& result) noexcept
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat3(&v1);
	XMVECTOR x2 = XMLoadFloat3(&v2);
	XMVECTOR X = XMVectorMax(x1, x2);
	XMStoreFloat3(&result, X);
}

inline TVector3 TVector3::Max(const TVector3& v1, const TVector3& v2) noexcept
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat3(&v1);
	XMVECTOR x2 = XMLoadFloat3(&v2);
	XMVECTOR X = XMVectorMax(x1, x2);

	TVector3 result;
	XMStoreFloat3(&result, X);
	return result;
}

inline void TVector3::Lerp(const TVector3& v1, const TVector3& v2, float t, TVector3& result) noexcept
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat3(&v1);
	XMVECTOR x2 = XMLoadFloat3(&v2);
	XMVECTOR X = XMVectorLerp(x1, x2, t);
	XMStoreFloat3(&result, X);
}

inline TVector3 TVector3::Lerp(const TVector3& v1, const TVector3& v2, float t) noexcept
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat3(&v1);
	XMVECTOR x2 = XMLoadFloat3(&v2);
	XMVECTOR X = XMVectorLerp(x1, x2, t);

	TVector3 result;
	XMStoreFloat3(&result, X);
	return result;
}

inline void TVector3::SmoothStep(const TVector3& v1, const TVector3& v2, float t, TVector3& result) noexcept
{
	using namespace DirectX;
	t = (t > 1.0f) ? 1.0f : ((t < 0.0f) ? 0.0f : t);  // Clamp value to 0 to 1
	t = t * t * (3.f - 2.f * t);
	XMVECTOR x1 = XMLoadFloat3(&v1);
	XMVECTOR x2 = XMLoadFloat3(&v2);
	XMVECTOR X = XMVectorLerp(x1, x2, t);
	XMStoreFloat3(&result, X);
}

inline TVector3 TVector3::SmoothStep(const TVector3& v1, const TVector3& v2, float t) noexcept
{
	using namespace DirectX;
	t = (t > 1.0f) ? 1.0f : ((t < 0.0f) ? 0.0f : t);  // Clamp value to 0 to 1
	t = t * t * (3.f - 2.f * t);
	XMVECTOR x1 = XMLoadFloat3(&v1);
	XMVECTOR x2 = XMLoadFloat3(&v2);
	XMVECTOR X = XMVectorLerp(x1, x2, t);

	TVector3 result;
	XMStoreFloat3(&result, X);
	return result;
}

inline void TVector3::Barycentric(const TVector3& v1, const TVector3& v2, const TVector3& v3, float f, float g, TVector3& result) noexcept
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat3(&v1);
	XMVECTOR x2 = XMLoadFloat3(&v2);
	XMVECTOR x3 = XMLoadFloat3(&v3);
	XMVECTOR X = XMVectorBaryCentric(x1, x2, x3, f, g);
	XMStoreFloat3(&result, X);
}

inline TVector3 TVector3::Barycentric(const TVector3& v1, const TVector3& v2, const TVector3& v3, float f, float g) noexcept
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat3(&v1);
	XMVECTOR x2 = XMLoadFloat3(&v2);
	XMVECTOR x3 = XMLoadFloat3(&v3);
	XMVECTOR X = XMVectorBaryCentric(x1, x2, x3, f, g);

	TVector3 result;
	XMStoreFloat3(&result, X);
	return result;
}

inline void TVector3::CatmullRom(const TVector3& v1, const TVector3& v2, const TVector3& v3, const TVector3& v4, float t, TVector3& result) noexcept
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat3(&v1);
	XMVECTOR x2 = XMLoadFloat3(&v2);
	XMVECTOR x3 = XMLoadFloat3(&v3);
	XMVECTOR x4 = XMLoadFloat3(&v4);
	XMVECTOR X = XMVectorCatmullRom(x1, x2, x3, x4, t);
	XMStoreFloat3(&result, X);
}

inline TVector3 TVector3::CatmullRom(const TVector3& v1, const TVector3& v2, const TVector3& v3, const TVector3& v4, float t) noexcept
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat3(&v1);
	XMVECTOR x2 = XMLoadFloat3(&v2);
	XMVECTOR x3 = XMLoadFloat3(&v3);
	XMVECTOR x4 = XMLoadFloat3(&v4);
	XMVECTOR X = XMVectorCatmullRom(x1, x2, x3, x4, t);

	TVector3 result;
	XMStoreFloat3(&result, X);
	return result;
}

inline void TVector3::Hermite(const TVector3& v1, const TVector3& t1, const TVector3& v2, const TVector3& t2, float t, TVector3& result) noexcept
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat3(&v1);
	XMVECTOR x2 = XMLoadFloat3(&t1);
	XMVECTOR x3 = XMLoadFloat3(&v2);
	XMVECTOR x4 = XMLoadFloat3(&t2);
	XMVECTOR X = XMVectorHermite(x1, x2, x3, x4, t);
	XMStoreFloat3(&result, X);
}

inline TVector3 TVector3::Hermite(const TVector3& v1, const TVector3& t1, const TVector3& v2, const TVector3& t2, float t) noexcept
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat3(&v1);
	XMVECTOR x2 = XMLoadFloat3(&t1);
	XMVECTOR x3 = XMLoadFloat3(&v2);
	XMVECTOR x4 = XMLoadFloat3(&t2);
	XMVECTOR X = XMVectorHermite(x1, x2, x3, x4, t);

	TVector3 result;
	XMStoreFloat3(&result, X);
	return result;
}

inline void TVector3::Reflect(const TVector3& ivec, const TVector3& nvec, TVector3& result) noexcept
{
	using namespace DirectX;
	XMVECTOR i = XMLoadFloat3(&ivec);
	XMVECTOR n = XMLoadFloat3(&nvec);
	XMVECTOR X = XMVector3Reflect(i, n);
	XMStoreFloat3(&result, X);
}

inline TVector3 TVector3::Reflect(const TVector3& ivec, const TVector3& nvec) noexcept
{
	using namespace DirectX;
	XMVECTOR i = XMLoadFloat3(&ivec);
	XMVECTOR n = XMLoadFloat3(&nvec);
	XMVECTOR X = XMVector3Reflect(i, n);

	TVector3 result;
	XMStoreFloat3(&result, X);
	return result;
}

inline void TVector3::Refract(const TVector3& ivec, const TVector3& nvec, float refractionIndex, TVector3& result) noexcept
{
	using namespace DirectX;
	XMVECTOR i = XMLoadFloat3(&ivec);
	XMVECTOR n = XMLoadFloat3(&nvec);
	XMVECTOR X = XMVector3Refract(i, n, refractionIndex);
	XMStoreFloat3(&result, X);
}

inline TVector3 TVector3::Refract(const TVector3& ivec, const TVector3& nvec, float refractionIndex) noexcept
{
	using namespace DirectX;
	XMVECTOR i = XMLoadFloat3(&ivec);
	XMVECTOR n = XMLoadFloat3(&nvec);
	XMVECTOR X = XMVector3Refract(i, n, refractionIndex);

	TVector3 result;
	XMStoreFloat3(&result, X);
	return result;
}