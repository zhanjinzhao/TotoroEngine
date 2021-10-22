#include "FbxLoader.h"
#include "File/FileHelpers.h"

using namespace std;

void TFbxLoader::Init()
{
	//Create FbxManager and FbxIOSettings
	Manager = unique_ptr<FbxManager, TFbxDeleter>(FbxManager::Create());

	IOSettings = unique_ptr<FbxIOSettings, TFbxDeleter>(FbxIOSettings::Create(Manager.get(), IOSROOT));

	Manager->SetIOSettings(IOSettings.get());

	//Debug
	bDebugMode = false;

	if (bDebugMode)
	{
		FILE* fp;
		freopen_s(&fp, "log.txt", "w", stdout);
	}
}

bool TFbxLoader::LoadFBXMesh(std::wstring MeshName, TMesh& Mesh)
{
	//Get model path and convert to UTF8
	std::wstring ModelDir = TFileHelpers::EngineDir() + L"Resource/Models/";
	std::wstring ModelPath = ModelDir + MeshName;
	char* ModelPath_UTF8;
	FbxWCToUTF8(ModelPath.c_str(), ModelPath_UTF8);

	// Create an importer 
	unique_ptr<FbxImporter, TFbxDeleter> Importer = unique_ptr<FbxImporter, TFbxDeleter>(FbxImporter::Create(Manager.get(), ""));

	// Use the first argument as the filename for the importer.
	if (!Importer->Initialize(ModelPath_UTF8, -1, Manager->GetIOSettings()))
	{
		return false;
	}

	// Create a scene 
	unique_ptr<FbxScene, TFbxDeleter> Scene = unique_ptr<FbxScene, TFbxDeleter>(FbxScene::Create(Manager.get(), ""));

	// Import the contents of the file into the scene.
	Importer->Import(Scene.get());

	//Convert system unit
	bool bUseMeterUnit = true;
	if (bUseMeterUnit)
	{
		auto FbxSystemUnit = Scene->GetGlobalSettings().GetSystemUnit();
		if (FbxSystemUnit != FbxSystemUnit::m)
		{
			FbxSystemUnit::m.ConvertScene(Scene.get());
		}
	}

	//Triangulate
	FbxGeometryConverter GeometryConverter(Manager.get());
	GeometryConverter.Triangulate(Scene.get(), true, false);

	FbxNode* RootNode = Scene->GetRootNode();
	if (!RootNode)
	{
		return false;
	}

	if (bDebugMode)
	{
		PrintNode(RootNode, 0);
	}

	//Process the scene tree from root node, collect all mesh data after processing
	ProcessNode(RootNode, Mesh);

	Mesh.GenerateIndices16();
	
	return true;
}

bool TFbxLoader::ProcessNode(FbxNode* Node, TMesh& Mesh)
{
	if (Node->GetMesh())
	{
		if (!ImportMesh(Node, Mesh))
			return false;
	}

	for (int i = 0; i < Node->GetChildCount(); i++)
	{
		if (!ProcessNode(Node->GetChild(i), Mesh))
		{
			return false;
		}
	}
	return true;
}

bool TFbxLoader::ImportMesh(FbxNode* Node, TMesh& Mesh)
{
	auto MeshObject = Node->GetMesh();

	if (!MeshObject->IsTriangleMesh())
	{
		return false;
	}

	MeshObject->GenerateNormals();
	MeshObject->GenerateTangentsDataForAllUVSets();

	uint32_t numTriangles = uint32_t(MeshObject->GetPolygonCount());
	uint32_t numPoints = MeshObject->GetControlPointsCount();
	int VertexCounter = 0;

	FbxAMatrix VertMatrix = Node->EvaluateGlobalTransform() * GetGeometryTransform(Node);

	for (uint32_t t = 0; t < numTriangles; t++)
	{
		for (uint32_t v = 0; v < 3; v++)
		{
			int CtrlPointIndex = MeshObject->GetPolygonVertex(t, v);

			//Add new index
			TMesh::uint32 index = TMesh::uint32(Mesh.Indices32.size());
			Mesh.Indices32.push_back(index);

			//Add new vertex
			FbxVector4 Position = ReadPositon(MeshObject, CtrlPointIndex);
			FbxVector4 FinalPos = VertMatrix.MultT(Position);
			FbxVector4 Normal = ReadNormal(MeshObject,CtrlPointIndex, VertexCounter, 0);
			FbxVector4 tangent = ReadTangent(MeshObject, CtrlPointIndex, VertexCounter, 0);
			FbxVector2 UV = ReadUV(MeshObject, CtrlPointIndex, MeshObject->GetTextureUVIndex(t, v), 0);
			
			TVertex Vertex;
			Vertex.Position.x = (float)FinalPos.mData[0];
			Vertex.Position.y = (float)FinalPos.mData[1];
			Vertex.Position.z = (float)FinalPos.mData[2];
			Vertex.Normal.x = (float)Normal[0];
			Vertex.Normal.y = (float)Normal[1];
			Vertex.Normal.z = (float)Normal[2];
			Vertex.TangentU.x = (float)tangent[0];
			Vertex.TangentU.y = (float)tangent[1];
			Vertex.TangentU.z = (float)tangent[2];
			Vertex.TexC.x = (float)UV[0];
			Vertex.TexC.y = 1.0f - (float)UV[1];

			Mesh.Vertices.push_back(Vertex);

			VertexCounter++;
		}
	}

	return true;
}

FbxAMatrix TFbxLoader::GetGeometryTransform(FbxNode* Node)
{
	assert(Node);

	const FbxVector4 T = Node->GetGeometricTranslation(FbxNode::eSourcePivot);
	const FbxVector4 R = Node->GetGeometricRotation(FbxNode::eSourcePivot);
	const FbxVector4 S = Node->GetGeometricScaling(FbxNode::eSourcePivot);

	return FbxAMatrix(T, R, S);
}


FbxVector4 TFbxLoader::ReadPositon(FbxMesh* MeshObject, int CtrlPointIndex)
{
	FbxVector4 Position = MeshObject->GetControlPointAt(CtrlPointIndex);

	return Position;
}

FbxVector4 TFbxLoader::ReadNormal(FbxMesh* MeshObject, int CtrlPointIndex, int VertexCounter, int Layer)
{
	FbxVector4 Normal;

	if (MeshObject->GetElementNormalCount() <= Layer)
	{
		return Normal;
	}

	FbxGeometryElementNormal* ElementNormal = MeshObject->GetElementNormal(Layer);

	switch (ElementNormal->GetMappingMode())
	{
		case FbxGeometryElement::eByControlPoint:
		{
			switch (ElementNormal->GetReferenceMode())
			{
			case FbxGeometryElement::eDirect:
			{
				Normal = ElementNormal->GetDirectArray().GetAt(CtrlPointIndex);
			}
			break;

			case FbxGeometryElement::eIndexToDirect:
			{
				int id = ElementNormal->GetIndexArray().GetAt(CtrlPointIndex);
				Normal = ElementNormal->GetDirectArray().GetAt(id);
			}
			break;

			default:
				break;
			}
		}
		break;

		case FbxGeometryElement::eByPolygonVertex:
		{
			switch (ElementNormal->GetReferenceMode())
			{
			case FbxGeometryElement::eDirect:
			{
				Normal = ElementNormal->GetDirectArray().GetAt(VertexCounter);
			}
			break;

			case FbxGeometryElement::eIndexToDirect:
			{
				int id = ElementNormal->GetIndexArray().GetAt(VertexCounter);
				Normal = ElementNormal->GetDirectArray().GetAt(id);
			}
			break;

			default:
				break;
			}
		}
		break;
	}

	return Normal;
}

FbxVector4 TFbxLoader::ReadTangent(FbxMesh* MeshObject, int CtrlPointIndex, int VertexCounter, int Layer)
{
	FbxVector4 Tangent;

	if (MeshObject->GetElementTangentCount() <= Layer)
	{
		return Tangent;
	}

	FbxGeometryElementTangent* ElementTangent = MeshObject->GetElementTangent(Layer);

	switch (ElementTangent->GetMappingMode())
	{
		case FbxGeometryElement::eByControlPoint:
		{
			switch (ElementTangent->GetReferenceMode())
			{
			case FbxGeometryElement::eDirect:
			{
				Tangent = ElementTangent->GetDirectArray().GetAt(CtrlPointIndex);
			}
			break;

			case FbxGeometryElement::eIndexToDirect:
			{
				int id = ElementTangent->GetIndexArray().GetAt(CtrlPointIndex);
				Tangent = ElementTangent->GetDirectArray().GetAt(id);
			}
			break;

			default:
				break;
			}
		}
		break;

		case FbxGeometryElement::eByPolygonVertex:
		{
			switch (ElementTangent->GetReferenceMode())
			{
			case FbxGeometryElement::eDirect:
			{
				Tangent = ElementTangent->GetDirectArray().GetAt(VertexCounter);
			}
			break;

			case FbxGeometryElement::eIndexToDirect:
			{
				int id = ElementTangent->GetIndexArray().GetAt(VertexCounter);
				Tangent = ElementTangent->GetDirectArray().GetAt(id);
			}
			break;

			default:
				break;
			}
		}
		break;
	}

	return Tangent;
}

FbxVector2 TFbxLoader::ReadUV(FbxMesh* MeshObject, int CtrlPointIndex, int TextureUVIndex, int Layer)
{
	FbxVector2 UV;

	if (MeshObject->GetElementUVCount() <= Layer)
	{
		return UV;
	}

	FbxGeometryElementUV* ElementUV = MeshObject->GetElementUV(Layer);

	switch (ElementUV->GetMappingMode())
	{
		case FbxGeometryElement::eByControlPoint:
		{
			switch (ElementUV->GetReferenceMode())
			{
			case FbxGeometryElement::eDirect:
			{
				UV = ElementUV->GetDirectArray().GetAt(CtrlPointIndex);
			}
			break;

			case FbxGeometryElement::eIndexToDirect:
			{
				int Id = ElementUV->GetIndexArray().GetAt(CtrlPointIndex);
				UV = ElementUV->GetDirectArray().GetAt(Id);
			}
			break;

			default:
				break;
			}
		}
		break;

		case FbxGeometryElement::eByPolygonVertex:
		{
			switch (ElementUV->GetReferenceMode())
			{
			case FbxGeometryElement::eDirect:
			case FbxGeometryElement::eIndexToDirect:
			{
				UV = ElementUV->GetDirectArray().GetAt(TextureUVIndex);
			}
			break;

			default:
				break;
			}
		}
		break;
	}

	return UV;
}


void TFbxLoader::PrintTabs(int depth)
{
	for (int i = 0; i < depth; i++)
	{
		printf("\t");
	}
}

FbxString TFbxLoader::GetAttributeTypeName(FbxNodeAttribute::EType type)
{
	switch (type) {
	case FbxNodeAttribute::eUnknown: return "unidentified";
	case FbxNodeAttribute::eNull: return "null";
	case FbxNodeAttribute::eMarker: return "marker";
	case FbxNodeAttribute::eSkeleton: return "skeleton";
	case FbxNodeAttribute::eMesh: return "mesh";
	case FbxNodeAttribute::eNurbs: return "nurbs";
	case FbxNodeAttribute::ePatch: return "patch";
	case FbxNodeAttribute::eCamera: return "camera";
	case FbxNodeAttribute::eCameraStereo: return "stereo";
	case FbxNodeAttribute::eCameraSwitcher: return "camera switcher";
	case FbxNodeAttribute::eLight: return "light";
	case FbxNodeAttribute::eOpticalReference: return "optical reference";
	case FbxNodeAttribute::eOpticalMarker: return "marker";
	case FbxNodeAttribute::eNurbsCurve: return "nurbs curve";
	case FbxNodeAttribute::eTrimNurbsSurface: return "trim nurbs surface";
	case FbxNodeAttribute::eBoundary: return "boundary";
	case FbxNodeAttribute::eNurbsSurface: return "nurbs surface";
	case FbxNodeAttribute::eShape: return "shape";
	case FbxNodeAttribute::eLODGroup: return "lodgroup";
	case FbxNodeAttribute::eSubDiv: return "subdiv";
	default: return "unknown";
	}
}

void TFbxLoader::PrintAttribute(FbxNodeAttribute* pAttribute, int depth)
{
	if (!pAttribute) return;

	FbxString typeName = GetAttributeTypeName(pAttribute->GetAttributeType());
	FbxString attrName = pAttribute->GetName();
	// Note: to retrieve the character array of a FbxString, use its Buffer() method.
	//printf(, );
	//

	PrintTabs(depth); printf("\t <attribute type='%s' name='%s'/> \n", typeName.Buffer(), attrName.Buffer());
}

void TFbxLoader::PrintNode(FbxNode* pNode, int depth)
{
	const char* nodeName = pNode->GetName();
	FbxDouble3 translation = pNode->LclTranslation.Get();
	FbxDouble3 rotation = pNode->LclRotation.Get();
	FbxDouble3 scaling = pNode->LclScaling.Get();

	// Print the contents of the node.
	PrintTabs(depth); printf("<node name='%s'>\n", nodeName);
	PrintTabs(depth); printf("\t translation='(%f, %f, %f) \n", translation[0], translation[1], translation[2]);
	PrintTabs(depth); printf("\t rotation='(%f, %f, %f) \n", rotation[0], rotation[1], rotation[2]);
	PrintTabs(depth); printf("\t scaling='(%f, %f, %f) \n", scaling[0], scaling[1], scaling[2]);


	// Print the node's attributes.
	for (int i = 0; i < pNode->GetNodeAttributeCount(); i++)
		PrintAttribute(pNode->GetNodeAttributeByIndex(i), depth);

	// Recursively print the children.
	for (int j = 0; j < pNode->GetChildCount(); j++)
		PrintNode(pNode->GetChild(j), depth + 1);

	PrintTabs(depth); printf("</node> \n");
}