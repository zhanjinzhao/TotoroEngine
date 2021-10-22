#pragma once

//#include "D3D/SimpleMath.h"
#include "Mesh/Color.h"
#include "Math/Math.h"

struct TVertex
{
    TVertex() {}
    TVertex(
        const TVector3& p,
        const TVector3& n,
        const TVector3& t,
        const TVector2& uv) :
        Position(p),
        Normal(n),
        TangentU(t),
        TexC(uv) {}
    TVertex(
        float px, float py, float pz,
        float nx, float ny, float nz,
        float tx, float ty, float tz,
        float u, float v) :
        Position(px, py, pz),
        Normal(nx, ny, nz),
        TangentU(tx, ty, tz),
        TexC(u, v) {}

    TVector3 Position;
    TVector3 Normal;
    TVector3 TangentU;
    TVector2 TexC;
};

struct TPrimitiveVertex
{
    TPrimitiveVertex() {}
    TPrimitiveVertex(
        const TVector3& p,
        const TColor& c) :
        Position(p),
        Color(c) {}

    TPrimitiveVertex(
        float px, float py, float pz,
        float cr, float cg, float cb, float ca) :
        Position(px, py, pz),
        Color(cr, cg, cb, ca) {}

    TVector3 Position;
    TColor Color;
};

struct TSpriteVertex
{
    TSpriteVertex() {}
    TSpriteVertex(
		const TVector3& p,
        const TVector2& uv):
        Position(p),
        TexC(uv) {}

    TVector3 Position;
    TVector2 TexC;
};