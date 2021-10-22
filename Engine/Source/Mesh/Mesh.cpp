#include "Mesh.h"
#include "File/FileHelpers.h"
#include <fstream>

TMesh::TMesh()
{
	InputLayoutName = "DefaultInputLayout";
}


void TMesh::CreateBox(float width, float height, float depth, uint32 numSubdivisions)
{
	//
	// Create the Vertices.
	//
	TVertex v[24];

	float w2 = 0.5f * width;
	float h2 = 0.5f * height;
	float d2 = 0.5f * depth;

	// Fill in the front face vertex data.
	v[0] = TVertex(-w2, -h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	v[1] = TVertex(-w2, +h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	v[2] = TVertex(+w2, +h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
	v[3] = TVertex(+w2, -h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);

	// Fill in the back face vertex data.
	v[4] = TVertex(-w2, -h2, +d2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f);
	v[5] = TVertex(+w2, -h2, +d2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	v[6] = TVertex(+w2, +h2, +d2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	v[7] = TVertex(-w2, +h2, +d2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f);

	// Fill in the top face vertex data.
	v[8] = TVertex(-w2, +h2, -d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	v[9] = TVertex(-w2, +h2, +d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	v[10] = TVertex(+w2, +h2, +d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
	v[11] = TVertex(+w2, +h2, -d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);

	// Fill in the bottom face vertex data.
	v[12] = TVertex(-w2, -h2, -d2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f);
	v[13] = TVertex(+w2, -h2, -d2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	v[14] = TVertex(+w2, -h2, +d2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	v[15] = TVertex(-w2, -h2, +d2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f);

	// Fill in the left face vertex data.
	v[16] = TVertex(-w2, -h2, +d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
	v[17] = TVertex(-w2, +h2, +d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
	v[18] = TVertex(-w2, +h2, -d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);
	v[19] = TVertex(-w2, -h2, -d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f);

	// Fill in the right face vertex data.
	v[20] = TVertex(+w2, -h2, -d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f);
	v[21] = TVertex(+w2, +h2, -d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
	v[22] = TVertex(+w2, +h2, +d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f);
	v[23] = TVertex(+w2, -h2, +d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);

	Vertices.assign(&v[0], &v[24]);

	//
	// Create the indices.
	//

	uint32 i[36];

	// Fill in the front face index data
	i[0] = 0; i[1] = 1; i[2] = 2;
	i[3] = 0; i[4] = 2; i[5] = 3;

	// Fill in the back face index data
	i[6] = 4; i[7] = 5; i[8] = 6;
	i[9] = 4; i[10] = 6; i[11] = 7;

	// Fill in the top face index data
	i[12] = 8; i[13] = 9; i[14] = 10;
	i[15] = 8; i[16] = 10; i[17] = 11;

	// Fill in the bottom face index data
	i[18] = 12; i[19] = 13; i[20] = 14;
	i[21] = 12; i[22] = 14; i[23] = 15;

	// Fill in the left face index data
	i[24] = 16; i[25] = 17; i[26] = 18;
	i[27] = 16; i[28] = 18; i[29] = 19;

	// Fill in the right face index data
	i[30] = 20; i[31] = 21; i[32] = 22;
	i[33] = 20; i[34] = 22; i[35] = 23;

	Indices32.assign(&i[0], &i[36]);


	// Put a cap on the number of subdivisions.
	numSubdivisions = std::min<uint32>(numSubdivisions, 6u);
	for (uint32 i = 0; i < numSubdivisions; ++i)
		Subdivide();


	GenerateIndices16();
}

void TMesh::Subdivide()
{
	// Save a copy of the input geometry.
	std::vector<TVertex> VerticesCopy = Vertices;
	std::vector<uint32> Indices32Copy = Indices32;


	Vertices.resize(0);
	Indices32.resize(0);

	//       v1
	//       *
	//      / \
	//     /   \
	//  m0*-----*m1
	//   / \   / \
	//  /   \ /   \
	// *-----*-----*
	// v0    m2     v2

	uint32 numTris = (uint32)Indices32Copy.size() / 3;
	for (uint32 i = 0; i < numTris; ++i)
	{
		TVertex v0 = VerticesCopy[Indices32Copy[i * 3 + 0]];
		TVertex v1 = VerticesCopy[Indices32Copy[i * 3 + 1]];
		TVertex v2 = VerticesCopy[Indices32Copy[i * 3 + 2]];

		//
		// Generate the midpoints.
		//

		TVertex m0 = MidPoint(v0, v1);
		TVertex m1 = MidPoint(v1, v2);
		TVertex m2 = MidPoint(v0, v2);

		//
		// Add new geometry.
		//

		Vertices.push_back(v0); // 0
		Vertices.push_back(v1); // 1
		Vertices.push_back(v2); // 2
		Vertices.push_back(m0); // 3
		Vertices.push_back(m1); // 4
		Vertices.push_back(m2); // 5

		Indices32.push_back(i * 6 + 0);
		Indices32.push_back(i * 6 + 3);
		Indices32.push_back(i * 6 + 5);

		Indices32.push_back(i * 6 + 3);
		Indices32.push_back(i * 6 + 4);
		Indices32.push_back(i * 6 + 5);

		Indices32.push_back(i * 6 + 5);
		Indices32.push_back(i * 6 + 4);
		Indices32.push_back(i * 6 + 2);

		Indices32.push_back(i * 6 + 3);
		Indices32.push_back(i * 6 + 1);
		Indices32.push_back(i * 6 + 4);
	}
}

TVertex TMesh::MidPoint(const TVertex& v0, const TVertex& v1)
{
	// Compute the midpoints of all the attributes.  Vectors need to be normalized
	// since linear interpolating can make them not unit length.  
	TVector3 pos = 0.5f * (v0.Position + v1.Position);
	TVector3 normal = 0.5f * (v0.Normal + v1.Normal);
	normal.Normalize();
	TVector3 tangent = 0.5f * (v0.TangentU + v1.TangentU);
	tangent.Normalize();
	TVector2 tex = 0.5f * (v0.TexC + v1.TexC);


	TVertex v(pos, normal, tangent, tex);

	return v;
}

void TMesh::CreateSphere(float radius, uint32 sliceCount, uint32 stackCount)
{
	//
	// Compute the vertices stating at the top pole and moving down the stacks.
	//

	// Poles: note that there will be texture coordinate distortion as there is
	// not a unique point on the texture map to assign to the pole when mapping
	// a rectangular texture onto a sphere.
	TVertex topVertex(0.0f, +radius, 0.0f, 0.0f, +1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	TVertex bottomVertex(0.0f, -radius, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);

	Vertices.push_back(topVertex);

	float phiStep = DirectX::XM_PI / stackCount;
	float thetaStep = 2.0f * DirectX::XM_PI / sliceCount;

	// Compute vertices for each stack ring (do not count the poles as rings).
	for (uint32 i = 1; i <= stackCount - 1; ++i)
	{
		float phi = i * phiStep;

		// Vertices of ring.
		for (uint32 j = 0; j <= sliceCount; ++j)
		{
			float theta = j * thetaStep;

			TVertex v;

			// spherical to cartesian
			v.Position.x = radius * sinf(phi) * cosf(theta);
			v.Position.y = radius * cosf(phi);
			v.Position.z = radius * sinf(phi) * sinf(theta);

			// Partial derivative of P with respect to theta
			v.TangentU.x = -radius * sinf(phi) * sinf(theta);
			v.TangentU.y = 0.0f;
			v.TangentU.z = +radius * sinf(phi) * cosf(theta);
			v.TangentU.Normalize();

			v.Normal = v.Position;
			v.Normal.Normalize();

			v.TexC.x = theta / DirectX::XM_2PI;
			v.TexC.y = phi / DirectX::XM_PI;

			Vertices.push_back(v);
		}
	}

	Vertices.push_back(bottomVertex);

	//
	// Compute indices for top stack.  The top stack was written first to the vertex buffer
	// and connects the top pole to the first ring.
	//

	for (uint32 i = 1; i <= sliceCount; ++i)
	{
		Indices32.push_back(0);
		Indices32.push_back(i + 1);
		Indices32.push_back(i);
	}

	//
	// Compute indices for inner stacks (not connected to poles).
	//

	// Offset the indices to the index of the first vertex in the first ring.
	// This is just skipping the top pole vertex.
	uint32 baseIndex = 1;
	uint32 ringVertexCount = sliceCount + 1;
	for (uint32 i = 0; i < stackCount - 2; ++i)
	{
		for (uint32 j = 0; j < sliceCount; ++j)
		{
			Indices32.push_back(baseIndex + i * ringVertexCount + j);
			Indices32.push_back(baseIndex + i * ringVertexCount + j + 1);
			Indices32.push_back(baseIndex + (i + 1) * ringVertexCount + j);

			Indices32.push_back(baseIndex + (i + 1) * ringVertexCount + j);
			Indices32.push_back(baseIndex + i * ringVertexCount + j + 1);
			Indices32.push_back(baseIndex + (i + 1) * ringVertexCount + j + 1);
		}
	}

	//
	// Compute indices for bottom stack.  The bottom stack was written last to the vertex buffer
	// and connects the bottom pole to the bottom ring.
	//

	// South pole vertex was added last.
	uint32 southPoleIndex = (uint32)Vertices.size() - 1;

	// Offset the indices to the index of the first vertex in the last ring.
	baseIndex = southPoleIndex - ringVertexCount;

	for (uint32 i = 0; i < sliceCount; ++i)
	{
		Indices32.push_back(southPoleIndex);
		Indices32.push_back(baseIndex + i);
		Indices32.push_back(baseIndex + i + 1);
	}

	GenerateIndices16();
}

void TMesh::CreateCylinder(float bottomRadius, float topRadius, float height, uint32 sliceCount, uint32 stackCount)
{
	//
	// Build Stacks.
	// 

	float stackHeight = height / stackCount;

	// Amount to increment radius as we move up each stack level from bottom to top.
	float radiusStep = (topRadius - bottomRadius) / stackCount;

	uint32 ringCount = stackCount + 1;

	// Compute vertices for each stack ring starting at the bottom and moving up.
	for (uint32 i = 0; i < ringCount; ++i)
	{
		float y = -0.5f * height + i * stackHeight;
		float r = bottomRadius + i * radiusStep;

		// vertices of ring
		float dTheta = 2.0f * DirectX::XM_PI / sliceCount;
		for (uint32 j = 0; j <= sliceCount; ++j)
		{
			TVertex vertex;

			float c = cosf(j * dTheta);
			float s = sinf(j * dTheta);

			vertex.Position = TVector3(r * c, y, r * s);

			vertex.TexC.x = (float)j / sliceCount;
			vertex.TexC.y = 1.0f - (float)i / stackCount;

			// Cylinder can be parameterized as follows, where we introduce v
			// parameter that goes in the same direction as the v tex-coord
			// so that the bitangent goes in the same direction as the v tex-coord.
			//   Let r0 be the bottom radius and let r1 be the top radius.
			//   y(v) = h - hv for v in [0,1].
			//   r(v) = r1 + (r0-r1)v
			//
			//   x(t, v) = r(v)*cos(t)
			//   y(t, v) = h - hv
			//   z(t, v) = r(v)*sin(t)
			// 
			//  dx/dt = -r(v)*sin(t)
			//  dy/dt = 0
			//  dz/dt = +r(v)*cos(t)
			//
			//  dx/dv = (r0-r1)*cos(t)
			//  dy/dv = -h
			//  dz/dv = (r0-r1)*sin(t)

			// This is unit length.
			vertex.TangentU = TVector3(-s, 0.0f, c);

			float dr = bottomRadius - topRadius;
			TVector3 bitangent(dr * c, -height, dr * s);

			vertex.Normal = vertex.TangentU.Cross(bitangent);
			vertex.Normal.Normalize();

			Vertices.push_back(vertex);
		}
	}

	// Add one because we duplicate the first and last vertex per ring
	// since the texture coordinates are different.
	uint32 ringVertexCount = sliceCount + 1;

	// Compute indices for each stack.
	for (uint32 i = 0; i < stackCount; ++i)
	{
		for (uint32 j = 0; j < sliceCount; ++j)
		{
			Indices32.push_back(i * ringVertexCount + j);
			Indices32.push_back((i + 1) * ringVertexCount + j);
			Indices32.push_back((i + 1) * ringVertexCount + j + 1);

			Indices32.push_back(i * ringVertexCount + j);
			Indices32.push_back((i + 1) * ringVertexCount + j + 1);
			Indices32.push_back(i * ringVertexCount + j + 1);
		}
	}

	BuildCylinderTopCap(bottomRadius, topRadius, height, sliceCount, stackCount);
	BuildCylinderBottomCap(bottomRadius, topRadius, height, sliceCount, stackCount);

	GenerateIndices16();
}

void TMesh::BuildCylinderTopCap(float bottomRadius, float topRadius, float height, uint32 sliceCount, uint32 stackCount)
{
	uint32 baseIndex = (uint32)Vertices.size();

	float y = 0.5f * height;
	float dTheta = 2.0f * DirectX::XM_PI / sliceCount;

	// Duplicate cap ring vertices because the texture coordinates and normals differ.
	for (uint32 i = 0; i <= sliceCount; ++i)
	{
		float x = topRadius * cosf(i * dTheta);
		float z = topRadius * sinf(i * dTheta);

		// Scale down by the height to try and make top cap texture coord area
		// proportional to base.
		float u = x / height + 0.5f;
		float v = z / height + 0.5f;

		Vertices.push_back(TVertex(x, y, z, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, u, v));
	}

	// Cap center vertex.
	Vertices.push_back(TVertex(0.0f, y, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.5f, 0.5f));

	// Index of center vertex.
	uint32 centerIndex = (uint32)Vertices.size() - 1;

	for (uint32 i = 0; i < sliceCount; ++i)
	{
		Indices32.push_back(centerIndex);
		Indices32.push_back(baseIndex + i + 1);
		Indices32.push_back(baseIndex + i);
	}
}

void TMesh::BuildCylinderBottomCap(float bottomRadius, float topRadius, float height, uint32 sliceCount, uint32 stackCount)
{
	// 
	// Build bottom cap.
	//

	uint32 baseIndex = (uint32)Vertices.size();
	float y = -0.5f * height;

	// vertices of ring
	float dTheta = 2.0f * DirectX::XM_PI / sliceCount;
	for (uint32 i = 0; i <= sliceCount; ++i)
	{
		float x = bottomRadius * cosf(i * dTheta);
		float z = bottomRadius * sinf(i * dTheta);

		// Scale down by the height to try and make top cap texture coord area
		// proportional to base.
		float u = x / height + 0.5f;
		float v = z / height + 0.5f;

		Vertices.push_back(TVertex(x, y, z, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, u, v));
	}

	// Cap center vertex.
	Vertices.push_back(TVertex(0.0f, y, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.5f, 0.5f));

	// Cache the index of center vertex.
	uint32 centerIndex = (uint32)Vertices.size() - 1;

	for (uint32 i = 0; i < sliceCount; ++i)
	{
		Indices32.push_back(centerIndex);
		Indices32.push_back(baseIndex + i);
		Indices32.push_back(baseIndex + i + 1);
	}
}

void TMesh::CreateGrid(float width, float depth, uint32 m, uint32 n)
{
	uint32 vertexCount = m * n;
	uint32 faceCount = (m - 1) * (n - 1) * 2;

	//
	// Create the vertices.
	//

	float halfWidth = 0.5f * width;
	float halfDepth = 0.5f * depth;

	float dx = width / (n - 1);
	float dz = depth / (m - 1);

	float du = 1.0f / (n - 1);
	float dv = 1.0f / (m - 1);

	Vertices.resize(vertexCount);
	for (uint32 i = 0; i < m; ++i)
	{
		float z = halfDepth - i * dz;
		for (uint32 j = 0; j < n; ++j)
		{
			float x = -halfWidth + j * dx;

			Vertices[i * n + j].Position = TVector3(x, 0.0f, z);
			Vertices[i * n + j].Normal = TVector3(0.0f, 1.0f, 0.0f);
			Vertices[i * n + j].TangentU = TVector3(1.0f, 0.0f, 0.0f);

			// Stretch texture over grid.
			Vertices[i * n + j].TexC.x = j * du;
			Vertices[i * n + j].TexC.y = i * dv;
		}
	}

	//
	// Create the indices.
	//

	Indices32.resize(faceCount * 3); // 3 indices per face

	// Iterate over each quad and compute indices.
	uint32 k = 0;
	for (uint32 i = 0; i < m - 1; ++i)
	{
		for (uint32 j = 0; j < n - 1; ++j)
		{
			Indices32[k] = i * n + j;
			Indices32[k + 1] = i * n + j + 1;
			Indices32[k + 2] = (i + 1) * n + j;

			Indices32[k + 3] = (i + 1) * n + j;
			Indices32[k + 4] = i * n + j + 1;
			Indices32[k + 5] = (i + 1) * n + j + 1;

			k += 6; // next quad
		}
	}

	GenerateIndices16();
}


void TMesh::CreateQuad(float x, float y, float w, float h, float depth)
{
	Vertices.resize(4);
	Indices32.resize(6);

	// Position coordinates specified in NDC space.
	Vertices[0] = TVertex(
		x, y - h, depth,
		0.0f, 0.0f, -1.0f,
		1.0f, 0.0f, 0.0f,
		0.0f, 1.0f);

	Vertices[1] = TVertex(
		x, y, depth,
		0.0f, 0.0f, -1.0f,
		1.0f, 0.0f, 0.0f,
		0.0f, 0.0f);

	Vertices[2] = TVertex(
		x + w, y, depth,
		0.0f, 0.0f, -1.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f);

	Vertices[3] = TVertex(
		x + w, y - h, depth,
		0.0f, 0.0f, -1.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 1.0f);

	Indices32[0] = 0;
	Indices32[1] = 1;
	Indices32[2] = 2;

	Indices32[3] = 0;
	Indices32[4] = 2;
	Indices32[5] = 3;


	GenerateIndices16();
}

void TMesh::GenerateIndices16()
{
	if (Indices16.empty())
	{
		Indices16.resize(Indices32.size());
		for (size_t i = 0; i < Indices32.size(); ++i)
			Indices16[i] = static_cast<uint16>(Indices32[i]);
	}
}

const std::vector<std::uint16_t>& TMesh::GetIndices16() const
{
	return Indices16;
}

std::string TMesh::GetInputLayoutName() const
{
	return InputLayoutName;
}

void TMesh::GenerateBoundingBox()
{
	BoundingBox.Init(Vertices);
}