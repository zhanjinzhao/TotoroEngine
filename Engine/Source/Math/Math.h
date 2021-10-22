#pragma once

#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Matrix.h"
#include "IntPoint.h"
#include <Windows.h>
#include <cstdint>
#include <limits>


#define MachineEpsilon (std::numeric_limits<float>::epsilon() * 0.5)

class TMath
{
public:

	static const float Infinity;
	static const float Pi;

	static int Log2Int(uint64_t v)
	{
		unsigned long lz = 0;
#if defined(_WIN64)
		_BitScanReverse64(&lz, v);
#else
		if (_BitScanReverse(&lz, v >> 32))
			lz += 32;
		else
			_BitScanReverse(&lz, v & 0xffffffff);
#endif // _WIN64
		return lz;
	}

	static int Log2Int(int64_t v) { return Log2Int((uint64_t)v); }

	static float gamma(int n)
	{
		return float(n * MachineEpsilon) / float(1 - n * MachineEpsilon);
	}

	static float RadiansToDegrees(float Radians)
	{
		return Radians * (180.0f / Pi);
	}

	static float DegreesToRadians(float Degrees)
	{
		return Degrees * (Pi / 180.0f);
	}
};
