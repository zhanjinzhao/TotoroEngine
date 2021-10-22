#include "Math.h"

const TVector3 TVector3::Zero = { 0.f, 0.f, 0.f };
const TVector3 TVector3::One = { 1.f, 1.f, 1.f };
const TVector3 TVector3::UnitX = { 1.f, 0.f, 0.f };
const TVector3 TVector3::UnitY = { 0.f, 1.f, 0.f };
const TVector3 TVector3::UnitZ = { 0.f, 0.f, 1.f };
const TVector3 TVector3::Up = { 0.f, 1.f, 0.f };
const TVector3 TVector3::Down = { 0.f, -1.f, 0.f };
const TVector3 TVector3::Right = { 1.f, 0.f, 0.f };
const TVector3 TVector3::Left = { -1.f, 0.f, 0.f };
const TVector3 TVector3::Forward = { 0.f, 0.f, -1.f };
const TVector3 TVector3::Backward = { 0.f, 0.f, 1.f };

const TVector4 TVector4::Zero = { 0.f, 0.f, 0.f, 0.f };
const TVector4 TVector4::One = { 1.f, 1.f, 1.f, 1.f };
const TVector4 TVector4::UnitX = { 1.f, 0.f, 0.f, 0.f };
const TVector4 TVector4::UnitY = { 0.f, 1.f, 0.f, 0.f };
const TVector4 TVector4::UnitZ = { 0.f, 0.f, 1.f, 0.f };
const TVector4 TVector4::UnitW = { 0.f, 0.f, 0.f, 1.f };

const TMatrix TMatrix::Identity = { 1.f, 0.f, 0.f, 0.f,
								  0.f, 1.f, 0.f, 0.f,
								  0.f, 0.f, 1.f, 0.f,
								  0.f, 0.f, 0.f, 1.f };


const float TMath::Infinity = FLT_MAX;
const float TMath::Pi = 3.1415926535f;