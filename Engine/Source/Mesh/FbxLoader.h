#pragma once

#include <fbxsdk.h>
#include <memory>
#include "Mesh.h"

class TFbxDeleter {
public:
	void operator()(FbxManager* Manager) { Manager->Destroy(); }
	void operator()(FbxScene* Scene) { Scene->Destroy(); }
	void operator()(FbxImporter* Importer) { Importer->Destroy(); }
	void operator()(FbxIOSettings* IoSettings) { IoSettings->Destroy(); }
};

class TFbxLoader
{
public:
	void Init();

	bool LoadFBXMesh(std::wstring MeshName, TMesh& Mesh);

private:
	bool ProcessNode(FbxNode* Node, TMesh& Mesh);

	bool ImportMesh(FbxNode* Node, TMesh& Mesh);

	FbxAMatrix GetGeometryTransform(FbxNode* Node);

	FbxVector4 ReadPositon(FbxMesh* MeshObject,int CtrlPointIndex);

	FbxVector4 ReadNormal(FbxMesh* MeshObject, int CtrlPointIndex, int VertexCounter, int Layer);

	FbxVector4 ReadTangent(FbxMesh* MeshObject, int CtrlPointIndex, int VertexCounter, int Layer);

	FbxVector2 ReadUV(FbxMesh* MeshObject, int CtrlPointIndex, int TextureUVIndex, int Layer);


private:
	//------------------------Debug-------------------------------
	void PrintTabs(int depth);

	FbxString GetAttributeTypeName(FbxNodeAttribute::EType type);

	void PrintAttribute(FbxNodeAttribute* pAttribute, int depth);

	// Print a node, its attributes, and all its children recursively.
	void PrintNode(FbxNode* pNode, int depth);

private:
	std::unique_ptr<FbxManager, TFbxDeleter> Manager;

	std::unique_ptr<FbxIOSettings, TFbxDeleter> IOSettings;

	bool bDebugMode;
};