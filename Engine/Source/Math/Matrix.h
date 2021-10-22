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
#include <string.h>
#include "Vector3.h"
#include "Vector4.h"

// 4x4 Matrix (use left-handed cooordinates !!!)
struct TMatrix : public DirectX::XMFLOAT4X4
{
	TMatrix() noexcept
		: DirectX::XMFLOAT4X4(1.f, 0, 0, 0,
			0, 1.f, 0, 0,
			0, 0, 1.f, 0,
			0, 0, 0, 1.f) {}
	constexpr TMatrix(float m00, float m01, float m02, float m03,
		float m10, float m11, float m12, float m13,
		float m20, float m21, float m22, float m23,
		float m30, float m31, float m32, float m33) noexcept
		: XMFLOAT4X4(m00, m01, m02, m03,
			m10, m11, m12, m13,
			m20, m21, m22, m23,
			m30, m31, m32, m33) {}
	explicit TMatrix(const TVector3& r0, const TVector3& r1, const TVector3& r2) noexcept
		: XMFLOAT4X4(r0.x, r0.y, r0.z, 0,
			r1.x, r1.y, r1.z, 0,
			r2.x, r2.y, r2.z, 0,
			0, 0, 0, 1.f) {}
	explicit TMatrix(const TVector4& r0, const TVector4& r1, const TVector4& r2, const TVector4& r3) noexcept
		: XMFLOAT4X4(r0.x, r0.y, r0.z, r0.w,
			r1.x, r1.y, r1.z, r1.w,
			r2.x, r2.y, r2.z, r2.w,
			r3.x, r3.y, r3.z, r3.w) {}
	TMatrix(const DirectX::XMFLOAT4X4& M) noexcept { memcpy_s(this, sizeof(float) * 16, &M, sizeof(XMFLOAT4X4)); }
	TMatrix(const DirectX::XMFLOAT3X3& M) noexcept;
	TMatrix(const DirectX::XMFLOAT4X3& M) noexcept;

	explicit TMatrix(_In_reads_(16) const float* pArray) noexcept : XMFLOAT4X4(pArray) {}
	TMatrix(DirectX::CXMMATRIX M) noexcept { DirectX::XMStoreFloat4x4(this, M); }

	TMatrix(const TMatrix&) = default;
	TMatrix& operator=(const TMatrix&) = default;

	TMatrix(TMatrix&&) = default;
	TMatrix& operator=(TMatrix&&) = default;

	operator DirectX::XMMATRIX() const noexcept { return DirectX::XMLoadFloat4x4(this); }

	// Comparison operators
	bool operator == (const TMatrix& M) const noexcept;
	bool operator != (const TMatrix& M) const noexcept;

	// Assignment operators
	TMatrix& operator= (const DirectX::XMFLOAT3X3& M) noexcept;
	TMatrix& operator= (const DirectX::XMFLOAT4X3& M) noexcept;
	TMatrix& operator+= (const TMatrix& M) noexcept;
	TMatrix& operator-= (const TMatrix& M) noexcept;
	TMatrix& operator*= (const TMatrix& M) noexcept;
	TMatrix& operator*= (float S) noexcept;
	TMatrix& operator/= (float S) noexcept;

	TMatrix& operator/= (const TMatrix& M) noexcept;
	// Element-wise divide

	// Unary operators
	TMatrix operator+ () const noexcept { return *this; }
	TMatrix operator- () const noexcept;

	// Properties
	TVector3 Up() const noexcept { return TVector3(_21, _22, _23); }
	void Up(const TVector3& v) noexcept { _21 = v.x; _22 = v.y; _23 = v.z; }

	TVector3 Down() const  noexcept { return TVector3(-_21, -_22, -_23); }
	void Down(const TVector3& v) noexcept { _21 = -v.x; _22 = -v.y; _23 = -v.z; }

	TVector3 Right() const noexcept { return TVector3(_11, _12, _13); }
	void Right(const TVector3& v) noexcept { _11 = v.x; _12 = v.y; _13 = v.z; }

	TVector3 Left() const noexcept { return TVector3(-_11, -_12, -_13); }
	void Left(const TVector3& v) noexcept { _11 = -v.x; _12 = -v.y; _13 = -v.z; }

	TVector3 Forward() const noexcept { return TVector3(-_31, -_32, -_33); }
	void Forward(const TVector3& v) noexcept { _31 = -v.x; _32 = -v.y; _33 = -v.z; }

	TVector3 Backward() const noexcept { return TVector3(_31, _32, _33); }
	void Backward(const TVector3& v) noexcept { _31 = v.x; _32 = v.y; _33 = v.z; }

	TVector3 Translation() const  noexcept { return TVector3(_41, _42, _43); }
	void Translation(const TVector3& v) noexcept { _41 = v.x; _42 = v.y; _43 = v.z; }

	// Matrix operations
	TMatrix Transpose() const noexcept;
	void Transpose(TMatrix& result) const noexcept;

	TMatrix Invert() const noexcept;
	void Invert(TMatrix& result) const noexcept;

	float Determinant() const noexcept;

	// Static functions
	static TMatrix CreateTranslation(const TVector3& position) noexcept;
	static TMatrix CreateTranslation(float x, float y, float z) noexcept;

	static TMatrix CreateScale(const TVector3& scales) noexcept;
	static TMatrix CreateScale(float xs, float ys, float zs) noexcept;
	static TMatrix CreateScale(float scale) noexcept;

	static TMatrix CreateRotationX(float radians) noexcept;
	static TMatrix CreateRotationY(float radians) noexcept;
	static TMatrix CreateRotationZ(float radians) noexcept;

	static TMatrix CreateFromAxisAngle(const TVector3& axis, float angle) noexcept;

	static TMatrix CreatePerspectiveFieldOfView(float fov, float aspectRatio, float nearPlane, float farPlane) noexcept;
	static TMatrix CreatePerspective(float width, float height, float nearPlane, float farPlane) noexcept;
	static TMatrix CreatePerspectiveOffCenter(float left, float right, float bottom, float top, float nearPlane, float farPlane) noexcept;
	static TMatrix CreateOrthographic(float width, float height, float zNearPlane, float zFarPlane) noexcept;
	static TMatrix CreateOrthographicOffCenter(float left, float right, float bottom, float top, float zNearPlane, float zFarPlane) noexcept;

	static TMatrix CreateLookAt(const TVector3& position, const TVector3& target, const TVector3& up) noexcept;
	static TMatrix CreateWorld(const TVector3& position, const TVector3& forward, const TVector3& up) noexcept;

	static TMatrix CreateFromYawPitchRoll(float yaw, float pitch, float roll) noexcept;

	static void Lerp(const TMatrix& M1, const TMatrix& M2, float t, TMatrix& result) noexcept;
	static TMatrix Lerp(const TMatrix& M1, const TMatrix& M2, float t) noexcept;

	TVector3 Transform(const TVector3& v) noexcept;
	TVector3 TransformNormal(const TVector3& v) noexcept;


	// Constants
	static const TMatrix Identity;
};


//------------------------------------------------------------------------------
// Comparision operators
//------------------------------------------------------------------------------

inline bool TMatrix::operator == (const TMatrix& M) const noexcept
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&_11));
	XMVECTOR x2 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&_21));
	XMVECTOR x3 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&_31));
	XMVECTOR x4 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&_41));

	XMVECTOR y1 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._11));
	XMVECTOR y2 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._21));
	XMVECTOR y3 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._31));
	XMVECTOR y4 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._41));

	return (XMVector4Equal(x1, y1)
		&& XMVector4Equal(x2, y2)
		&& XMVector4Equal(x3, y3)
		&& XMVector4Equal(x4, y4)) != 0;
}

inline bool TMatrix::operator != (const TMatrix& M) const noexcept
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&_11));
	XMVECTOR x2 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&_21));
	XMVECTOR x3 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&_31));
	XMVECTOR x4 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&_41));

	XMVECTOR y1 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._11));
	XMVECTOR y2 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._21));
	XMVECTOR y3 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._31));
	XMVECTOR y4 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._41));

	return (XMVector4NotEqual(x1, y1)
		|| XMVector4NotEqual(x2, y2)
		|| XMVector4NotEqual(x3, y3)
		|| XMVector4NotEqual(x4, y4)) != 0;
}

//------------------------------------------------------------------------------
// Assignment operators
//------------------------------------------------------------------------------

inline TMatrix::TMatrix(const DirectX::XMFLOAT3X3& M) noexcept
{
	_11 = M._11; _12 = M._12; _13 = M._13; _14 = 0.f;
	_21 = M._21; _22 = M._22; _23 = M._23; _24 = 0.f;
	_31 = M._31; _32 = M._32; _33 = M._33; _34 = 0.f;
	_41 = 0.f;   _42 = 0.f;   _43 = 0.f;   _44 = 1.f;
}

inline TMatrix::TMatrix(const DirectX::XMFLOAT4X3& M) noexcept
{
	_11 = M._11; _12 = M._12; _13 = M._13; _14 = 0.f;
	_21 = M._21; _22 = M._22; _23 = M._23; _24 = 0.f;
	_31 = M._31; _32 = M._32; _33 = M._33; _34 = 0.f;
	_41 = M._41; _42 = M._42; _43 = M._43; _44 = 1.f;
}

inline TMatrix& TMatrix::operator= (const DirectX::XMFLOAT3X3& M) noexcept
{
	_11 = M._11; _12 = M._12; _13 = M._13; _14 = 0.f;
	_21 = M._21; _22 = M._22; _23 = M._23; _24 = 0.f;
	_31 = M._31; _32 = M._32; _33 = M._33; _34 = 0.f;
	_41 = 0.f;   _42 = 0.f;   _43 = 0.f;   _44 = 1.f;
	return *this;
}

inline TMatrix& TMatrix::operator= (const DirectX::XMFLOAT4X3& M) noexcept
{
	_11 = M._11; _12 = M._12; _13 = M._13; _14 = 0.f;
	_21 = M._21; _22 = M._22; _23 = M._23; _24 = 0.f;
	_31 = M._31; _32 = M._32; _33 = M._33; _34 = 0.f;
	_41 = M._41; _42 = M._42; _43 = M._43; _44 = 1.f;
	return *this;
}

inline TMatrix& TMatrix::operator+= (const TMatrix& M) noexcept
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&_11));
	XMVECTOR x2 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&_21));
	XMVECTOR x3 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&_31));
	XMVECTOR x4 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&_41));

	XMVECTOR y1 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._11));
	XMVECTOR y2 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._21));
	XMVECTOR y3 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._31));
	XMVECTOR y4 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._41));

	x1 = XMVectorAdd(x1, y1);
	x2 = XMVectorAdd(x2, y2);
	x3 = XMVectorAdd(x3, y3);
	x4 = XMVectorAdd(x4, y4);

	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&_11), x1);
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&_21), x2);
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&_31), x3);
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&_41), x4);
	return *this;
}

inline TMatrix& TMatrix::operator-= (const TMatrix& M) noexcept
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&_11));
	XMVECTOR x2 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&_21));
	XMVECTOR x3 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&_31));
	XMVECTOR x4 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&_41));

	XMVECTOR y1 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._11));
	XMVECTOR y2 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._21));
	XMVECTOR y3 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._31));
	XMVECTOR y4 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._41));

	x1 = XMVectorSubtract(x1, y1);
	x2 = XMVectorSubtract(x2, y2);
	x3 = XMVectorSubtract(x3, y3);
	x4 = XMVectorSubtract(x4, y4);

	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&_11), x1);
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&_21), x2);
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&_31), x3);
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&_41), x4);
	return *this;
}

inline TMatrix& TMatrix::operator*= (const TMatrix& M) noexcept
{
	using namespace DirectX;
	XMMATRIX M1 = XMLoadFloat4x4(this);
	XMMATRIX M2 = XMLoadFloat4x4(&M);
	XMMATRIX X = XMMatrixMultiply(M1, M2);
	DirectX::XMStoreFloat4x4(this, X);
	return *this;
}

inline TMatrix& TMatrix::operator*= (float S) noexcept
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&_11));
	XMVECTOR x2 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&_21));
	XMVECTOR x3 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&_31));
	XMVECTOR x4 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&_41));

	x1 = XMVectorScale(x1, S);
	x2 = XMVectorScale(x2, S);
	x3 = XMVectorScale(x3, S);
	x4 = XMVectorScale(x4, S);

	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&_11), x1);
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&_21), x2);
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&_31), x3);
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&_41), x4);
	return *this;
}

inline TMatrix& TMatrix::operator/= (float S) noexcept
{
	using namespace DirectX;
	assert(S != 0.f);
	XMVECTOR x1 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&_11));
	XMVECTOR x2 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&_21));
	XMVECTOR x3 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&_31));
	XMVECTOR x4 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&_41));

	float rs = 1.f / S;

	x1 = XMVectorScale(x1, rs);
	x2 = XMVectorScale(x2, rs);
	x3 = XMVectorScale(x3, rs);
	x4 = XMVectorScale(x4, rs);

	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&_11), x1);
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&_21), x2);
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&_31), x3);
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&_41), x4);
	return *this;
}

inline TMatrix& TMatrix::operator/= (const TMatrix& M) noexcept
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&_11));
	XMVECTOR x2 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&_21));
	XMVECTOR x3 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&_31));
	XMVECTOR x4 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&_41));

	XMVECTOR y1 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._11));
	XMVECTOR y2 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._21));
	XMVECTOR y3 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._31));
	XMVECTOR y4 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._41));

	x1 = XMVectorDivide(x1, y1);
	x2 = XMVectorDivide(x2, y2);
	x3 = XMVectorDivide(x3, y3);
	x4 = XMVectorDivide(x4, y4);

	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&_11), x1);
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&_21), x2);
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&_31), x3);
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&_41), x4);
	return *this;
}

//------------------------------------------------------------------------------
// Urnary operators
//------------------------------------------------------------------------------

inline TMatrix TMatrix::operator- () const noexcept
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&_11));
	XMVECTOR v2 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&_21));
	XMVECTOR v3 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&_31));
	XMVECTOR v4 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&_41));

	v1 = XMVectorNegate(v1);
	v2 = XMVectorNegate(v2);
	v3 = XMVectorNegate(v3);
	v4 = XMVectorNegate(v4);

	TMatrix R;
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R._11), v1);
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R._21), v2);
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R._31), v3);
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R._41), v4);
	return R;
}

//------------------------------------------------------------------------------
// Binary operators
//------------------------------------------------------------------------------

inline TMatrix operator+ (const TMatrix& M1, const TMatrix& M2) noexcept
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M1._11));
	XMVECTOR x2 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M1._21));
	XMVECTOR x3 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M1._31));
	XMVECTOR x4 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M1._41));

	XMVECTOR y1 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M2._11));
	XMVECTOR y2 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M2._21));
	XMVECTOR y3 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M2._31));
	XMVECTOR y4 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M2._41));

	x1 = XMVectorAdd(x1, y1);
	x2 = XMVectorAdd(x2, y2);
	x3 = XMVectorAdd(x3, y3);
	x4 = XMVectorAdd(x4, y4);

	TMatrix R;
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R._11), x1);
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R._21), x2);
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R._31), x3);
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R._41), x4);
	return R;
}

inline TMatrix operator- (const TMatrix& M1, const TMatrix& M2) noexcept
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M1._11));
	XMVECTOR x2 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M1._21));
	XMVECTOR x3 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M1._31));
	XMVECTOR x4 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M1._41));

	XMVECTOR y1 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M2._11));
	XMVECTOR y2 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M2._21));
	XMVECTOR y3 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M2._31));
	XMVECTOR y4 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M2._41));

	x1 = XMVectorSubtract(x1, y1);
	x2 = XMVectorSubtract(x2, y2);
	x3 = XMVectorSubtract(x3, y3);
	x4 = XMVectorSubtract(x4, y4);

	TMatrix R;
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R._11), x1);
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R._21), x2);
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R._31), x3);
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R._41), x4);
	return R;
}

inline TMatrix operator* (const TMatrix& M1, const TMatrix& M2) noexcept
{
	using namespace DirectX;
	XMMATRIX m1 = XMLoadFloat4x4(&M1);
	XMMATRIX m2 = XMLoadFloat4x4(&M2);
	XMMATRIX X = XMMatrixMultiply(m1, m2);

	TMatrix R;
	DirectX::XMStoreFloat4x4(&R, X);
	return R;
}

inline TMatrix operator* (const TMatrix& M, float S) noexcept
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._11));
	XMVECTOR x2 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._21));
	XMVECTOR x3 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._31));
	XMVECTOR x4 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._41));

	x1 = XMVectorScale(x1, S);
	x2 = XMVectorScale(x2, S);
	x3 = XMVectorScale(x3, S);
	x4 = XMVectorScale(x4, S);

	TMatrix R;
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R._11), x1);
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R._21), x2);
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R._31), x3);
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R._41), x4);
	return R;
}

inline TMatrix operator/ (const TMatrix& M, float S) noexcept
{
	using namespace DirectX;
	assert(S != 0.f);

	XMVECTOR x1 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._11));
	XMVECTOR x2 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._21));
	XMVECTOR x3 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._31));
	XMVECTOR x4 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._41));

	float rs = 1.f / S;

	x1 = XMVectorScale(x1, rs);
	x2 = XMVectorScale(x2, rs);
	x3 = XMVectorScale(x3, rs);
	x4 = XMVectorScale(x4, rs);

	TMatrix R;
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R._11), x1);
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R._21), x2);
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R._31), x3);
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R._41), x4);
	return R;
}

inline TMatrix operator/ (const TMatrix& M1, const TMatrix& M2) noexcept
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M1._11));
	XMVECTOR x2 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M1._21));
	XMVECTOR x3 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M1._31));
	XMVECTOR x4 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M1._41));

	XMVECTOR y1 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M2._11));
	XMVECTOR y2 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M2._21));
	XMVECTOR y3 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M2._31));
	XMVECTOR y4 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M2._41));

	x1 = XMVectorDivide(x1, y1);
	x2 = XMVectorDivide(x2, y2);
	x3 = XMVectorDivide(x3, y3);
	x4 = XMVectorDivide(x4, y4);

	TMatrix R;
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R._11), x1);
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R._21), x2);
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R._31), x3);
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R._41), x4);
	return R;
}

inline TMatrix operator* (float S, const TMatrix& M) noexcept
{
	using namespace DirectX;

	XMVECTOR x1 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._11));
	XMVECTOR x2 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._21));
	XMVECTOR x3 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._31));
	XMVECTOR x4 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._41));

	x1 = XMVectorScale(x1, S);
	x2 = XMVectorScale(x2, S);
	x3 = XMVectorScale(x3, S);
	x4 = XMVectorScale(x4, S);

	TMatrix R;
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R._11), x1);
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R._21), x2);
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R._31), x3);
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R._41), x4);
	return R;
}

//------------------------------------------------------------------------------
// TMatrix operations
//------------------------------------------------------------------------------
inline TMatrix TMatrix::Transpose() const noexcept
{
	using namespace DirectX;
	XMMATRIX M = XMLoadFloat4x4(this);
	TMatrix R;
	XMStoreFloat4x4(&R, XMMatrixTranspose(M));
	return R;
}

inline void TMatrix::Transpose(TMatrix& result) const noexcept
{
	using namespace DirectX;
	XMMATRIX M = XMLoadFloat4x4(this);
	XMStoreFloat4x4(&result, XMMatrixTranspose(M));
}

inline TMatrix TMatrix::Invert() const noexcept
{
	using namespace DirectX;
	XMMATRIX M = XMLoadFloat4x4(this);
	TMatrix R;
	XMVECTOR det;
	XMStoreFloat4x4(&R, XMMatrixInverse(&det, M));
	return R;
}

inline void TMatrix::Invert(TMatrix& result) const noexcept
{
	using namespace DirectX;
	XMMATRIX M = XMLoadFloat4x4(this);
	XMVECTOR det;
	XMStoreFloat4x4(&result, XMMatrixInverse(&det, M));
}

inline float TMatrix::Determinant() const noexcept
{
	using namespace DirectX;
	XMMATRIX M = XMLoadFloat4x4(this);
	return XMVectorGetX(XMMatrixDeterminant(M));
}

//------------------------------------------------------------------------------
// Static functions
//------------------------------------------------------------------------------

inline TMatrix TMatrix::CreateTranslation(const TVector3& position) noexcept
{
	using namespace DirectX;
	TMatrix R;
	XMStoreFloat4x4(&R, XMMatrixTranslation(position.x, position.y, position.z));
	return R;
}

inline TMatrix TMatrix::CreateTranslation(float x, float y, float z) noexcept
{
	using namespace DirectX;
	TMatrix R;
	XMStoreFloat4x4(&R, XMMatrixTranslation(x, y, z));
	return R;
}

inline TMatrix TMatrix::CreateScale(const TVector3& scales) noexcept
{
	using namespace DirectX;
	TMatrix R;
	XMStoreFloat4x4(&R, XMMatrixScaling(scales.x, scales.y, scales.z));
	return R;
}

inline TMatrix TMatrix::CreateScale(float xs, float ys, float zs) noexcept
{
	using namespace DirectX;
	TMatrix R;
	XMStoreFloat4x4(&R, XMMatrixScaling(xs, ys, zs));
	return R;
}

inline TMatrix TMatrix::CreateScale(float scale) noexcept
{
	using namespace DirectX;
	TMatrix R;
	XMStoreFloat4x4(&R, XMMatrixScaling(scale, scale, scale));
	return R;
}

inline TMatrix TMatrix::CreateRotationX(float radians) noexcept
{
	using namespace DirectX;
	TMatrix R;
	XMStoreFloat4x4(&R, XMMatrixRotationX(radians));
	return R;
}

inline TMatrix TMatrix::CreateRotationY(float radians) noexcept
{
	using namespace DirectX;
	TMatrix R;
	XMStoreFloat4x4(&R, XMMatrixRotationY(radians));
	return R;
}

inline TMatrix TMatrix::CreateRotationZ(float radians) noexcept
{
	using namespace DirectX;
	TMatrix R;
	XMStoreFloat4x4(&R, XMMatrixRotationZ(radians));
	return R;
}

inline TMatrix TMatrix::CreateFromAxisAngle(const TVector3& axis, float angle) noexcept
{
	using namespace DirectX;
	TMatrix R;
	XMVECTOR a = XMLoadFloat3(&axis);
	XMStoreFloat4x4(&R, XMMatrixRotationAxis(a, angle));
	return R;
}

inline TMatrix TMatrix::CreatePerspectiveFieldOfView(float fov, float aspectRatio, float nearPlane, float farPlane) noexcept
{
	using namespace DirectX;
	TMatrix R;
	XMStoreFloat4x4(&R, XMMatrixPerspectiveFovLH(fov, aspectRatio, nearPlane, farPlane));

	return R;
}

inline TMatrix TMatrix::CreatePerspective(float width, float height, float nearPlane, float farPlane) noexcept
{
	using namespace DirectX;
	TMatrix R;
	XMStoreFloat4x4(&R, XMMatrixPerspectiveLH(width, height, nearPlane, farPlane));
	return R;
}

inline TMatrix TMatrix::CreatePerspectiveOffCenter(float left, float right, float bottom, float top, float nearPlane, float farPlane) noexcept
{
	using namespace DirectX;
	TMatrix R;
	XMStoreFloat4x4(&R, XMMatrixPerspectiveOffCenterLH(left, right, bottom, top, nearPlane, farPlane));
	return R;
}

inline TMatrix TMatrix::CreateOrthographic(float width, float height, float zNearPlane, float zFarPlane) noexcept
{
	using namespace DirectX;
	TMatrix R;
	XMStoreFloat4x4(&R, XMMatrixOrthographicLH(width, height, zNearPlane, zFarPlane));
	return R;
}

inline TMatrix TMatrix::CreateOrthographicOffCenter(float left, float right, float bottom, float top, float zNearPlane, float zFarPlane) noexcept
{
	using namespace DirectX;
	TMatrix R;
	XMStoreFloat4x4(&R, XMMatrixOrthographicOffCenterLH(left, right, bottom, top, zNearPlane, zFarPlane));

	return R;
}

inline TMatrix TMatrix::CreateLookAt(const TVector3& eye, const TVector3& target, const TVector3& up) noexcept
{
	using namespace DirectX;
	TMatrix R;
	XMVECTOR eyev = XMLoadFloat3(&eye);
	XMVECTOR targetv = XMLoadFloat3(&target);
	XMVECTOR upv = XMLoadFloat3(&up);

	XMStoreFloat4x4(&R, XMMatrixLookAtLH(eyev, targetv, upv));

	return R;
}

inline TMatrix TMatrix::CreateWorld(const TVector3& position, const TVector3& forward, const TVector3& up) noexcept
{
	using namespace DirectX;
	XMVECTOR zaxis = XMVector3Normalize(XMVectorNegate(XMLoadFloat3(&forward)));
	XMVECTOR yaxis = XMLoadFloat3(&up);
	XMVECTOR xaxis = XMVector3Normalize(XMVector3Cross(yaxis, zaxis));
	yaxis = XMVector3Cross(zaxis, xaxis);

	TMatrix R;
	XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&R._11), xaxis);
	XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&R._21), yaxis);
	XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&R._31), zaxis);
	R._14 = R._24 = R._34 = 0.f;
	R._41 = position.x; R._42 = position.y; R._43 = position.z;
	R._44 = 1.f;
	return R;
}

inline TMatrix TMatrix::CreateFromYawPitchRoll(float yaw, float pitch, float roll) noexcept
{
	using namespace DirectX;
	TMatrix R;
	XMStoreFloat4x4(&R, XMMatrixRotationRollPitchYaw(pitch, yaw, roll));
	return R;
}

inline void TMatrix::Lerp(const TMatrix& M1, const TMatrix& M2, float t, TMatrix& result) noexcept
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M1._11));
	XMVECTOR x2 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M1._21));
	XMVECTOR x3 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M1._31));
	XMVECTOR x4 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M1._41));

	XMVECTOR y1 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M2._11));
	XMVECTOR y2 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M2._21));
	XMVECTOR y3 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M2._31));
	XMVECTOR y4 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M2._41));

	x1 = XMVectorLerp(x1, y1, t);
	x2 = XMVectorLerp(x2, y2, t);
	x3 = XMVectorLerp(x3, y3, t);
	x4 = XMVectorLerp(x4, y4, t);

	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result._11), x1);
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result._21), x2);
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result._31), x3);
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result._41), x4);
}

inline TMatrix TMatrix::Lerp(const TMatrix& M1, const TMatrix& M2, float t) noexcept
{
	using namespace DirectX;
	XMVECTOR x1 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M1._11));
	XMVECTOR x2 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M1._21));
	XMVECTOR x3 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M1._31));
	XMVECTOR x4 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M1._41));

	XMVECTOR y1 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M2._11));
	XMVECTOR y2 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M2._21));
	XMVECTOR y3 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M2._31));
	XMVECTOR y4 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M2._41));

	x1 = XMVectorLerp(x1, y1, t);
	x2 = XMVectorLerp(x2, y2, t);
	x3 = XMVectorLerp(x3, y3, t);
	x4 = XMVectorLerp(x4, y4, t);

	TMatrix result;
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result._11), x1);
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result._21), x2);
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result._31), x3);
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result._41), x4);
	return result;
}

inline TVector3 TMatrix::Transform(const TVector3& v) noexcept
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat3(&v);
	XMMATRIX M = XMLoadFloat4x4(this);
	XMVECTOR X = XMVector3TransformCoord(v1, M);

	TVector3 result;
	XMStoreFloat3(&result, X);
	return result;
}

inline TVector3 TMatrix::TransformNormal(const TVector3& v) noexcept
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat3(&v);
	XMMATRIX M = XMLoadFloat4x4(this);
	XMVECTOR X = XMVector3TransformNormal(v1, M);

	TVector3 result;
	XMStoreFloat3(&result, X);
	return result;
}