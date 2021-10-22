#include "Render.h"
#include "Actor/CameraActor.h"
#include "Actor/SkyActor.h"
#include "Actor/Light/DirectionalLightActor.h"
#include "Actor/Light/PointLightActor.h"
#include "Actor/Light/SpotLightActor.h"
#include "Actor/Light/AreaLightActor.h"
#include "Texture/TextureRepository.h"
#include "Material/MaterialRepository.h"
#include "Mesh/MeshRepository.h"
#include "Mesh/KdTree.h"
#include "Texture/TextureInfo.h"
#include "Utils/Logger.h"
#include <fstream>
#include <algorithm>
#include "File/FileHelpers.h"
#include "File/BinarySaver.h"
#include "File/BinaryReader.h"
#include "Sampler.h"

using namespace DirectX;

TRender::TRender()
	:bInitialize(false)
{
}

TRender::~TRender()
{
	OnDestroy();
}

bool TRender::IsInitialize()
{
	return bInitialize;
}

bool TRender::Initialize(int WindowWidth, int WindowHeight, TD3D12RHI* InD3D12RHI, TWorld* InWorld, const TRenderSettings& Settings)
{
	D3D12RHI = InD3D12RHI;

	World = InWorld;

	RenderSettings = Settings;

	D3DDevice = D3D12RHI->GetDevice()->GetD3DDevice();
	CommandList = D3D12RHI->GetDevice()->GetCommandList();

	GraphicsPSOManager = std::make_unique<TGraphicsPSOManager>(D3D12RHI, &InputLayoutManager);

	ComputePSOManager = std::make_unique<TComputePSOManager>(D3D12RHI);

	// Do the initial resize code.
	OnResize(WindowWidth, WindowHeight);

	CreateRenderResource();

	bInitialize = true;

	return true;
}

void TRender::OnResize(int NewWidth, int NewHeight)
{
	//TLogger::LogToOutput("---------------------------TRender Resize()----------------------\n");

	WindowWidth = NewWidth;
	WindowHeight = NewHeight;

	// Reset viewport
	D3D12RHI->ResizeViewport(NewWidth, NewHeight);

	// Reset camera
	// TODO: Merge the logic of camera and viewport
	TCameraComponent* CameraComponent = World->GetCameraComponent();
	CameraComponent->SetLens(CameraComponent->GetFovY(), AspectRatio(), CameraComponent->GetNearZ(), CameraComponent->GetFarZ());

	// Resize GBuffers
	CreateGBuffers();

	// Resize other buffers
	CreateSSAOBuffer();

	CreateBackDepth();

	CreateColorTextures();

	CreateComputeShaderResource();
}

float TRender::AspectRatio()const
{
	return static_cast<float>(WindowWidth) / WindowHeight;
}

TD3D12Resource* TRender::CurrentBackBuffer() const
{
	return D3D12RHI->GetViewport()->GetCurrentBackBuffer(); 
}

D3D12_CPU_DESCRIPTOR_HANDLE TRender::CurrentBackBufferView() const
{
	return D3D12RHI->GetViewport()->GetCurrentBackBufferView()->GetDescriptorHandle();
}

float* TRender::CurrentBackBufferClearColor() const
{
	return D3D12RHI->GetViewport()->GetCurrentBackBufferClearColor();
}

D3D12_CPU_DESCRIPTOR_HANDLE TRender::DepthStencilView() const
{
	return D3D12RHI->GetViewport()->GetDepthStencilView()->GetDescriptorHandle();
}


void TRender::CreateRenderResource()
{
	//Prepare for recording initialization commands.
	D3D12RHI->ResetCommandList();

	CreateNullDescriptors();

	CreateTextures();

	GetSkyInfo();
	if (SkyMeshComponent)
	{
		bEnableIBLEnvLighting = true;

		CreateSceneCaptureCube();
	}

	CreateMeshProxys();

	CreateInputLayouts();

	CreateGlobalShaders();

	CreateGlobalPSO();

	CreateComputePSO();

	// Execute the initialization commands.
	D3D12RHI->ExecuteCommandLists();

	// Wait until initialization is complete.
	D3D12RHI->FlushCommandQueue();
}

void TRender::CreateNullDescriptors()
{
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC SrvDesc = {};
		SrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		SrvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		SrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;

		Texture2DNullDescriptor = std::make_unique<TD3D12ShaderResourceView>(D3D12RHI->GetDevice(), SrvDesc, nullptr);
	}

	{
		D3D12_SHADER_RESOURCE_VIEW_DESC SrvDesc = {};
		SrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		SrvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		SrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;

		Texture3DNullDescriptor = std::make_unique<TD3D12ShaderResourceView>(D3D12RHI->GetDevice(), SrvDesc, nullptr);
	}

	{
		D3D12_SHADER_RESOURCE_VIEW_DESC SrvDesc = {};
		SrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		SrvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		SrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;

		TextureCubeNullDescriptor = std::make_unique<TD3D12ShaderResourceView>(D3D12RHI->GetDevice(), SrvDesc, nullptr);
	}

	{
		D3D12_SHADER_RESOURCE_VIEW_DESC SrvDesc = {};
		SrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		SrvDesc.Format = DXGI_FORMAT_UNKNOWN;
		SrvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		SrvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
		SrvDesc.Buffer.StructureByteStride = 1;
		SrvDesc.Buffer.NumElements = 1;
		SrvDesc.Buffer.FirstElement = 0;

		StructuredBufferNullDescriptor = std::make_unique<TD3D12ShaderResourceView>(D3D12RHI->GetDevice(), SrvDesc, nullptr);
	}
}

void TRender::CreateTextures()
{
	const auto& TextureMap = TTextureRepository::Get().TextureMap;

	// Create textures in reposity
	for (const auto& TexturePair : TextureMap)
	{
		TexturePair.second->LoadTextureResourceFromFlie(D3D12RHI);
		TexturePair.second->CreateTexture(D3D12RHI);
	}

	// Create SpriteFont and font texture
	std::wstring FontFilePath = TFileHelpers::EngineDir() + L"Resource/Fonts/myfile.spritefont";
	SpriteFont = std::make_unique<TSpriteFont>(D3DDevice, FontFilePath.c_str());

	TTextureInfo TextureInfo;
	TextureInfo.ArraySize = 1;
	TextureInfo.Format = SpriteFont->textureFormat;
	TextureInfo.Width = SpriteFont->textureWidth;
	TextureInfo.Height = SpriteFont->textureHeight;
	TextureInfo.Depth = 1;
	TextureInfo.MipCount = 1;
	TextureInfo.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

	D3D12_SUBRESOURCE_DATA InitData;
	InitData.pData = SpriteFont->textureData.data();
	InitData.RowPitch = static_cast<LONG_PTR>(SpriteFont->textureStride);
	InitData.SlicePitch = InitData.RowPitch * uint64_t(SpriteFont->textureRows);
	
	SpriteFont->GetFontTexture()->SetTextureResourceDirectly(TextureInfo, SpriteFont->textureData, InitData);
	SpriteFont->GetFontTexture()->CreateTexture(D3D12RHI);
}

void TRender::CreateSceneCaptureCube()
{
	IBLEnvironmentMap = std::make_unique<TSceneCaptureCube>(false, 512, DXGI_FORMAT_R32G32B32A32_FLOAT, D3D12RHI);
	IBLEnvironmentMap->CreatePerspectiveViews({ 0.0f, 0.0f, 0.0f }, 0.1f, 10.0f);

	IBLIrradianceMap = std::make_unique<TSceneCaptureCube>(false, 32, DXGI_FORMAT_R32G32B32A32_FLOAT, D3D12RHI);
	IBLIrradianceMap->CreatePerspectiveViews({ 0.0f, 0.0f, 0.0f }, 0.1f, 10.0f);

	for (UINT Mip = 0; Mip < IBLPrefilterMaxMipLevel; Mip++)
	{
		UINT MipWidth = UINT(128 * std::pow(0.5, Mip));
		auto PrefilterEnvMap = std::make_unique<TSceneCaptureCube>(false, MipWidth, DXGI_FORMAT_R16G16B16A16_FLOAT, D3D12RHI);
		PrefilterEnvMap->CreatePerspectiveViews({ 0.0f, 0.0f, 0.0f }, 0.1f, 10.0f);

		IBLPrefilterEnvMaps.push_back(std::move(PrefilterEnvMap));
	}
}

void TRender::CreateGBuffers()
{
	GBufferBaseColor = std::make_unique<TRenderTarget2D>(D3D12RHI, false, WindowWidth, WindowHeight, DXGI_FORMAT_R32G32B32A32_FLOAT);

	GBufferNormal = std::make_unique<TRenderTarget2D>(D3D12RHI, false, WindowWidth, WindowHeight, DXGI_FORMAT_R8G8B8A8_SNORM);

	GBufferWorldPos = std::make_unique<TRenderTarget2D>(D3D12RHI, false, WindowWidth, WindowHeight, DXGI_FORMAT_R32G32B32A32_FLOAT);

	GBufferORM = std::make_unique<TRenderTarget2D>(D3D12RHI, false, WindowWidth, WindowHeight, DXGI_FORMAT_R8G8B8A8_UNORM);

	GBufferVelocity = std::make_unique<TRenderTarget2D>(D3D12RHI, false, WindowWidth, WindowHeight, DXGI_FORMAT_R16G16_FLOAT);

	GBufferEmissive = std::make_unique<TRenderTarget2D>(D3D12RHI, false, WindowWidth, WindowHeight, DXGI_FORMAT_R8G8B8A8_UNORM);
}

void TRender::CreateSSAOBuffer()
{
	SSAOBuffer = std::make_unique<TRenderTarget2D>(D3D12RHI, false, WindowWidth, WindowHeight, DXGI_FORMAT_R16_UNORM);
}

void TRender::CreateBackDepth()
{
	BackDepth = std::make_unique<TRenderTarget2D>(D3D12RHI, true, WindowWidth, WindowHeight, DXGI_FORMAT_R24G8_TYPELESS);
}

void TRender::CreateColorTextures()
{
	TTextureInfo TextureInfo;
	TextureInfo.Type = ETextureType::TEXTURE_2D;
	TextureInfo.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	TextureInfo.Width = WindowWidth;
	TextureInfo.Height = WindowHeight;
	TextureInfo.Depth = 1;
	TextureInfo.ArraySize = 1;
	TextureInfo.MipCount = 1;
	TextureInfo.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	TextureInfo.InitState = D3D12_RESOURCE_STATE_COMMON;

	ColorTexture = D3D12RHI->CreateTexture(TextureInfo, TexCreate_SRV | TexCreate_RTV);

	CacheColorTexture = D3D12RHI->CreateTexture(TextureInfo, TexCreate_SRV);

	PrevColorTexture = D3D12RHI->CreateTexture(TextureInfo, TexCreate_SRV);
}

void TRender::CreateMeshProxys()
{
	auto& MeshMap = TMeshRepository::Get().MeshMap;

	for (auto& MeshPair : MeshMap)
	{
		TMesh& Mesh = MeshPair.second;

		// Create SDF texture
		CreateMeshSDFTexture(Mesh);

		// Generate MeshProxy
		MeshProxyMap.emplace(Mesh.MeshName, TMeshProxy());
		TMeshProxy& MeshProxy = MeshProxyMap.at(Mesh.MeshName);

		const UINT VbByteSize = (UINT)Mesh.Vertices.size() * sizeof(TVertex);

		std::vector<std::uint16_t> indices = Mesh.GetIndices16();
		const UINT IbByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

		MeshProxy.VertexBufferRef = D3D12RHI->CreateVertexBuffer(Mesh.Vertices.data(), VbByteSize);
		MeshProxy.IndexBufferRef = D3D12RHI->CreateIndexBuffer(indices.data(), IbByteSize);

		MeshProxy.VertexByteStride = sizeof(TVertex);
		MeshProxy.VertexBufferByteSize = VbByteSize;
		MeshProxy.IndexFormat = DXGI_FORMAT_R16_UINT;
		MeshProxy.IndexBufferByteSize = IbByteSize;

		TSubmeshProxy submesh;
		submesh.IndexCount = (UINT)indices.size();
		submesh.StartIndexLocation = 0;
		submesh.BaseVertexLocation = 0;

		MeshProxy.SubMeshs["Default"] = submesh;
	}
}

void TRender::CreateMeshSDFTexture(TMesh& Mesh)
{
	std::vector<uint8_t> MeshSDF;

	std::wstring MeshSDFPath = TFileHelpers::EngineDir() + L"Save/MeshSDF/" + TFormatConvert::StrToWStr(Mesh.MeshName) + L".sdf";
	if (TFileHelpers::IsFileExit(MeshSDFPath)) //Read SDF from file
	{
		TBinaryReader Reader(MeshSDFPath.c_str());
		TMeshSDFDescriptor& Descriptor = Mesh.SDFDescriptor;
		Descriptor.Center.x = Reader.Read<float>();
		Descriptor.Center.y = Reader.Read<float>();
		Descriptor.Center.z = Reader.Read<float>();
		Descriptor.Extent = Reader.Read<float>();
		Descriptor.Resolution = Reader.Read<int>();

		int SDFDataCount = Descriptor.Resolution * Descriptor.Resolution * Descriptor.Resolution;
		const uint8_t* SDFData = Reader.ReadArray<uint8_t>(SDFDataCount);
		MeshSDF.assign(SDFData, SDFData + SDFDataCount);
	}
	else // Bulid SDF and save to file
	{
		BulidMeshSDF(Mesh, MeshSDF);

		TBinarySaver FileSaver(MeshSDFPath);
		const TMeshSDFDescriptor& Descriptor = Mesh.SDFDescriptor;
		FileSaver.Save(Descriptor.Center.x);
		FileSaver.Save(Descriptor.Center.y);
		FileSaver.Save(Descriptor.Center.z);
		FileSaver.Save(Descriptor.Extent);
		FileSaver.Save(Descriptor.Resolution);

		FileSaver.SaveArray<uint8_t>(MeshSDF.data(), MeshSDF.size());
	}

	int SDFResolution = Mesh.SDFDescriptor.Resolution;

	// Create  SDF texture
	TTextureInfo TextureInfo;
	TextureInfo.Type = ETextureType::TEXTURE_3D;
	TextureInfo.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
	TextureInfo.Width = (UINT)(SDFResolution);
	TextureInfo.Height = (UINT)(SDFResolution);
	TextureInfo.Depth = (UINT)(SDFResolution);
	TextureInfo.ArraySize = 1;
	TextureInfo.MipCount = 1;
	TextureInfo.Format = DXGI_FORMAT::DXGI_FORMAT_R8_UNORM;

	D3D12_SUBRESOURCE_DATA InitData;
	InitData.pData = MeshSDF.data();
	InitData.RowPitch = static_cast<LONG_PTR>(SDFResolution);
	InitData.SlicePitch = InitData.RowPitch * SDFResolution;

	auto SDFTeture = std::make_unique<TTexture3D>("SDFTeture", false, L" ");
	SDFTeture->SetTextureResourceDirectly(TextureInfo, MeshSDF, InitData);
	SDFTeture->CreateTexture(D3D12RHI);

	Mesh.SetSDFTexture(SDFTeture);
}

void GenerateUniformSphereSamples(UINT SampleCount, std::vector<TVector3>& Samples)
{
	// Generate Fibonacci lattice.
	// Ref: https://stackoverflow.com/questions/9600801/evenly-distributing-n-points-on-a-sphere/26127012#26127012

	const float Phi = TMath::Pi * (3.0f - sqrt(5.0f)); // Golden angle in radians
	for (UINT i = 0; i < SampleCount; i++)
	{
		TVector3 Sample;
		Sample.y = 1 - (i / float(SampleCount - 1)) * 2;  // y goes from 1 to - 1
		float Radius = sqrt(1.0f - Sample.y * Sample.y);  // Radius at y

		float Theta = Phi * i;    // Golden angle increment
		Sample.x = Radius * cos(Theta);
		Sample.z = Radius * sin(Theta);

		Samples.push_back(Sample);
	}
}

void TRender::BulidMeshSDF(TMesh& Mesh, std::vector<uint8_t>& OutMeshSDF)
{
	std::vector<uint32_t>& Indices = Mesh.Indices32;
	std::vector<TVertex>& Vertices = Mesh.Vertices;
	UINT IndiceCount = (UINT)Indices.size();
	UINT TriangleCount = IndiceCount / 3;

	// Get all triangles
	std::vector<std::shared_ptr<TPrimitive>> BuildTriangles;
	for (UINT i = 0; i < TriangleCount; i++) 
	{
		// Indices for this triangle.
		UINT i0 = Indices[i * 3 + 0];
		UINT i1 = Indices[i * 3 + 1];
		UINT i2 = Indices[i * 3 + 2];

		// Vertices for this triangle.
		TVector3 v0 = Vertices[i0].Position;
		TVector3 v1 = Vertices[i1].Position;
		TVector3 v2 = Vertices[i2].Position;

		auto Triangle = std::make_shared<TTriangle>(v0, v1, v2, TColor::Black);
		Triangle->GenerateBoundingBox();

		BuildTriangles.push_back(Triangle);
	}

	// Bulid kd-tree
	auto KdTree = std::make_unique<TKdTreeAccelerator>(std::move(BuildTriangles));

	// Build SDF
	TVector3 SDFCenter = Mesh.BoundingBox.GetCenter();
	float SDFWidth = Mesh.BoundingBox.GetMaxWidth() * 1.4f;
	const int SDFResolution = 32;
	float SDFUnit = SDFWidth / SDFResolution;
	const int SampleCount = 256;

	std::vector<float> SDF;
	SDF.resize(SDFResolution * SDFResolution * SDFResolution);

	for (int z = 0; z < SDFResolution; z++)
	{
		for (int y = 0; y < SDFResolution; y++)
		{
			for (int x = 0; x < SDFResolution; x++)
			{
				TVector3 RayOrigin(
					((float)x - SDFResolution / 2 + 0.5f) * SDFUnit + SDFCenter.x,
					((float)y - SDFResolution / 2 + 0.5f) * SDFUnit + SDFCenter.y,
					((float)z - SDFResolution / 2 + 0.5f) * SDFUnit + SDFCenter.z
				);

				float MinDistance = TMath::Infinity;
				int FrontCount = 0;
				int BackCount = 0;

				std::vector<TVector3> SampleDirections;
				GenerateUniformSphereSamples(SampleCount, SampleDirections);

				// Fibonacci lattices.
				for (int i = 0; i < SampleCount; i++)
				{
					TVector3 RayDirection = SampleDirections[i];

					// Ray-Triangle Intersection test with kd-tree
					float Dist;
					bool bBackFace;
					if (KdTree->Intersect(TRay(RayOrigin, RayDirection), Dist, bBackFace))
					{
						if (Dist < MinDistance)
						{
							MinDistance = Dist;
						}

						if (bBackFace)
						{
							BackCount++;
						}
						else
						{
							FrontCount++;
						}
					}
				}

				int SDFIndex = z * SDFResolution * SDFResolution + y * SDFResolution + x;
				SDF[SDFIndex] = MinDistance;
				if (BackCount > FrontCount)
				{
					SDF[SDFIndex] *= -1.0f;
				}
			}
		}
	}

	// Draw SDF debug point
	if (false)
	{
		for (int z = 0; z < SDFResolution; z++)
		{
			for (int y = 0; y < SDFResolution; y++)
			{
				for (int x = 0; x < SDFResolution; x++)
				{
					TVector3 DrawCenter(-5.0f, 2.0f, 0.0f);
					float DrawUnit = SDFUnit * 0.5f;
					TVector3 DrawPosition(
						((float)x - SDFResolution / 2 + 0.5f) * DrawUnit + DrawCenter.x,
						((float)y - SDFResolution / 2 + 0.5f) * DrawUnit + DrawCenter.y,
						((float)z - SDFResolution / 2 + 0.5f) * DrawUnit + DrawCenter.z
					);

					int SDFIndex = z * SDFResolution * SDFResolution + y * SDFResolution + x;

					float SDFValue = SDF[SDFIndex];
					if (std::abs(SDFValue) < 0.1f)
					{
						World->DrawPoint(DrawPosition, TColor::Red, 3);
					}
				}
			}
		}
	}

	// Convert to EightBitFixedPoint(uint8)
	std::vector<uint8_t> QuantizedSDF;
	QuantizedSDF.resize(SDFResolution * SDFResolution * SDFResolution);
	for (int Index = 0; Index < SDF.size(); Index++)
	{
		if (SDF[Index] == TMath::Infinity)
		{
			QuantizedSDF[Index] = 255;
		}
		else
		{
			// Convert to range [-1, 1]
			float Value = SDF[Index] / SDFWidth;

			// Convert to range [0, 1]
			Value = Value * 0.5f + 0.5f;

			// Covert to range [0, 255], based on D3D format conversion rules for DXGI_FORMAT_R8_UNORM
			int QuantizedValue = int(Value * 255.0f + .5f);
			QuantizedSDF[Index] = (UINT8)std::clamp(QuantizedValue, 0, 255);
		}
	}

	std::swap(QuantizedSDF, OutMeshSDF);

	// Save SDFDescriptor
	Mesh.SDFDescriptor.Center = SDFCenter;
	Mesh.SDFDescriptor.Extent = SDFWidth * 0.5f;
	Mesh.SDFDescriptor.Resolution = SDFResolution;
}

void TRender::CreateInputLayouts()
{
	//DefaultInputLayout
	std::vector<D3D12_INPUT_ELEMENT_DESC>  DefaultInputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 36, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};

	InputLayoutManager.AddInputLayout("DefaultInputLayout", DefaultInputLayout);


	//PositionColorInputLayout
	std::vector<D3D12_INPUT_ELEMENT_DESC>  PositionColorInputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};

	InputLayoutManager.AddInputLayout("PositionColorInputLayout", PositionColorInputLayout);

	//PositionTexcoordInputLayout
	std::vector<D3D12_INPUT_ELEMENT_DESC> PositionTexcoordInputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};

	InputLayoutManager.AddInputLayout("PositionTexcoordInputLayout", PositionTexcoordInputLayout);
}

void TRender::CreateGlobalShaders()
{
	{
		TShaderInfo ShaderInfo;
		ShaderInfo.ShaderName = "SingleShadowMap";
		ShaderInfo.FileName = "SingleShadowMap";
		ShaderInfo.bCreateVS = true;
		ShaderInfo.bCreatePS = true;
		SingleShadowMapShader = std::make_unique<TShader>(ShaderInfo, D3D12RHI);
	}

	{
		TShaderInfo ShaderInfo;
		ShaderInfo.ShaderName = "OmniShadowMap";
		ShaderInfo.FileName = "OmniShadowMap";
		ShaderInfo.bCreateVS = true;
		ShaderInfo.bCreatePS = true;
		OmniShadowMapShader = std::make_unique<TShader>(ShaderInfo, D3D12RHI);
	}

	{
		TShaderInfo ShaderInfo;
		ShaderInfo.ShaderName = "BackDepth";
		ShaderInfo.FileName = "BackDepth";
		ShaderInfo.bCreateVS = true;
		ShaderInfo.bCreatePS = true;
		BackDepthShader = std::make_unique<TShader>(ShaderInfo, D3D12RHI);
	}

	if (bEnableIBLEnvLighting)
	{
		TShaderInfo ShaderInfo;
		ShaderInfo.ShaderName = "IBLEnvironment";
		ShaderInfo.FileName = "IBLEnvironment";
		ShaderInfo.bCreateVS = true;
		ShaderInfo.bCreatePS = true;
		IBLEnvironmentShader = std::make_unique<TShader>(ShaderInfo, D3D12RHI);
	}

	if(bEnableIBLEnvLighting)
	{
		TShaderInfo ShaderInfo;
		ShaderInfo.ShaderName = "IBLIrradiance";
		ShaderInfo.FileName = "IBLIrradiance";
		ShaderInfo.bCreateVS = true;
		ShaderInfo.bCreatePS = true;
		IBLIrradianceShader = std::make_unique<TShader>(ShaderInfo, D3D12RHI);
	}

	if(bEnableIBLEnvLighting)
	{
		TShaderInfo ShaderInfo;
		ShaderInfo.ShaderName = "IBLPrefilterEnv";
		ShaderInfo.FileName = "IBLPrefilterEnv";
		ShaderInfo.bCreateVS = true;
		ShaderInfo.bCreatePS = true;
		IBLPrefilterEnvShader = std::make_unique<TShader>(ShaderInfo, D3D12RHI);
	}

	{
		TShaderInfo ShaderInfo;
		ShaderInfo.ShaderName = "DeferredLighting";
		ShaderInfo.FileName = "DeferredLighting";
		if (RenderSettings.bUseTBDR)
		{
			ShaderInfo.ShaderDefines.SetDefine("USE_TBDR", "1");
		}
		if (RenderSettings.ShadowMapImpl == EShadowMapImpl::PCSS)
		{
			ShaderInfo.ShaderDefines.SetDefine("USE_PCSS", "1");
		}
		else if (RenderSettings.ShadowMapImpl == EShadowMapImpl::VSM)
		{
			ShaderInfo.ShaderDefines.SetDefine("USE_VSM", "1");
		}
		else if (RenderSettings.ShadowMapImpl == EShadowMapImpl::SDF)
		{
			ShaderInfo.ShaderDefines.SetDefine("USE_SDF", "1");
		}
		ShaderInfo.bCreateVS = true;
		ShaderInfo.bCreatePS = true;
		DeferredLightingShader = std::make_unique<TShader>(ShaderInfo, D3D12RHI);
	}

	{
		TShaderInfo ShaderInfo;
		ShaderInfo.ShaderName = "Primitive";
		ShaderInfo.FileName = "Primitive";
		ShaderInfo.bCreateVS = true;
		ShaderInfo.bCreatePS = true;
		PrimitiveShader = std::make_unique<TShader>(ShaderInfo, D3D12RHI);
	}

	{
		TShaderInfo ShaderInfo;
		ShaderInfo.ShaderName = "Sprite";
		ShaderInfo.FileName = "Sprite";
		ShaderInfo.bCreateVS = true;
		ShaderInfo.bCreatePS = true;
		SpriteShader = std::make_unique<TShader>(ShaderInfo, D3D12RHI);
	}

	{
		TShaderInfo ShaderInfo;
		ShaderInfo.ShaderName = "DebugSDFScene";
		ShaderInfo.FileName = "DebugSDFScene";
		ShaderInfo.bCreateVS = true;
		ShaderInfo.bCreatePS = true;
		DebugSDFSceneShader = std::make_unique<TShader>(ShaderInfo, D3D12RHI);
	}

	{
		TShaderInfo ShaderInfo;
		ShaderInfo.ShaderName = "SSAO";
		ShaderInfo.FileName = "SSAO";
		ShaderInfo.bCreateVS = true;
		ShaderInfo.bCreatePS = true;
		SSAOShader = std::make_unique<TShader>(ShaderInfo, D3D12RHI);
	}

	{
		TShaderInfo ShaderInfo;
		ShaderInfo.ShaderName = "SSR";
		ShaderInfo.FileName = "SSR";
		ShaderInfo.bCreateVS = true;
		ShaderInfo.bCreatePS = true;
		SSRShader = std::make_unique<TShader>(ShaderInfo, D3D12RHI);
	}

	{
		TShaderInfo ShaderInfo;
		ShaderInfo.ShaderName = "PostProcess";
		ShaderInfo.FileName = "PostProcess";
		ShaderInfo.bCreateVS = true;
		ShaderInfo.bCreatePS = true;
		PostProcessShader = std::make_unique<TShader>(ShaderInfo, D3D12RHI);
	}

	{
		TShaderInfo ShaderInfo;
		ShaderInfo.ShaderName = "TAA";
		ShaderInfo.FileName = "TAA";
		ShaderInfo.bCreateVS = true;
		ShaderInfo.bCreatePS = true;
		TAAShader = std::make_unique<TShader>(ShaderInfo, D3D12RHI);
	}

	{
		TShaderInfo ShaderInfo;
		ShaderInfo.ShaderName = "HorzBlur";
		ShaderInfo.FileName = "Blur";
		ShaderInfo.bCreateCS = true;
		ShaderInfo.CSEntryPoint = "HorzBlurCS";
		HorzBlurShader = std::make_unique<TShader>(ShaderInfo, D3D12RHI);
	}

	{
		TShaderInfo ShaderInfo;
		ShaderInfo.ShaderName = "VertBlur";
		ShaderInfo.FileName = "Blur";
		ShaderInfo.bCreateCS = true;
		ShaderInfo.CSEntryPoint = "VertBlurCS";
		VertBlurShader = std::make_unique<TShader>(ShaderInfo, D3D12RHI);
	}

	if(RenderSettings.ShadowMapImpl == EShadowMapImpl::VSM)
	{
		TShaderInfo ShaderInfo;
		ShaderInfo.ShaderName = "HorzBlurVSM";
		ShaderInfo.FileName = "BlurVSM";
		ShaderInfo.bCreateCS = true;
		ShaderInfo.CSEntryPoint = "HorzBlurCS";
		HorzBlurVSMShader = std::make_unique<TShader>(ShaderInfo, D3D12RHI);
	}

	if (RenderSettings.ShadowMapImpl == EShadowMapImpl::VSM)
	{
		TShaderInfo ShaderInfo;
		ShaderInfo.ShaderName = "VertBlurVSM";
		ShaderInfo.FileName = "BlurVSM";
		ShaderInfo.bCreateCS = true;
		ShaderInfo.CSEntryPoint = "VertBlurCS";
		VertBlurVSMShader = std::make_unique<TShader>(ShaderInfo, D3D12RHI);
	}

	if (RenderSettings.ShadowMapImpl == EShadowMapImpl::VSM)
	{
		TShaderInfo ShaderInfo;
		ShaderInfo.ShaderName = "GenerateVSM";
		ShaderInfo.FileName = "GenerateVSM";
		ShaderInfo.bCreateCS = true;
		GenerateVSMShader = std::make_unique<TShader>(ShaderInfo, D3D12RHI);
	}

	if(RenderSettings.bUseTBDR)
	{
		TShaderInfo ShaderInfo;
		ShaderInfo.ShaderName = "TiledBaseLightCulling";
		ShaderInfo.FileName = "TiledBaseLightCulling";
		ShaderInfo.bCreateCS = true;
		TiledBaseLightCullingShader = std::make_unique<TShader>(ShaderInfo, D3D12RHI);
	}
}

void TRender::CreateGlobalPSO()
{
	// IBLEnvironmentPSODescriptor
	if(bEnableIBLEnvLighting)
	{
		IBLEnvironmentPSODescriptor.InputLayoutName = std::string("DefaultInputLayout");
		IBLEnvironmentPSODescriptor.Shader = IBLEnvironmentShader.get();
		IBLEnvironmentPSODescriptor.RTVFormats[0] = IBLEnvironmentMap->GetRTCube()->GetFormat();
		IBLEnvironmentPSODescriptor.NumRenderTargets = 1;
		// The camera is inside the cube, so just turn off culling.
		IBLEnvironmentPSODescriptor.RasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
		IBLEnvironmentPSODescriptor.DepthStencilDesc.DepthEnable = false;
		IBLEnvironmentPSODescriptor.DepthStencilFormat = DXGI_FORMAT_UNKNOWN;

		GraphicsPSOManager->TryCreatePSO(IBLEnvironmentPSODescriptor);
	}

	// IBLIrradiance PSO
	if(bEnableIBLEnvLighting)
	{	
		IBLIrradiancePSODescriptor.InputLayoutName = std::string("DefaultInputLayout");
		IBLIrradiancePSODescriptor.Shader = IBLIrradianceShader.get();
		IBLIrradiancePSODescriptor.RTVFormats[0] = IBLIrradianceMap->GetRTCube()->GetFormat();
		IBLIrradiancePSODescriptor.NumRenderTargets = 1;
		// The camera is inside the cube, so just turn off culling.
		IBLIrradiancePSODescriptor.RasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
		IBLIrradiancePSODescriptor.DepthStencilDesc.DepthEnable = false;
		IBLIrradiancePSODescriptor.DepthStencilFormat = DXGI_FORMAT_UNKNOWN;

		GraphicsPSOManager->TryCreatePSO(IBLIrradiancePSODescriptor);
	}

	// IBLPrefilterEnv PSO
	if(bEnableIBLEnvLighting)
	{
		IBLPrefilterEnvPSODescriptor.InputLayoutName = std::string("DefaultInputLayout");
		IBLPrefilterEnvPSODescriptor.Shader = IBLPrefilterEnvShader.get();
		IBLPrefilterEnvPSODescriptor.RTVFormats[0] = IBLPrefilterEnvMaps[0]->GetRTCube()->GetFormat();
		IBLPrefilterEnvPSODescriptor.NumRenderTargets = 1;
		// The camera is inside the cube, so just turn off culling.
		IBLPrefilterEnvPSODescriptor.RasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
		IBLPrefilterEnvPSODescriptor.DepthStencilDesc.DepthEnable = false;
		IBLPrefilterEnvPSODescriptor.DepthStencilFormat = DXGI_FORMAT_UNKNOWN;

		GraphicsPSOManager->TryCreatePSO(IBLPrefilterEnvPSODescriptor);
	}

	// DeferredLighting
	{
		D3D12_DEPTH_STENCIL_DESC lightPassDSD;
		lightPassDSD.DepthEnable = false;
		lightPassDSD.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
		lightPassDSD.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
		lightPassDSD.StencilEnable = true;
		lightPassDSD.StencilReadMask = 0xff;
		lightPassDSD.StencilWriteMask = 0x0;
		lightPassDSD.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
		lightPassDSD.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
		lightPassDSD.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
		lightPassDSD.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_GREATER_EQUAL;
		// We are not rendering backfacing polygons, so these settings do not matter. 
		lightPassDSD.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
		lightPassDSD.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
		lightPassDSD.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
		lightPassDSD.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_GREATER_EQUAL;

		auto blendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		blendState.AlphaToCoverageEnable = false;
		blendState.IndependentBlendEnable = false;

		blendState.RenderTarget[0].BlendEnable = true;
		blendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		blendState.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		blendState.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;

		auto rasterizer = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		//rasterizer.CullMode = D3D12_CULL_MODE_FRONT; // Front culling for point light
		rasterizer.CullMode = D3D12_CULL_MODE_NONE;
		rasterizer.DepthClipEnable = false;

		DeferredLightingPSODescriptor.InputLayoutName = std::string("DefaultInputLayout");
		DeferredLightingPSODescriptor.Shader = DeferredLightingShader.get();
		DeferredLightingPSODescriptor.BlendDesc = blendState;
		DeferredLightingPSODescriptor.DepthStencilDesc = lightPassDSD;
		DeferredLightingPSODescriptor.RasterizerDesc = rasterizer;
		DeferredLightingPSODescriptor.RTVFormats[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;
		DeferredLightingPSODescriptor.NumRenderTargets = 1;
		DeferredLightingPSODescriptor._4xMsaaState = false; //can't use msaa in deferred rendering.

		GraphicsPSOManager->TryCreatePSO(DeferredLightingPSODescriptor);
	}

	// DebugSDFScene
	{
		DebugSDFScenePSODescriptor.InputLayoutName = std::string("DefaultInputLayout");
		DebugSDFScenePSODescriptor.Shader = DebugSDFSceneShader.get();
		DebugSDFScenePSODescriptor.RasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
		DebugSDFScenePSODescriptor.DepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
		DebugSDFScenePSODescriptor.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		DebugSDFScenePSODescriptor.RTVFormats[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;

		GraphicsPSOManager->TryCreatePSO(DebugSDFScenePSODescriptor);
	}

	// SSAO
	{
		SSAOPSODescriptor.InputLayoutName = std::string("DefaultInputLayout");
		SSAOPSODescriptor.Shader = SSAOShader.get();
		SSAOPSODescriptor.RTVFormats[0] = SSAOBuffer->GetFormat();
		SSAOPSODescriptor.NumRenderTargets = 1;		
		SSAOPSODescriptor.RasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
		SSAOPSODescriptor.DepthStencilDesc.DepthEnable = false;
		//SSAOPSODescriptor.DepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
		SSAOPSODescriptor.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

		GraphicsPSOManager->TryCreatePSO(SSAOPSODescriptor);
	}

	// SSR
	{
		SSRPSODescriptor.InputLayoutName = std::string("DefaultInputLayout");
		SSRPSODescriptor.Shader = SSRShader.get();
		SSRPSODescriptor.RasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
		SSRPSODescriptor.DepthStencilDesc.DepthEnable = false;
		SSRPSODescriptor.DepthStencilFormat = DXGI_FORMAT_UNKNOWN;
		SSRPSODescriptor.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		SSRPSODescriptor.RTVFormats[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;

		GraphicsPSOManager->TryCreatePSO(SSRPSODescriptor);
	}

	// PostProcess
	{
		PostProcessPSODescriptor.InputLayoutName = std::string("DefaultInputLayout");
		PostProcessPSODescriptor.Shader = PostProcessShader.get();
		PostProcessPSODescriptor.RasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
		PostProcessPSODescriptor.DepthStencilDesc.DepthEnable = false;
		PostProcessPSODescriptor.DepthStencilFormat = DXGI_FORMAT_UNKNOWN;
		PostProcessPSODescriptor.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		PostProcessPSODescriptor.RTVFormats[0] = D3D12RHI->GetViewportInfo().BackBufferFormat;

		GraphicsPSOManager->TryCreatePSO(PostProcessPSODescriptor);
	}

	// TAA
	{
		TAAPSODescriptor.InputLayoutName = std::string("DefaultInputLayout");
		TAAPSODescriptor.Shader = TAAShader.get();
		TAAPSODescriptor.RasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
		TAAPSODescriptor.DepthStencilDesc.DepthEnable = false;
		TAAPSODescriptor.DepthStencilFormat = DXGI_FORMAT_UNKNOWN;
		TAAPSODescriptor.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		TAAPSODescriptor.RTVFormats[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;

		GraphicsPSOManager->TryCreatePSO(TAAPSODescriptor);
	}
}

void TRender::CreateComputePSO()
{
	HorzBlurPSODescriptor.Shader = HorzBlurShader.get();
	HorzBlurPSODescriptor.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	ComputePSOManager->TryCreatePSO(HorzBlurPSODescriptor);

	VertBlurPSODescriptor.Shader = VertBlurShader.get();
	VertBlurPSODescriptor.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	ComputePSOManager->TryCreatePSO(VertBlurPSODescriptor);

	if (RenderSettings.ShadowMapImpl == EShadowMapImpl::VSM)
	{
		HorzBlurVSMPSODescriptor.Shader = HorzBlurVSMShader.get();
		HorzBlurVSMPSODescriptor.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
		ComputePSOManager->TryCreatePSO(HorzBlurVSMPSODescriptor);

		VertBlurVSMPSODescriptor.Shader = VertBlurVSMShader.get();
		VertBlurVSMPSODescriptor.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
		ComputePSOManager->TryCreatePSO(VertBlurVSMPSODescriptor);

		GenerateVSMPSODescriptor.Shader = GenerateVSMShader.get();
		GenerateVSMPSODescriptor.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
		ComputePSOManager->TryCreatePSO(GenerateVSMPSODescriptor);
	}

	if (RenderSettings.bUseTBDR)
	{
		TiledBaseLightCullingPSODescriptor.Shader = TiledBaseLightCullingShader.get();
		TiledBaseLightCullingPSODescriptor.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
		ComputePSOManager->TryCreatePSO(TiledBaseLightCullingPSODescriptor);
	}
}

std::vector<float> CalcGaussWeights(float sigma)
{
	float twoSigma2 = 2.0f * sigma * sigma;

	// Estimate the blur radius based on sigma since sigma controls the "width" of the bell curve.
	// For example, for sigma = 3, the width of the bell curve is 
	int blurRadius = (int)ceil(2.0f * sigma);

	const int MaxBlurRadius = 5;
	assert(blurRadius <= MaxBlurRadius);

	std::vector<float> weights;
	weights.resize(2 * blurRadius + 1);

	float weightSum = 0.0f;

	for (int i = -blurRadius; i <= blurRadius; ++i)
	{
		float x = (float)i;

		weights[i + blurRadius] = expf(-x * x / twoSigma2);

		weightSum += weights[i + blurRadius];
	}

	// Divide by the sum so all the weights add up to 1.0.
	for (int i = 0; i < weights.size(); ++i)
	{
		weights[i] /= weightSum;
	}

	return weights;
}

void TRender::CreateComputeShaderResource()
{
	// Blur
	{
		TTextureInfo TextureInfo;
		TextureInfo.Type = ETextureType::TEXTURE_2D;
		TextureInfo.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		TextureInfo.Width = WindowWidth;
		TextureInfo.Height = WindowHeight;
		TextureInfo.Depth = 1;
		TextureInfo.ArraySize = 1;
		TextureInfo.MipCount = 1;
		TextureInfo.Format = DXGI_FORMAT_R16_UNORM;
		TextureInfo.InitState = D3D12_RESOURCE_STATE_COMMON;

		BlurMap0 = D3D12RHI->CreateTexture(TextureInfo, TexCreate_SRV | TexCreate_UAV);

		BlurMap1 = D3D12RHI->CreateTexture(TextureInfo, TexCreate_SRV | TexCreate_UAV);
	}

	{
		auto Weights = CalcGaussWeights(2.5f);
		int BlurRadius = (int)Weights.size() / 2;

		BlurSettingsConstants Constants;
		Constants.gBlurRadius = BlurRadius;
		size_t DataSize = Weights.size() * sizeof(float);
		memcpy_s(&(Constants.w0), DataSize, Weights.data(), DataSize);

		BlurSettingsCB = D3D12RHI->CreateConstantBuffer(&Constants, sizeof(Constants));
	}

	if (RenderSettings.ShadowMapImpl == EShadowMapImpl::VSM)
	{
		auto Weights = CalcGaussWeights(1.5f);
		int BlurRadius = (int)Weights.size() / 2;

		BlurSettingsConstants Constants;
		Constants.gBlurRadius = BlurRadius;
		size_t DataSize = Weights.size() * sizeof(float);
		memcpy_s(&(Constants.w0), DataSize, Weights.data(), DataSize);

		VSMBlurSettingsCB = D3D12RHI->CreateConstantBuffer(&Constants, sizeof(Constants));
	}

	// TiledDepthDebug
	if(RenderSettings.bUseTBDR)
	{
		TTextureInfo TextureInfo;
		TextureInfo.Type = ETextureType::TEXTURE_2D;
		TextureInfo.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		TextureInfo.Width = WindowWidth;
		TextureInfo.Height = WindowHeight;
		TextureInfo.Depth = 1;
		TextureInfo.ArraySize = 1;
		TextureInfo.MipCount = 1;
		TextureInfo.Format = DXGI_FORMAT_R16G16_UNORM;
		TextureInfo.InitState = D3D12_RESOURCE_STATE_COMMON;

		TiledDepthDebugTexture = D3D12RHI->CreateTexture(TextureInfo, TexCreate_UAV);
	}

	// TileLightInfoList
	if(RenderSettings.bUseTBDR)
	{
		UINT MaxWindowWidth = 1920; //TODO
		UINT MaxWindowHeight = 1080; //TODO
		UINT MaxTileCountX = (UINT)ceilf(MaxWindowWidth / float(TILE_BLOCK_SIZE));
		UINT MaxTileCountY = (UINT)ceilf(MaxWindowHeight / float(TILE_BLOCK_SIZE));

		TileLightInfoList = D3D12RHI->CreateRWStructuredBuffer(sizeof(TTileLightInfo), MaxTileCountX * MaxTileCountY);
	}
}

void TRender::Draw(const GameTimer& gt)
{
	D3D12RHI->ResetCommandAllocator();

	D3D12RHI->ResetCommandList();

	SetDescriptorHeaps();

	if (bEnableIBLEnvLighting && FrameCount == 0)
	{
		CreateIBLEnviromentMap();

		CreateIBLIrradianceMap();

		CreateIBLPrefilterEnvMap();
	}

	GatherAllMeshBatchs();

	UpdateLightData();

	ShadowPass();

	BasePass();

	if (RenderSettings.bEnableSSR)
	{
		BackDepthPass();
	}

	if (RenderSettings.bEnableSSAO)
	{
		SSAOPass();
	}

	PrimitivesPass();

	if (RenderSettings.bUseTBDR)
	{
		TiledBaseLightCullingPass();
	}

	UpdateSDFData();

	DeferredLightingPass();

	if (RenderSettings.bEnableSSR)
	{
		SSRPass();
	}

	if (RenderSettings.bEnableTAA)
	{
		TAAPass();
	}

	SpritePass();

	PostProcessPass();

	if (RenderSettings.bDebugSDFScene)
	{
		DebugSDFScenePass();
	}

	D3D12RHI->ExecuteCommandLists();

	D3D12RHI->Present();

	D3D12RHI->FlushCommandQueue();
}

void TRender::EndFrame()
{
	D3D12RHI->EndFrame();

	FrameCount++;
}

void TRender::SetDescriptorHeaps()
{
	auto CacheCbvSrvUavDescriptorHeap = D3D12RHI->GetDevice()->GetCommandContext()->GetDescriptorCache()->GetCacheCbvSrvUavDescriptorHeap();
	ID3D12DescriptorHeap* DescriptorHeaps[] = { CacheCbvSrvUavDescriptorHeap.Get() };
	CommandList->SetDescriptorHeaps(1, DescriptorHeaps);
}

void TRender::GetSkyInfo()
{
	auto SkyActors = World->GetAllActorsOfClass<TSkyActor>();

	if (SkyActors.size() > 0)
	{
		assert(SkyActors.size() == 1);
		auto Sky = SkyActors[0];
		SkyMeshComponent = Sky->MeshComponent;
		SkyCubeTextureName = SkyMeshComponent->GetMaterialInstance()->Parameters.TextureMap["SkyCubeTexture"];
	}
}

void TRender::UpdateIBLEnviromentPassCB()
{
	for (int i = 0; i < 6; i++)
	{
		TMatrix View = IBLEnvironmentMap->GetSceneView(i).View;
		TMatrix Proj = IBLEnvironmentMap->GetSceneView(i).Proj;

		PassConstants PassCB;
		PassCB.View = View.Transpose();
		PassCB.Proj = Proj.Transpose();

		IBLEnviromentPassCBRef[i] = D3D12RHI->CreateConstantBuffer(&PassCB, sizeof(PassCB));
	}
}

void TRender::CreateIBLEnviromentMap()
{
	UpdateIBLEnviromentPassCB();

	D3D12RHI->TransitionResource(IBLEnvironmentMap->GetRTCube()->GetResource(), D3D12_RESOURCE_STATE_RENDER_TARGET);

	// Set viewport
	CommandList->RSSetViewports(1, &IBLEnvironmentMap->GetViewport());
	CommandList->RSSetScissorRects(1, &IBLEnvironmentMap->GetScissorRect());

	// Set PSO
	CommandList->SetPipelineState(GraphicsPSOManager->GetPSO(IBLEnvironmentPSODescriptor));

	// Set RootSignature
	TShader* Shader = IBLEnvironmentPSODescriptor.Shader;
	CommandList->SetGraphicsRootSignature(Shader->RootSignature.Get()); //should before binding

	for (UINT i = 0; i < 6; i++)
	{
		// Set renderTarget
		auto RTV = IBLEnvironmentMap->GetRTCube()->GetRTV(i);
		float* ClearValue = IBLEnvironmentMap->GetRTCube()->GetClearColorPtr();
		CommandList->ClearRenderTargetView(RTV->GetDescriptorHandle(), ClearValue, 0, nullptr);
		CommandList->OMSetRenderTargets(1, &RTV->GetDescriptorHandle(), true, nullptr);

		// Draw Box
		{
			Shader->SetParameter("cbPass", IBLEnviromentPassCBRef[i]);

			auto& TextureMap = TTextureRepository::Get().TextureMap;
			auto& EquirectangularSRV = TextureMap[SkyCubeTextureName]->GetD3DTexture()->SRVs[0];
			Shader->SetParameter("EquirectangularMap", EquirectangularSRV.get());

			// Bind paramters
			Shader->BindParameters();

			const TMeshProxy& MeshProxy = MeshProxyMap.at("BoxMesh");

			// Set vertex buffer
			D3D12RHI->SetVertexBuffer(MeshProxy.VertexBufferRef, 0, MeshProxy.VertexByteStride, MeshProxy.VertexBufferByteSize);

			// Set index buffer
			D3D12RHI->SetIndexBuffer(MeshProxy.IndexBufferRef, 0, MeshProxy.IndexFormat, MeshProxy.IndexBufferByteSize);

			D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
			CommandList->IASetPrimitiveTopology(PrimitiveType);

			// Draw 
			auto& SubMesh = MeshProxy.SubMeshs.at("Default");
			CommandList->DrawIndexedInstanced(SubMesh.IndexCount, 1, SubMesh.StartIndexLocation, SubMesh.BaseVertexLocation, 0);
		}
	}

	// Change back to GENERIC_READ so we can read the texture in a shader.
	D3D12RHI->TransitionResource(IBLEnvironmentMap->GetRTCube()->GetResource(), D3D12_RESOURCE_STATE_GENERIC_READ);
}

void TRender::UpdateIBLIrradiancePassCB()
{
	for (int i = 0; i < 6; i++)
	{
		TMatrix View = IBLIrradianceMap->GetSceneView(i).View;
		TMatrix Proj = IBLIrradianceMap->GetSceneView(i).Proj;

		PassConstants PassCB;
		PassCB.View = View.Transpose();
		PassCB.Proj = Proj.Transpose();

		IBLIrradiancePassCBRef[i] = D3D12RHI->CreateConstantBuffer(&PassCB, sizeof(PassCB));
	}
}

void TRender::CreateIBLIrradianceMap()
{
	UpdateIBLIrradiancePassCB();

	D3D12RHI->TransitionResource(IBLIrradianceMap->GetRTCube()->GetResource(), D3D12_RESOURCE_STATE_RENDER_TARGET);

	// Set viewport
	CommandList->RSSetViewports(1, &IBLIrradianceMap->GetViewport());
	CommandList->RSSetScissorRects(1, &IBLIrradianceMap->GetScissorRect());

	// Set PSO
	CommandList->SetPipelineState(GraphicsPSOManager->GetPSO(IBLIrradiancePSODescriptor));

	// Set RootSignature
	TShader* Shader = IBLIrradiancePSODescriptor.Shader;
	CommandList->SetGraphicsRootSignature(Shader->RootSignature.Get()); //should before binding

	for (UINT i = 0; i < 6; i++)
	{
		// Set renderTarget
		auto RTV = IBLIrradianceMap->GetRTCube()->GetRTV(i);
		float* ClearValue = IBLIrradianceMap->GetRTCube()->GetClearColorPtr();
		CommandList->ClearRenderTargetView(RTV->GetDescriptorHandle(), ClearValue, 0, nullptr);
		CommandList->OMSetRenderTargets(1, &RTV->GetDescriptorHandle(), true, nullptr);

		// Draw Box
		{
			Shader->SetParameter("cbPass", IBLIrradiancePassCBRef[i]);
	
			TD3D12ShaderResourceView* EnvironmentSRV = IBLEnvironmentMap->GetRTCube()->GetSRV();
			Shader->SetParameter("EnvironmentMap", EnvironmentSRV);

			// Bind paramters
			Shader->BindParameters();

			const TMeshProxy& MeshProxy = MeshProxyMap.at("BoxMesh");

			// Set vertex buffer
			D3D12RHI->SetVertexBuffer(MeshProxy.VertexBufferRef, 0, MeshProxy.VertexByteStride, MeshProxy.VertexBufferByteSize);

			// Set index buffer
			D3D12RHI->SetIndexBuffer(MeshProxy.IndexBufferRef, 0, MeshProxy.IndexFormat, MeshProxy.IndexBufferByteSize);

			D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
			CommandList->IASetPrimitiveTopology(PrimitiveType);

			// Draw 
			auto& SubMesh = MeshProxy.SubMeshs.at("Default");
			CommandList->DrawIndexedInstanced(SubMesh.IndexCount, 1, SubMesh.StartIndexLocation, SubMesh.BaseVertexLocation, 0);
		}
	}

	// Change back to GENERIC_READ so we can read the texture in a shader.
	D3D12RHI->TransitionResource(IBLIrradianceMap->GetRTCube()->GetResource(), D3D12_RESOURCE_STATE_GENERIC_READ);
}

void TRender::UpdateIBLPrefilterEnvCB()
{
	for (UINT Mip = 0; Mip < IBLPrefilterMaxMipLevel; Mip++)
	{
		float Roughness = (float)Mip / (float)(IBLPrefilterMaxMipLevel - 1);

		for (UINT i = 0; i < 6; i++)
		{
			TMatrix View = IBLPrefilterEnvMaps[Mip]->GetSceneView(i).View;
			TMatrix Proj = IBLPrefilterEnvMaps[Mip]->GetSceneView(i).Proj;

			PrefilterEnvironmentConstant PassCB;
			PassCB.View = View.Transpose();
			PassCB.Proj = Proj.Transpose();
			PassCB.Roughness = Roughness;

			IBLPrefilterEnvPassCBRef[Mip * 6 + i] = D3D12RHI->CreateConstantBuffer(&PassCB, sizeof(PassCB));
		}
	}
}

void TRender::CreateIBLPrefilterEnvMap()
{
	UpdateIBLPrefilterEnvCB();

	for (UINT Mip = 0; Mip < IBLPrefilterMaxMipLevel; Mip++)
	{
		D3D12RHI->TransitionResource(IBLPrefilterEnvMaps[Mip]->GetRTCube()->GetResource(), D3D12_RESOURCE_STATE_RENDER_TARGET);

		// Set viewport
		CommandList->RSSetViewports(1, &IBLPrefilterEnvMaps[Mip]->GetViewport());
		CommandList->RSSetScissorRects(1, &IBLPrefilterEnvMaps[Mip]->GetScissorRect());

		// Set PSO
		CommandList->SetPipelineState(GraphicsPSOManager->GetPSO(IBLPrefilterEnvPSODescriptor));

		// Set RootSignature
		CommandList->SetGraphicsRootSignature(IBLPrefilterEnvShader->RootSignature.Get()); //should before binding	

		for (UINT i = 0; i < 6; i++)
		{
			// Set renderTarget
			auto RTV = IBLPrefilterEnvMaps[Mip]->GetRTCube()->GetRTV(i);
			float* ClearValue = IBLPrefilterEnvMaps[Mip]->GetRTCube()->GetClearColorPtr();
			CommandList->ClearRenderTargetView(RTV->GetDescriptorHandle(), ClearValue, 0, nullptr);
			CommandList->OMSetRenderTargets(1, &RTV->GetDescriptorHandle(), true, nullptr);

			// Draw Box
			{
				IBLPrefilterEnvShader->SetParameter("cbPrefilterEnvPass", IBLPrefilterEnvPassCBRef[Mip * 6 + i]);


				TD3D12ShaderResourceView* EnvironmentSRV = IBLEnvironmentMap->GetRTCube()->GetSRV();
				IBLPrefilterEnvShader->SetParameter("EnvironmentMap", EnvironmentSRV);

				// Bind paramters
				IBLPrefilterEnvShader->BindParameters();

				const TMeshProxy& MeshProxy = MeshProxyMap.at("BoxMesh");

				// Set vertex buffer
				D3D12RHI->SetVertexBuffer(MeshProxy.VertexBufferRef, 0, MeshProxy.VertexByteStride, MeshProxy.VertexBufferByteSize);

				// Set index buffer
				D3D12RHI->SetIndexBuffer(MeshProxy.IndexBufferRef, 0, MeshProxy.IndexFormat, MeshProxy.IndexBufferByteSize);

				D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
				CommandList->IASetPrimitiveTopology(PrimitiveType);

				// Draw 
				auto& SubMesh = MeshProxy.SubMeshs.at("Default");
				CommandList->DrawIndexedInstanced(SubMesh.IndexCount, 1, SubMesh.StartIndexLocation, SubMesh.BaseVertexLocation, 0);
			}
		}

		// Change back to GENERIC_READ so we can read the texture in a shader.
		D3D12RHI->TransitionResource(IBLPrefilterEnvMaps[Mip]->GetRTCube()->GetResource(), D3D12_RESOURCE_STATE_GENERIC_READ);
	}
}

void TRender::GatherAllMeshBatchs()
{
	MeshBatchs.clear();

	// Get all mesh components in world
	auto Actors = World->GetActors();
	std::vector<TMeshComponent*> AllMeshComponents;

	for (auto Actor : Actors)
	{
		auto Light = dynamic_cast<TLightActor*>(Actor);
		if (Light && !Light->IsDrawMesh())
		{
			continue;
		}	

		auto MeshComponents = Actor->GetComponentsOfClass<TMeshComponent>();
		for (auto MeshComponent : MeshComponents)
		{
			if (MeshComponent->IsMeshValid())
			{
				AllMeshComponents.push_back(MeshComponent);
			}
		}		
	}

	// Calculate BoundingFrustum in view space
	TCameraComponent* CameraComponent = World->GetCameraComponent();
	TMatrix ViewToWorld = CameraComponent->GetView().Invert();

	BoundingFrustum ViewSpaceFrustum;
	BoundingFrustum::CreateFromMatrix(ViewSpaceFrustum, CameraComponent->GetProj());	

	// Culling
	//World->DrawString(11, "Mesh count before culling: " + std::to_string(AllMeshComponents.size()), 0.1f);

	std::vector<TMeshComponent*> MeshComponentsAfterCulling;
	for (auto MeshComponent : AllMeshComponents)
	{		
		TMatrix WorldToLocal = MeshComponent->GetWorldTransform().GetTransformMatrix().Invert();
		TMatrix ViewToLocal = ViewToWorld * WorldToLocal;

		// Transform the frustum from view space to the object's local space.
		BoundingFrustum LocalSpaceFrustum;

		// Note: BoundingFrustum::Transform( BoundingFrustum& Out, FXMMATRIX M) cannot contain a scale transform.
		// Ref: https://docs.microsoft.com/en-us/windows/win32/api/directxcollision/nf-directxcollision-boundingfrustum-transform
		// So it will have problems when actor has scale transform !!!
		// TODO: fix it
		ViewSpaceFrustum.Transform(LocalSpaceFrustum, ViewToLocal);

		TBoundingBox BoundingBox;
		if (bEnableFrustumCulling && MeshComponent->GetLocalBoundingBox(BoundingBox))
		{
			if (LocalSpaceFrustum.Contains(BoundingBox.GetD3DBox()) != DirectX::DISJOINT)
			{
				MeshComponentsAfterCulling.push_back(MeshComponent);
			}
		}
		else
		{
			MeshComponentsAfterCulling.push_back(MeshComponent);
		}
	}

	//World->DrawString(12, "Mesh count after culling: " + std::to_string(MeshComponentsAfterCulling.size()), 0.1f);


	// Generate MeshBatchs
	for (auto MeshComponent : MeshComponentsAfterCulling)
	{
		std::string MeshName = MeshComponent->GetMeshName();

		TMeshBatch MeshBatch;
		MeshBatch.MeshName = MeshName;
		MeshBatch.InputLayoutName = TMeshRepository::Get().MeshMap.at(MeshName).GetInputLayoutName();

		//Create Object ConstantBuffer		
		TMatrix World = MeshComponent->GetWorldTransform().GetTransformMatrix();
		TMatrix PrevWorld = MeshComponent->GetPrevWorldTransform().GetTransformMatrix();
		TMatrix TexTransform = MeshComponent->TexTransform;

		ObjectConstants ObjConst;
		ObjConst.World = World.Transpose();
		ObjConst.PrevWorld = PrevWorld.Transpose();
		ObjConst.TexTransform = TexTransform.Transpose();
		MeshBatch.ObjConstantBuffer = D3D12RHI->CreateConstantBuffer(&ObjConst, sizeof(ObjConst));

		MeshBatch.MeshComponent = MeshComponent;
		MeshBatch.bUseSDF = MeshComponent->bUseSDF;

		//Add to list
		MeshBatchs.emplace_back(MeshBatch);
	}	
}

TMatrix TRender::TextureTransform()
{
	TMatrix T(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);

	return T;
}

void TRender::UpdateLightData()
{
	std::vector<TLightShaderParameters> LightShaderParametersArray;

	ShadowMapSRVs.clear();
	ShadowMapCubeSRVs.clear();

	//---------------------------Direct Lights----------------------------------------------------------//

	auto Lights = World->GetAllActorsOfClass<TLightActor>();

	for (UINT LightIdx = 0; LightIdx < Lights.size(); LightIdx++)
	{
		auto Light = Lights[LightIdx];

		if (Light->GetType() == ELightType::DirectionalLight)
		{
			auto DirectionalLight = dynamic_cast<TDirectionalLightActor*>(Light);
			assert(DirectionalLight);

			TLightShaderParameters LightShaderParameter;
			LightShaderParameter.Color = DirectionalLight->GetLightColor();
			LightShaderParameter.Intensity = DirectionalLight->GetLightIntensity();
			LightShaderParameter.Position = DirectionalLight->GetActorLocation();
			LightShaderParameter.Direction = DirectionalLight->GetLightDirection();
			LightShaderParameter.LightType = ELightType::DirectionalLight;

			// Update shadow info
			if (DirectionalLight->IsCastShadows())
			{
				DirectionalLight->UpdateShadowData(D3D12RHI, RenderSettings.ShadowMapImpl);
				TShadowMap2D* ShadowMap = DirectionalLight->GetShadowMap();

				if (RenderSettings.ShadowMapImpl == EShadowMapImpl::PCF || RenderSettings.ShadowMapImpl == EShadowMapImpl::PCSS)
				{
					ShadowMapSRVs.push_back(ShadowMap->GetRT()->GetSRV());
				}
				else if (RenderSettings.ShadowMapImpl == EShadowMapImpl::VSM)
				{
					TD3D12TextureRef VSMTexture = DirectionalLight->GetVSMTexture();
					ShadowMapSRVs.push_back(VSMTexture->SRVs[0].get());
				}
				assert(ShadowMapSRVs.size() <= MAX_SHADOW_MAP_2D_NUM);

				LightShaderParameter.ShadowMapIdx = INT(ShadowMapSRVs.size() - 1);
				TMatrix LightView = ShadowMap->GetSceneView().View;
				TMatrix LightProj = ShadowMap->GetSceneView().Proj;
				LightShaderParameter.LightProj = LightProj.Transpose();
				TMatrix ViewProjTex = LightView * LightProj * TextureTransform();
				LightShaderParameter.ShadowTransform = ViewProjTex.Transpose();
			}
			else
			{
				LightShaderParameter.ShadowMapIdx = -1;
			}

			LightShaderParametersArray.push_back(LightShaderParameter);			
		}
		else if (Light->GetType() == ELightType::PointLight)
		{
			auto PointLight = dynamic_cast<TPointLightActor*>(Light);
			assert(PointLight);

			TLightShaderParameters LightShaderParameter;
			LightShaderParameter.Color = PointLight->GetLightColor();
			LightShaderParameter.Intensity = PointLight->GetLightIntensity();
			LightShaderParameter.Position = PointLight->GetActorLocation();
			LightShaderParameter.Range = PointLight->GetAttenuationRange();
			LightShaderParameter.LightType = ELightType::PointLight;

			// Update shadow info
			if (PointLight->IsCastShadows())
			{
				PointLight->UpdateShadowData(D3D12RHI);

				TShadowMapCube* ShadowMap = PointLight->GetShadowMap();
				ShadowMapCubeSRVs.push_back(ShadowMap->GetRTCube()->GetSRV());
				assert(ShadowMapCubeSRVs.size() <= MAX_SHADOW_MAP_CUBE_NUM);

				LightShaderParameter.ShadowMapIdx = UINT(ShadowMapCubeSRVs.size() - 1);
				//LightShaderParameter.LightProj        //Don't need to set
				//LightShaderParameter.ShadowTransform  //Don't need to set
			}
			else
			{
				LightShaderParameter.ShadowMapIdx = -1;
			}
					
			LightShaderParametersArray.push_back(LightShaderParameter);
		}
		else if (Light->GetType() == ELightType::SpotLight)
		{
			auto SpotLight = dynamic_cast<TSpotLightActor*>(Light);
			assert(SpotLight);

			TLightShaderParameters LightShaderParameter;
			LightShaderParameter.Color = SpotLight->GetLightColor();
			LightShaderParameter.Intensity = SpotLight->GetLightIntensity();
			LightShaderParameter.Position = SpotLight->GetActorLocation();
			LightShaderParameter.Range = SpotLight->GetAttenuationRange();
			LightShaderParameter.Direction = SpotLight->GetLightDirection();

			float ClampedInnerConeAngle = std::clamp(SpotLight->GetInnerConeAngle(), 0.0f, 89.0f);
			float ClampedOuterConeAngle = std::clamp(SpotLight->GetOuterConeAngle(), ClampedInnerConeAngle + 0.001f, 89.0f + 0.001f);
			ClampedInnerConeAngle *= (TMath::Pi / 180.0f);
			ClampedOuterConeAngle *= (TMath::Pi / 180.0f);
			float CosInnerCone = cos(ClampedInnerConeAngle);
			float CosOuterCone = cos(ClampedOuterConeAngle);
			float InvCosConeDifference = 1.0f / (CosInnerCone - CosOuterCone);

			LightShaderParameter.SpotAngles = TVector2(CosOuterCone, InvCosConeDifference);
			LightShaderParameter.SpotRadius = SpotLight->GetBottomRadius();
			LightShaderParameter.LightType = ELightType::SpotLight;

			// Update shadow info
			if (SpotLight->IsCastShadows())
			{
				SpotLight->UpdateShadowData(D3D12RHI, RenderSettings.ShadowMapImpl);
				TShadowMap2D* ShadowMap = SpotLight->GetShadowMap();

				if (RenderSettings.ShadowMapImpl == EShadowMapImpl::PCF || RenderSettings.ShadowMapImpl == EShadowMapImpl::PCSS)
				{
					ShadowMapSRVs.push_back(ShadowMap->GetRT()->GetSRV());
				}
				else if (RenderSettings.ShadowMapImpl == EShadowMapImpl::VSM)
				{
					TD3D12TextureRef VSMTexture = SpotLight->GetVSMTexture();
					ShadowMapSRVs.push_back(VSMTexture->SRVs[0].get());
				}
				assert(ShadowMapSRVs.size() <= MAX_SHADOW_MAP_2D_NUM);

				LightShaderParameter.ShadowMapIdx = UINT(ShadowMapSRVs.size() - 1);
				TMatrix LightView = ShadowMap->GetSceneView().View;
				TMatrix LightProj = ShadowMap->GetSceneView().Proj;
				LightShaderParameter.LightProj = LightProj.Transpose();
				TMatrix ViewProjTex = LightView * LightProj * TextureTransform();
				LightShaderParameter.ShadowTransform = ViewProjTex.Transpose();
			}
			else
			{
				LightShaderParameter.ShadowMapIdx = -1;
			}

			LightShaderParametersArray.push_back(LightShaderParameter);
		}

		else if (Light->GetType() == ELightType::AreaLight)
		{
			auto AreaLight = dynamic_cast<TAreaLightActor*>(Light);
			assert(AreaLight);

			TMatrix LightToWorld = AreaLight->GetActorTransform().GetTransformMatrix();

			TLightShaderParameters LightShaderParameter;
			LightShaderParameter.Color = AreaLight->GetLightColor();
			LightShaderParameter.Intensity = AreaLight->GetLightIntensity();
			LightShaderParameter.AreaLightPoint0InWorld = LightToWorld.Transform(TVector3(-0.5f, 0.5f, 5.0f));
			LightShaderParameter.AreaLightPoint1InWorld = LightToWorld.Transform(TVector3(-0.5f, -0.5f, 5.0f));
			LightShaderParameter.AreaLightPoint2InWorld = LightToWorld.Transform(TVector3(0.5f, -0.5f, 5.0f));
			LightShaderParameter.AreaLightPoint3InWorld = LightToWorld.Transform(TVector3(0.5f, 0.5f, 5.0f));
			LightShaderParameter.LightType = ELightType::AreaLight;
			LightShaderParameter.ShadowMapIdx = -1;
			LightShaderParametersArray.push_back(LightShaderParameter);
		}
	}

	//---------------------------IBL ambient Light----------------------------------------------------------//	
    if(bEnableIBLEnvLighting)
	{
		TLightShaderParameters LightShaderParameter;
		LightShaderParameter.LightType = ELightType::AmbientLight;
		LightShaderParametersArray.push_back(LightShaderParameter);
	}

	LightCount = (UINT)LightShaderParametersArray.size();
	if (LightCount > 0)
	{
		LightShaderParametersBuffer = D3D12RHI->CreateStructuredBuffer(LightShaderParametersArray.data(),
			(uint32_t)(sizeof(TLightShaderParameters)), (uint32_t)(LightShaderParametersArray.size()));
	}
	else
	{
		LightShaderParametersBuffer = nullptr;
	}

	TLightCommonData LightCommonData;
	LightCommonData.LightCount = LightCount;
	LightCommonDataBuffer = D3D12RHI->CreateConstantBuffer(&LightCommonData, sizeof(LightCommonData));
}

void TRender::UpdateShadowPassCB(const TSceneView& SceneView, UINT ShadowWidth, UINT ShadowHeight)
{
	TMatrix LightView = SceneView.View;
	TMatrix LightProj = SceneView.Proj;
	TVector3 LightPos = SceneView.EyePos;

	TMatrix ViewProj = LightView * LightProj;
	TMatrix InvView = LightView.Invert();
	TMatrix InvProj = LightProj.Invert();
	TMatrix InvViewProj = ViewProj.Invert();

	PassConstants ShadowPassCB;
	ShadowPassCB.View = LightView.Transpose();
	ShadowPassCB.Proj = LightProj.Transpose();
	ShadowPassCB.ViewProj = ViewProj.Transpose();
	ShadowPassCB.InvView = InvView.Transpose();
	ShadowPassCB.InvProj = InvProj.Transpose();
	ShadowPassCB.InvViewProj = InvViewProj.Transpose();
	ShadowPassCB.EyePosW = LightPos;

	ShadowPassCB.RenderTargetSize = TVector2((float)ShadowWidth, (float)ShadowHeight);
	ShadowPassCB.InvRenderTargetSize = TVector2(1.0f / ShadowWidth, 1.0f / ShadowHeight);
	ShadowPassCB.NearZ = SceneView.Near;
	ShadowPassCB.FarZ = SceneView.Far;


	ShadowPassCBRef = D3D12RHI->CreateConstantBuffer(&ShadowPassCB, sizeof(ShadowPassCB));
}

void TRender::GetShadowPassMeshCommandMap(EShadowMapType Type)
{
	ShadowMeshCommandMap.clear();

	for (const TMeshBatch& MeshBatch : MeshBatchs)
	{
		// Create MeshCommand
		TMeshCommand MeshCommand;
		MeshCommand.MeshName = MeshBatch.MeshName;

		auto MaterialInstance = MeshBatch.MeshComponent->GetMaterialInstance();
		// Get material constanct buffer
		if (MaterialInstance->MaterialConstantBuffer == nullptr)
		{
			MaterialInstance->CreateMaterialConstantBuffer(D3D12RHI);
		}

		// Set shader paramters
		MeshCommand.SetShaderParameter("cbMaterialData", MaterialInstance->MaterialConstantBuffer);
		MeshCommand.SetShaderParameter("cbPerObject", MeshBatch.ObjConstantBuffer);
		MeshCommand.SetShaderParameter("cbPass", ShadowPassCBRef);

		// Get PSO descriptor of this mesh
		TGraphicsPSODescriptor Descriptor;
		Descriptor.InputLayoutName = MeshBatch.InputLayoutName;
		if (Type == EShadowMapType::SM_SINGLE)
		{
			Descriptor.Shader = SingleShadowMapShader.get();
		}
		else
		{
			Descriptor.Shader = OmniShadowMapShader.get();
		}

		// We will adjust shadow sample value according to depth derivative in shader,
		// So don't need to enable hardware depth bias
		//Descriptor.RasterizerDesc.DepthBias = 100000;
		//Descriptor.RasterizerDesc.DepthBiasClamp = 0.0f;
		//Descriptor.RasterizerDesc.SlopeScaledDepthBias = 1.0f;
	
		// Shadow map pass does not have a render target.
		Descriptor.RTVFormats[0] = DXGI_FORMAT_UNKNOWN;
		Descriptor.NumRenderTargets = 0;

		// Create a new PSO if we don't have the pso with this descriptor
		GraphicsPSOManager->TryCreatePSO(Descriptor);

		// Save MeshCommand according to PSO type
		ShadowMeshCommandMap.insert({Descriptor, TMeshCommandList()});
		ShadowMeshCommandMap[Descriptor].emplace_back(MeshCommand);
	}
}

void TRender::ShadowPass()
{
	auto Lights = World->GetAllActorsOfClass<TLightActor>();

	for (UINT LightIdx = 0; LightIdx < Lights.size(); LightIdx++)
	{
		auto Light = Lights[LightIdx];

		if (!Light->IsCastShadows())
		{
			continue;
		}

		if (Light->GetType() == ELightType::DirectionalLight)
		{
			auto DirectionalLight = dynamic_cast<TDirectionalLightActor*>(Light);
			assert(DirectionalLight);

			GenerateSingleShadowMap(DirectionalLight->GetShadowMap());

			if (RenderSettings.ShadowMapImpl == EShadowMapImpl::VSM)
			{
				GenerateVSMShadow(DirectionalLight->GetShadowMap(), DirectionalLight->GetVSMTexture(), DirectionalLight->GetVSMBlurTexture());
			}
		}
		else if (Light->GetType() == ELightType::SpotLight)
		{
			auto SpotLight = dynamic_cast<TSpotLightActor*>(Light);
			assert(SpotLight);

			GenerateSingleShadowMap(SpotLight->GetShadowMap());

			if (RenderSettings.ShadowMapImpl == EShadowMapImpl::VSM)
			{
				GenerateVSMShadow(SpotLight->GetShadowMap(), SpotLight->GetVSMTexture(), SpotLight->GetVSMBlurTexture());
			}
		}
		else if (Light->GetType() == ELightType::PointLight)
		{
			auto PointLight = dynamic_cast<TPointLightActor*>(Light);
			assert(PointLight);

			GenerateOmniShadowMap(PointLight->GetShadowMap());
		}
	}
}

void TRender::GenerateSingleShadowMap(TShadowMap2D* ShadowMap)
{
	UpdateShadowPassCB(ShadowMap->GetSceneView(), ShadowMap->GetWidth(), ShadowMap->GetHeight());

	GetShadowPassMeshCommandMap(EShadowMapType::SM_SINGLE);

	// Change to DEPTH_WRITE.
	D3D12RHI->TransitionResource(ShadowMap->GetRT()->GetResource(), D3D12_RESOURCE_STATE_DEPTH_WRITE);

	CommandList->RSSetViewports(1, &ShadowMap->GetViewport());
	CommandList->RSSetScissorRects(1, &ShadowMap->GetScissorRect());

	// Clear depth buffer.
	CommandList->ClearDepthStencilView(ShadowMap->GetRT()->GetDSV()->GetDescriptorHandle(),
		D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	// Set null render target because we are only going to draw to
	// depth buffer.  Setting a null render target will disable color writes.
	// Note the active PSO also must specify a render target count of 0.
	CommandList->OMSetRenderTargets(0, nullptr, false, &ShadowMap->GetRT()->GetDSV()->GetDescriptorHandle());

	//Draw all mesh
	for (const auto& Pair : ShadowMeshCommandMap)
	{
		const TGraphicsPSODescriptor& PSODescriptor = Pair.first;
		const TMeshCommandList& MeshCommandList = Pair.second;

		// Set PSO
		CommandList->SetPipelineState(GraphicsPSOManager->GetPSO(PSODescriptor));

		// Set RootSignature
		TShader* Shader = PSODescriptor.Shader;
		CommandList->SetGraphicsRootSignature(Shader->RootSignature.Get()); //should before binding

		for (const TMeshCommand& MeshCommand : MeshCommandList)
		{
			// Set paramters
			MeshCommand.ApplyShaderParamters(Shader);

			// Bind paramters
			Shader->BindParameters();

			const TMeshProxy& MeshProxy = MeshProxyMap.at(MeshCommand.MeshName);

			// Set vertex buffer
			D3D12RHI->SetVertexBuffer(MeshProxy.VertexBufferRef, 0, MeshProxy.VertexByteStride, MeshProxy.VertexBufferByteSize);

			// Set index buffer
			D3D12RHI->SetIndexBuffer(MeshProxy.IndexBufferRef, 0, MeshProxy.IndexFormat, MeshProxy.IndexBufferByteSize);

			D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
			CommandList->IASetPrimitiveTopology(PrimitiveType);

			// Draw 
			auto& SubMesh = MeshProxy.SubMeshs.at("Default");
			CommandList->DrawIndexedInstanced(SubMesh.IndexCount, 1, SubMesh.StartIndexLocation, SubMesh.BaseVertexLocation, 0);
		}
	}

	// Change back to GENERIC_READ so we can read the texture in a shader.
	D3D12RHI->TransitionResource(ShadowMap->GetRT()->GetResource(), D3D12_RESOURCE_STATE_GENERIC_READ);
}

void TRender::GenerateOmniShadowMap(TShadowMapCube* ShadowMap)
{
	for (UINT i = 0; i < 6; i++)
	{
		UpdateShadowPassCB(ShadowMap->GetSceneView(i), ShadowMap->GetCubeMapSize(), ShadowMap->GetCubeMapSize());

		GetShadowPassMeshCommandMap(EShadowMapType::SM_OMNI);

		// Change to DEPTH_WRITE.
		D3D12RHI->TransitionResource(ShadowMap->GetRTCube()->GetResource(), D3D12_RESOURCE_STATE_DEPTH_WRITE);

		CommandList->RSSetViewports(1, &ShadowMap->GetViewport());
		CommandList->RSSetScissorRects(1, &ShadowMap->GetScissorRect());

		// Clear depth buffer.
		CommandList->ClearDepthStencilView(ShadowMap->GetRTCube()->GetDSV(i)->GetDescriptorHandle(),
			D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

		// Set null render target because we are only going to draw to
		// depth buffer.  Setting a null render target will disable color writes.
		// Note the active PSO also must specify a render target count of 0.
		CommandList->OMSetRenderTargets(0, nullptr, false, &ShadowMap->GetRTCube()->GetDSV(i)->GetDescriptorHandle());

		//Draw all mesh
		for (const auto& Pair : ShadowMeshCommandMap)
		{
			const TGraphicsPSODescriptor& PSODescriptor = Pair.first;
			const TMeshCommandList& MeshCommandList = Pair.second;

			// Set PSO
			CommandList->SetPipelineState(GraphicsPSOManager->GetPSO(PSODescriptor));

			// Set RootSignature
			TShader* Shader = PSODescriptor.Shader;
			CommandList->SetGraphicsRootSignature(Shader->RootSignature.Get()); //should before binding

			for (const TMeshCommand& MeshCommand : MeshCommandList)
			{
				// Set paramters
				MeshCommand.ApplyShaderParamters(Shader);

				// Bind paramters
				Shader->BindParameters();

				const TMeshProxy& MeshProxy = MeshProxyMap.at(MeshCommand.MeshName);

				// Set vertex buffer
				D3D12RHI->SetVertexBuffer(MeshProxy.VertexBufferRef, 0, MeshProxy.VertexByteStride, MeshProxy.VertexBufferByteSize);

				// Set index buffer
				D3D12RHI->SetIndexBuffer(MeshProxy.IndexBufferRef, 0, MeshProxy.IndexFormat, MeshProxy.IndexBufferByteSize);

				D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
				CommandList->IASetPrimitiveTopology(PrimitiveType);

				// Draw 
				auto& SubMesh = MeshProxy.SubMeshs.at("Default");
				CommandList->DrawIndexedInstanced(SubMesh.IndexCount, 1, SubMesh.StartIndexLocation, SubMesh.BaseVertexLocation, 0);
			}
		}

		// Change back to GENERIC_READ so we can read the texture in a shader.
		D3D12RHI->TransitionResource(ShadowMap->GetRTCube()->GetResource(), D3D12_RESOURCE_STATE_GENERIC_READ);

	}
}

void TRender::GenerateVSMShadow(TShadowMap2D* ShadowMap, TD3D12TextureRef VSMTexture, TD3D12TextureRef VSMBlurTexture)
{
	//-------------------------------------------Get depth square-----------------------------------------

	// Set PSO
	CommandList->SetPipelineState(ComputePSOManager->GetPSO(GenerateVSMPSODescriptor));

	// Set RootSignature
	CommandList->SetComputeRootSignature(GenerateVSMShader->RootSignature.Get()); // should before binding

	// Set parameters
	GenerateVSMShader->SetParameter("ShadowMap", ShadowMap->GetRT()->GetSRV());
	GenerateVSMShader->SetParameter("VSM", VSMTexture->UAVs[0].get());

	// Bind paramters
	GenerateVSMShader->BindParameters();

	UINT NumGroupsX = (UINT)ceilf(ShadowSize / 16.0f);
	UINT NumGroupsY = (UINT)ceilf(ShadowSize / 16.0f);
	CommandList->Dispatch(NumGroupsX, NumGroupsY, 1);

	//--------------------------------------------HorzBlur--------------------------------------------

	// Set PSO
	CommandList->SetPipelineState(ComputePSOManager->GetPSO(HorzBlurVSMPSODescriptor));

	// Set RootSignature
	CommandList->SetComputeRootSignature(HorzBlurVSMShader->RootSignature.Get()); // should before binding

	// Set parameters
	HorzBlurVSMShader->SetParameter("InputTexture", VSMTexture->SRVs[0].get());
	HorzBlurVSMShader->SetParameter("OutputTexture", VSMBlurTexture->UAVs[0].get());
	HorzBlurVSMShader->SetParameter("cbBlurSettings", VSMBlurSettingsCB);

	// Bind paramters
	HorzBlurVSMShader->BindParameters();

	NumGroupsX = (UINT)ceilf(ShadowSize / 256.0f);
	NumGroupsY = ShadowSize;
	CommandList->Dispatch(NumGroupsX, NumGroupsY, 1);

	//--------------------------------------------VertBlur--------------------------------------------
	// Set PSO
	CommandList->SetPipelineState(ComputePSOManager->GetPSO(VertBlurVSMPSODescriptor));

	// Set RootSignature
	CommandList->SetComputeRootSignature(VertBlurVSMShader->RootSignature.Get()); // should before binding

	// Set parameters
	VertBlurVSMShader->SetParameter("InputTexture", VSMBlurTexture->SRVs[0].get());
	VertBlurVSMShader->SetParameter("OutputTexture", VSMTexture->UAVs[0].get());
	VertBlurVSMShader->SetParameter("cbBlurSettings", VSMBlurSettingsCB);

	// Bind paramters
	VertBlurVSMShader->BindParameters();

	NumGroupsX = ShadowSize;
	NumGroupsY = (UINT)ceilf(ShadowSize / 256.0f);
	CommandList->Dispatch(NumGroupsX, NumGroupsY, 1);
}

void TRender::UpdateBasePassCB()
{
	TCameraComponent* CameraComponent = World->GetCameraComponent();

	TMatrix View = CameraComponent->GetView();
	TMatrix Proj = CameraComponent->GetProj();

	if (RenderSettings.bEnableTAA)
	{
		UINT SampleIdx = FrameCount % TAA_SAMPLE_COUNT;
		double JitterX = Halton_2[SampleIdx] / (double)WindowWidth;
		double JitterY = Halton_3[SampleIdx] / (double)WindowHeight;
		Proj(2, 0) += (float)JitterX;
		Proj(2, 1) += (float)JitterY;
	}

	TMatrix ViewProj = View * Proj;
	TMatrix InvView = View.Invert();
	TMatrix InvProj = Proj.Invert();
	TMatrix InvViewProj = ViewProj.Invert();
	TMatrix PrevViewProj = CameraComponent->GetPrevViewProj();

	PassConstants BasePassCB;
	BasePassCB.View = View.Transpose();
	BasePassCB.Proj = Proj.Transpose();
	BasePassCB.ViewProj = ViewProj.Transpose();
	BasePassCB.InvView = InvView.Transpose();
	BasePassCB.InvProj = InvProj.Transpose();
	BasePassCB.InvViewProj = InvViewProj.Transpose();
	BasePassCB.PrevViewProj = PrevViewProj.Transpose();
	BasePassCB.EyePosW = CameraComponent->GetWorldLocation();
	BasePassCB.RenderTargetSize = TVector2((float)WindowWidth, (float)WindowHeight);
	BasePassCB.InvRenderTargetSize = TVector2(1.0f / WindowWidth, 1.0f / WindowHeight);
	BasePassCB.NearZ = CameraComponent->GetNearZ();
	BasePassCB.FarZ = CameraComponent->GetFarZ();

	BasePassCBRef = D3D12RHI->CreateConstantBuffer(&BasePassCB, sizeof(BasePassCB));
}

void TRender::GetBasePassMeshCommandMap()
{
	BaseMeshCommandMap.clear();

	for (const TMeshBatch& MeshBatch : MeshBatchs)
	{
		// Create MeshCommand
		TMeshCommand MeshCommand;
		MeshCommand.MeshName = MeshBatch.MeshName;

		auto MaterialInstance = MeshBatch.MeshComponent->GetMaterialInstance();
		MeshCommand.RenderState = MaterialInstance->Material->RenderState;

		// Get material constanct buffer
		if (MaterialInstance->MaterialConstantBuffer == nullptr)
		{
			MaterialInstance->CreateMaterialConstantBuffer(D3D12RHI);
		}

		// Set shader parameters
		MeshCommand.SetShaderParameter("cbMaterialData", MaterialInstance->MaterialConstantBuffer);
		MeshCommand.SetShaderParameter("cbPass", BasePassCBRef);
		MeshCommand.SetShaderParameter("cbPerObject", MeshBatch.ObjConstantBuffer);
		for (const auto& Pair : MaterialInstance->Parameters.TextureMap)
		{
			std::string TextureName = Pair.second;
			TD3D12ShaderResourceView* SRV = nullptr;

			if (TextureName == SkyCubeTextureName)
			{
				SRV = IBLEnvironmentMap->GetRTCube()->GetSRV();
			}
			else
			{
				SRV = TTextureRepository::Get().TextureMap[TextureName]->GetD3DTexture()->SRVs[0].get();
			}

			MeshCommand.SetShaderParameter(Pair.first, SRV);
		}

		// Get PSO descriptor of this mesh
		TGraphicsPSODescriptor Descriptor;
		Descriptor.InputLayoutName = MeshBatch.InputLayoutName;
		Descriptor.RasterizerDesc.CullMode = MeshCommand.RenderState.CullMode;
		Descriptor.DepthStencilDesc.DepthFunc = MeshCommand.RenderState.DepthFunc;

		TMaterial* Material = MaterialInstance->Material;
		TShaderDefines EmptyShaderDefines;
		Descriptor.Shader = Material->GetShader(EmptyShaderDefines, D3D12RHI);
		
		// GBuffer PSO common settings
		Descriptor.RTVFormats[0] = GBufferBaseColor->GetFormat();
		Descriptor.RTVFormats[1] = GBufferNormal->GetFormat();
		Descriptor.RTVFormats[2] = GBufferWorldPos->GetFormat();
		Descriptor.RTVFormats[3] = GBufferORM->GetFormat();
		Descriptor.RTVFormats[4] = GBufferVelocity->GetFormat();
		Descriptor.RTVFormats[5] = GBufferEmissive->GetFormat();
		Descriptor.NumRenderTargets = GBufferCount;
		Descriptor.DepthStencilFormat = D3D12RHI->GetViewportInfo().DepthStencilFormat;
		Descriptor._4xMsaaState = false; //can't use msaa in deferred rendering.

		// Create a new PSO if we don't have the pso with this descriptor
		GraphicsPSOManager->TryCreatePSO(Descriptor);

		// Save MeshCommand according to PSO type
		BaseMeshCommandMap.insert({ Descriptor, TMeshCommandList() });
		BaseMeshCommandMap[Descriptor].emplace_back(MeshCommand);
	}
}

void TRender::BasePass()
{
	UpdateBasePassCB();

	GetBasePassMeshCommandMap();

	// Use screen viewport 
	D3D12_VIEWPORT ScreenViewport;
	D3D12_RECT ScissorRect;
	D3D12RHI->GetViewport()->GetD3DViewport(ScreenViewport, ScissorRect);
	CommandList->RSSetViewports(1, &ScreenViewport);
	CommandList->RSSetScissorRects(1, &ScissorRect);

	// Transit to render target state
	D3D12RHI->TransitionResource(GBufferBaseColor->GetTexture()->GetResource(), D3D12_RESOURCE_STATE_RENDER_TARGET);
	D3D12RHI->TransitionResource(GBufferNormal->GetTexture()->GetResource(), D3D12_RESOURCE_STATE_RENDER_TARGET);
	D3D12RHI->TransitionResource(GBufferWorldPos->GetTexture()->GetResource(), D3D12_RESOURCE_STATE_RENDER_TARGET);
	D3D12RHI->TransitionResource(GBufferORM->GetTexture()->GetResource(), D3D12_RESOURCE_STATE_RENDER_TARGET);
	D3D12RHI->TransitionResource(GBufferVelocity->GetTexture()->GetResource(), D3D12_RESOURCE_STATE_RENDER_TARGET);
	D3D12RHI->TransitionResource(GBufferEmissive->GetTexture()->GetResource(), D3D12_RESOURCE_STATE_RENDER_TARGET);

	// Clear renderTargets
	CommandList->ClearRenderTargetView(GBufferBaseColor->GetRTV()->GetDescriptorHandle(), GBufferBaseColor->GetClearColorPtr(), 0, nullptr);
	CommandList->ClearRenderTargetView(GBufferNormal->GetRTV()->GetDescriptorHandle(), GBufferNormal->GetClearColorPtr(), 0, nullptr);
	CommandList->ClearRenderTargetView(GBufferWorldPos->GetRTV()->GetDescriptorHandle(), GBufferWorldPos->GetClearColorPtr(), 0, nullptr);
	CommandList->ClearRenderTargetView(GBufferORM->GetRTV()->GetDescriptorHandle(), GBufferORM->GetClearColorPtr(), 0, nullptr);
	CommandList->ClearRenderTargetView(GBufferVelocity->GetRTV()->GetDescriptorHandle(), GBufferVelocity->GetClearColorPtr(), 0, nullptr);
	CommandList->ClearRenderTargetView(GBufferEmissive->GetRTV()->GetDescriptorHandle(), GBufferEmissive->GetClearColorPtr(), 0, nullptr);

	// Clear depthstencil
	CommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	// Specify the renderTargets we are going to render to.
	std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> RtvDescriptors;
	RtvDescriptors.push_back(GBufferBaseColor->GetRTV()->GetDescriptorHandle());
	RtvDescriptors.push_back(GBufferNormal->GetRTV()->GetDescriptorHandle());
	RtvDescriptors.push_back(GBufferWorldPos->GetRTV()->GetDescriptorHandle());
	RtvDescriptors.push_back(GBufferORM->GetRTV()->GetDescriptorHandle());
	RtvDescriptors.push_back(GBufferVelocity->GetRTV()->GetDescriptorHandle());
	RtvDescriptors.push_back(GBufferEmissive->GetRTV()->GetDescriptorHandle());

	auto DescriptorCache = D3D12RHI->GetDevice()->GetCommandContext()->GetDescriptorCache();
	CD3DX12_GPU_DESCRIPTOR_HANDLE GpuHandle;
	CD3DX12_CPU_DESCRIPTOR_HANDLE CpuHandle;
	DescriptorCache->AppendRtvDescriptors(RtvDescriptors, GpuHandle, CpuHandle);

	CommandList->OMSetRenderTargets(GBufferCount, &CpuHandle, true, &DepthStencilView());

	// Draw all mesh
	for (const auto& Pair : BaseMeshCommandMap)
	{
		const TGraphicsPSODescriptor& PSODescriptor = Pair.first;
		const TMeshCommandList& MeshCommandList = Pair.second;

		// Set PSO
		CommandList->SetPipelineState(GraphicsPSOManager->GetPSO(PSODescriptor));

		// Set RootSignature
		TShader* Shader = PSODescriptor.Shader;
		CommandList->SetGraphicsRootSignature(Shader->RootSignature.Get()); //should before binding

		for (const TMeshCommand& MeshCommand : MeshCommandList)
		{
			auto& TextureMap = TTextureRepository::Get().TextureMap;

			// Set paramters
			MeshCommand.ApplyShaderParamters(Shader);

			// Bind paramters
			Shader->BindParameters();

			const TMeshProxy& MeshProxy = MeshProxyMap.at(MeshCommand.MeshName);

			// Set vertex buffer
			D3D12RHI->SetVertexBuffer(MeshProxy.VertexBufferRef, 0, MeshProxy.VertexByteStride, MeshProxy.VertexBufferByteSize);

			// Set index buffer
			D3D12RHI->SetIndexBuffer(MeshProxy.IndexBufferRef, 0, MeshProxy.IndexFormat, MeshProxy.IndexBufferByteSize);

			D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
			CommandList->IASetPrimitiveTopology(PrimitiveType);

			// Draw 
			auto& SubMesh = MeshProxy.SubMeshs.at("Default");
			CommandList->DrawIndexedInstanced(SubMesh.IndexCount, 1, SubMesh.StartIndexLocation, SubMesh.BaseVertexLocation, 0);
		}
	}

	// Transit to generic read state
	D3D12RHI->TransitionResource(GBufferBaseColor->GetTexture()->GetResource(), D3D12_RESOURCE_STATE_GENERIC_READ);
	D3D12RHI->TransitionResource(GBufferNormal->GetTexture()->GetResource(), D3D12_RESOURCE_STATE_GENERIC_READ);
	D3D12RHI->TransitionResource(GBufferWorldPos->GetTexture()->GetResource(), D3D12_RESOURCE_STATE_GENERIC_READ);
	D3D12RHI->TransitionResource(GBufferORM->GetTexture()->GetResource(), D3D12_RESOURCE_STATE_GENERIC_READ);
	D3D12RHI->TransitionResource(GBufferVelocity->GetTexture()->GetResource(), D3D12_RESOURCE_STATE_GENERIC_READ);
	D3D12RHI->TransitionResource(GBufferEmissive->GetTexture()->GetResource(), D3D12_RESOURCE_STATE_GENERIC_READ);
}

void TRender::GetBackDepthPassMeshCommandMap()
{
	BackDepthCommandMap.clear();

	for (const TMeshBatch& MeshBatch : MeshBatchs)
	{
		// Create MeshCommand
		TMeshCommand MeshCommand;
		MeshCommand.MeshName = MeshBatch.MeshName;

		auto MaterialInstance = MeshBatch.MeshComponent->GetMaterialInstance();
		// Get material constanct buffer
		if (MaterialInstance->MaterialConstantBuffer == nullptr)
		{
			MaterialInstance->CreateMaterialConstantBuffer(D3D12RHI);
		}

		// Set shader paramters
		MeshCommand.SetShaderParameter("cbMaterialData", MaterialInstance->MaterialConstantBuffer);
		MeshCommand.SetShaderParameter("cbPerObject", MeshBatch.ObjConstantBuffer);
		MeshCommand.SetShaderParameter("cbPass", BasePassCBRef);

		// Get PSO descriptor of this mesh
		TGraphicsPSODescriptor Descriptor;
		Descriptor.InputLayoutName = MeshBatch.InputLayoutName;
		Descriptor.Shader = BackDepthShader.get();
		// Shadow map pass does not have a render target.
		Descriptor.RTVFormats[0] = DXGI_FORMAT_UNKNOWN;
		Descriptor.NumRenderTargets = 0;
		Descriptor.RasterizerDesc.CullMode = D3D12_CULL_MODE_FRONT; // Cull front

		// Create a new PSO if we don't have the pso with this descriptor
		GraphicsPSOManager->TryCreatePSO(Descriptor);

		// Save MeshCommand according to PSO type
		BackDepthCommandMap.insert({ Descriptor, TMeshCommandList() });
		BackDepthCommandMap[Descriptor].emplace_back(MeshCommand);
	}
}

void TRender::BackDepthPass()
{
	UpdateBasePassCB();

	GetBackDepthPassMeshCommandMap();

	// Use screen viewport 
	D3D12_VIEWPORT ScreenViewport;
	D3D12_RECT ScissorRect;
	D3D12RHI->GetViewport()->GetD3DViewport(ScreenViewport, ScissorRect);
	CommandList->RSSetViewports(1, &ScreenViewport);
	CommandList->RSSetScissorRects(1, &ScissorRect);

	// Transit to render target state
	D3D12RHI->TransitionResource(BackDepth->GetResource(), D3D12_RESOURCE_STATE_DEPTH_WRITE);

	// Clear depthstencil
	CommandList->ClearDepthStencilView(BackDepth->GetDSV()->GetDescriptorHandle(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	// Set depth buffer
	CommandList->OMSetRenderTargets(0, nullptr, false, &BackDepth->GetDSV()->GetDescriptorHandle());


	// Draw all mesh
	for (const auto& Pair : BackDepthCommandMap)
	{
		const TGraphicsPSODescriptor& PSODescriptor = Pair.first;
		const TMeshCommandList& MeshCommandList = Pair.second;

		// Set PSO
		CommandList->SetPipelineState(GraphicsPSOManager->GetPSO(PSODescriptor));

		// Set RootSignature
		TShader* Shader = PSODescriptor.Shader;
		CommandList->SetGraphicsRootSignature(Shader->RootSignature.Get()); //should before binding

		for (const TMeshCommand& MeshCommand : MeshCommandList)
		{
			auto& TextureMap = TTextureRepository::Get().TextureMap;

			// Set paramters
			MeshCommand.ApplyShaderParamters(Shader);

			// Bind paramters
			Shader->BindParameters();

			const TMeshProxy& MeshProxy = MeshProxyMap.at(MeshCommand.MeshName);

			// Set vertex buffer
			D3D12RHI->SetVertexBuffer(MeshProxy.VertexBufferRef, 0, MeshProxy.VertexByteStride, MeshProxy.VertexBufferByteSize);

			// Set index buffer
			D3D12RHI->SetIndexBuffer(MeshProxy.IndexBufferRef, 0, MeshProxy.IndexFormat, MeshProxy.IndexBufferByteSize);

			D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
			CommandList->IASetPrimitiveTopology(PrimitiveType);

			// Draw 
			auto& SubMesh = MeshProxy.SubMeshs.at("Default");
			CommandList->DrawIndexedInstanced(SubMesh.IndexCount, 1, SubMesh.StartIndexLocation, SubMesh.BaseVertexLocation, 0);
		}
	}

	// Transit to generic read state
	D3D12RHI->TransitionResource(BackDepth->GetResource(), D3D12_RESOURCE_STATE_GENERIC_READ);
}

void TRender::UpdateSSAOPassCB()
{
	SSAOPassConstants SSAOPassCB;

	TCameraComponent* CameraComponent = World->GetCameraComponent();
	TMatrix Proj = CameraComponent->GetProj();
	TMatrix ProjTex = Proj * TextureTransform();

	SSAOPassCB.ProjTex = ProjTex.Transpose();

	SSAOPassCB.OcclusionRadius = 0.03f;
	SSAOPassCB.OcclusionFadeStart = 0.01f;
	SSAOPassCB.OcclusionFadeEnd = 0.03f;
	SSAOPassCB.SurfaceEpsilon = 0.001f;

	SSAOPassCBRef = D3D12RHI->CreateConstantBuffer(&SSAOPassCB, sizeof(SSAOPassCB));
}

void TRender::SSAOPass()
{
	UpdateSSAOPassCB();

	// Indicate a state transition on the resource usage.
	D3D12RHI->TransitionResource(SSAOBuffer->GetTexture()->GetResource(), D3D12_RESOURCE_STATE_RENDER_TARGET);

	// Set the viewport and scissor rect.
	D3D12_VIEWPORT ScreenViewport;
	D3D12_RECT ScissorRect;
	D3D12RHI->GetViewport()->GetD3DViewport(ScreenViewport, ScissorRect);
	CommandList->RSSetViewports(1, &ScreenViewport);
	CommandList->RSSetScissorRects(1, &ScissorRect);

	// Clear the back buffer and depth buffer.
	CommandList->ClearRenderTargetView(SSAOBuffer->GetRTV()->GetDescriptorHandle(), SSAOBuffer->GetClearColorPtr(), 0, nullptr);

	// Specify the buffers we are going to render to.
	CommandList->OMSetRenderTargets(1, &(SSAOBuffer->GetRTV()->GetDescriptorHandle()), true, nullptr);

	// Set PSO
	CommandList->SetPipelineState(GraphicsPSOManager->GetPSO(SSAOPSODescriptor));

	// Set RootSignature
	CommandList->SetGraphicsRootSignature(SSAOShader->RootSignature.Get()); //should before binding

	// Set paramters
	SSAOShader->SetParameter("cbSSAO", SSAOPassCBRef);
	SSAOShader->SetParameter("cbPass", BasePassCBRef);
	SSAOShader->SetParameter("NormalGbuffer", GBufferNormal->GetTexture()->SRVs[0].get());
	SSAOShader->SetParameter("DepthGbuffer", D3D12RHI->GetViewport()->GetDepthShaderResourceView());

	// Bind paramters
	SSAOShader->BindParameters();

	// Draw ScreenQuad
	{
		const TMeshProxy& MeshProxy = MeshProxyMap.at("ScreenQuadMesh");

		// Set vertex buffer
		D3D12RHI->SetVertexBuffer(MeshProxy.VertexBufferRef, 0, MeshProxy.VertexByteStride, MeshProxy.VertexBufferByteSize);

		// Set index buffer
		D3D12RHI->SetIndexBuffer(MeshProxy.IndexBufferRef, 0, MeshProxy.IndexFormat, MeshProxy.IndexBufferByteSize);

		D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		CommandList->IASetPrimitiveTopology(PrimitiveType);

		// Draw 
		auto& SubMesh = MeshProxy.SubMeshs.at("Default");
		CommandList->DrawIndexedInstanced(SubMesh.IndexCount, 1, SubMesh.StartIndexLocation, SubMesh.BaseVertexLocation, 0);
	}


	//--------------------------------------------HorzBlur--------------------------------------------

	// Set PSO
	CommandList->SetPipelineState(ComputePSOManager->GetPSO(HorzBlurPSODescriptor));

	// Set RootSignature
	CommandList->SetComputeRootSignature(HorzBlurShader->RootSignature.Get()); // should before binding

	// Copy SSAOBuffer to BlurMap0.
	D3D12RHI->TransitionResource(SSAOBuffer->GetTexture()->GetResource(), D3D12_RESOURCE_STATE_COPY_SOURCE);

	D3D12RHI->TransitionResource(BlurMap0->GetResource(), D3D12_RESOURCE_STATE_COPY_DEST);

	D3D12RHI->CopyResource(BlurMap0->GetResource(), SSAOBuffer->GetTexture()->GetResource());

	D3D12RHI->TransitionResource(BlurMap0->GetResource(), D3D12_RESOURCE_STATE_GENERIC_READ);

	// Set parameters
	HorzBlurShader->SetParameter("InputTexture", BlurMap0->SRVs[0].get());
	HorzBlurShader->SetParameter("OutputTexture", BlurMap1->UAVs[0].get());
	HorzBlurShader->SetParameter("cbBlurSettings", BlurSettingsCB);

	// Bind paramters
	HorzBlurShader->BindParameters();

	UINT NumGroupsX = (UINT)ceilf(WindowWidth / 256.0f);
	UINT NumGroupsY = WindowHeight;
	CommandList->Dispatch(NumGroupsX, NumGroupsY, 1);

	//--------------------------------------------VertBlur--------------------------------------------
	// Set PSO
	CommandList->SetPipelineState(ComputePSOManager->GetPSO(VertBlurPSODescriptor));

	// Set RootSignature
	CommandList->SetComputeRootSignature(VertBlurShader->RootSignature.Get()); // should before binding

	// Set parameters
	VertBlurShader->SetParameter("InputTexture", BlurMap1->SRVs[0].get());
	VertBlurShader->SetParameter("OutputTexture", BlurMap0->UAVs[0].get());
	VertBlurShader->SetParameter("cbBlurSettings", BlurSettingsCB);

	// Bind paramters
	VertBlurShader->BindParameters();

	NumGroupsX = WindowWidth;
	NumGroupsY = (UINT)ceilf(WindowHeight / 256.0f);
	CommandList->Dispatch(NumGroupsX, NumGroupsY, 1);

	// Copy blurred output to SSAOBuffer.
	D3D12RHI->TransitionResource(BlurMap0->GetResource(), D3D12_RESOURCE_STATE_COPY_SOURCE);

	D3D12RHI->TransitionResource(SSAOBuffer->GetTexture()->GetResource(), D3D12_RESOURCE_STATE_COPY_DEST);

	D3D12RHI->CopyResource(SSAOBuffer->GetTexture()->GetResource(), BlurMap0->GetResource());

	// Transit SSAOBuffer to generic read state
	D3D12RHI->TransitionResource(SSAOBuffer->GetTexture()->GetResource(), D3D12_RESOURCE_STATE_GENERIC_READ);
}


void TRender::GatherLightDebugPrimitives(std::vector<TLine>& OutLines)
{
	// Lights debug primitives
	{
		auto Lights = World->GetAllActorsOfClass<TLightActor>();
		for (UINT LightIdx = 0; LightIdx < Lights.size(); LightIdx++)
		{
			auto Light = Lights[LightIdx];

			if (!Light->IsDrawDebug())
			{
				continue;
			}

			if (Light->GetType() == ELightType::DirectionalLight)
			{
				auto DirectionalLight = dynamic_cast<TDirectionalLightActor*>(Light);
				assert(DirectionalLight);

				TVector3 Direction = DirectionalLight->GetLightDirection();
				TVector3 StartPos = DirectionalLight->GetActorLocation();
				float DebugLength = 3.0f;
				TVector3 EndPos = StartPos + Direction * DebugLength;

				TVector3 V1 = Direction.Cross(TVector3::Up);
				V1.Normalize();
				TVector3 Offset = V1 * 0.5f;

				OutLines.push_back(TLine(StartPos - Offset, EndPos - Offset, TColor::White));
				OutLines.push_back(TLine(StartPos, EndPos, TColor::White));
				OutLines.push_back(TLine(StartPos + Offset, EndPos + Offset, TColor::White));

			}
			else if (Light->GetType() == ELightType::PointLight)
			{
				auto PointLight = dynamic_cast<TPointLightActor*>(Light);
				assert(PointLight);

				TVector3 CenterPos = PointLight->GetActorLocation();
				float Radius = PointLight->GetAttenuationRange();
				TVector3 V1 = TVector3(1.0f, 0.0f, 0.0f);
				TVector3 V2 = TVector3(0.0f, 0.0f, 1.0f);
				TVector3 V3 = TVector3(0.0f, 1.0f, 0.0f);

				// Horizon circle
				TVector3 LastPoint = TVector3::Zero;
				bool bFirstPoint = true;
				for (float DeltaAngle = 0; DeltaAngle <= 360; DeltaAngle += 20)
				{
					float Radian = DeltaAngle * (TMath::Pi / 180.0f);
					TVector3 Point = CenterPos + (V1 * sin(Radian) + V2 * cos(Radian)) * Radius;

					if (bFirstPoint)
					{
						bFirstPoint = false;
					}
					else
					{
						OutLines.push_back(TLine(LastPoint, Point, TColor::Yellow));
					}

					LastPoint = Point;
				}

				// Vertical circle
				LastPoint = TVector3::Zero;
				bFirstPoint = true;
				for (float DeltaAngle = 0; DeltaAngle <= 360; DeltaAngle += 20)
				{
					float Radian = DeltaAngle * (TMath::Pi / 180.0f);
					TVector3 Point = CenterPos + (V1 * sin(Radian) + V3 * cos(Radian)) * Radius;

					if (bFirstPoint)
					{
						bFirstPoint = false;
					}
					else
					{
						OutLines.push_back(TLine(LastPoint, Point, TColor::Yellow));
					}

					LastPoint = Point;
				}

			}
			else if (Light->GetType() == ELightType::SpotLight)
			{
				auto SpotLight = dynamic_cast<TSpotLightActor*>(Light);
				assert(SpotLight);

				TVector3 TipPos = SpotLight->GetActorLocation();
				float Height = SpotLight->GetAttenuationRange();
				TVector3 Direction = SpotLight->GetLightDirection();
				TVector3 DirectionEnd = TipPos + Height * Direction;
				float BottomRadius = SpotLight->GetBottomRadius();

				// Direction line
				OutLines.push_back(TLine(TipPos, DirectionEnd, TColor::White));

				// Slant lines
				TVector3 V1 = Direction.Cross(TVector3::Up);
				V1.Normalize();
				TVector3 V2 = Direction.Cross(V1);
				V2.Normalize();

				for (float DeltaAngle = 0; DeltaAngle <= 360; DeltaAngle += 20)
				{
					float Radian = DeltaAngle * (TMath::Pi / 180.0f);
					TVector3 SlantPoint = DirectionEnd + (V1 * sin(Radian) + V2 * cos(Radian)) * BottomRadius;

					OutLines.push_back(TLine(TipPos, SlantPoint, TColor::Yellow));
				}			
			}
		}
	}
}

void TRender::GatherAllPrimitiveBatchs()
{
	PSOPrimitiveBatchMap.clear();

	// Points
	{
		// Get all points
		std::vector<TPrimitiveVertex> Vertices;
		const auto& Points = World->GetPoints();
		for (const auto& Point : Points)
		{
			Vertices.push_back(TPrimitiveVertex(Point.Point, Point.Color));
		}

		GatherPrimitiveBatchs(Vertices, D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT);
	}

	// Lines
	{
		// Get all lines
		std::vector<TLine> Lines;
		const auto& WorldLines = World->GetLines();
		Lines.insert(Lines.end(), WorldLines.begin(), WorldLines.end());
		
		std::vector<TLine> LightDebugLines;
		GatherLightDebugPrimitives(LightDebugLines);
		Lines.insert(Lines.end(), LightDebugLines.begin(), LightDebugLines.end());

		std::vector<TPrimitiveVertex> Vertices;
		for (const auto& Line : Lines)
		{
			Vertices.push_back(TPrimitiveVertex(Line.PointA, Line.Color));
			Vertices.push_back(TPrimitiveVertex(Line.PointB, Line.Color));
		}

		GatherPrimitiveBatchs(Vertices, D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE);
	}


	// Triangles
	{
		// Get all triangles
		std::vector<TPrimitiveVertex> Vertices;
		const auto& Triangles = World->GetTriangles();
		for (const auto& Triangle : Triangles) {
			Vertices.push_back(TPrimitiveVertex(Triangle.PointA, Triangle.Color));
			Vertices.push_back(TPrimitiveVertex(Triangle.PointB, Triangle.Color));
			Vertices.push_back(TPrimitiveVertex(Triangle.PointC, Triangle.Color));
		}

		GatherPrimitiveBatchs(Vertices, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
	}
}

void TRender::GatherPrimitiveBatchs(const std::vector<TPrimitiveVertex>& Vertices, D3D12_PRIMITIVE_TOPOLOGY_TYPE PrimitiveType)
{
	// Primitive PSO
	TGraphicsPSODescriptor PSODescriptor;
	PSODescriptor.InputLayoutName = std::string("PositionColorInputLayout");
	PSODescriptor.Shader = PrimitiveShader.get();
	PSODescriptor.PrimitiveTopologyType = PrimitiveType;

	// GBuffer PSO common settings
	PSODescriptor.RTVFormats[0] = GBufferBaseColor->GetFormat();
	PSODescriptor.RTVFormats[1] = GBufferNormal->GetFormat();
	PSODescriptor.RTVFormats[2] = GBufferWorldPos->GetFormat();
	PSODescriptor.RTVFormats[3] = GBufferORM->GetFormat();
	PSODescriptor.RTVFormats[4] = GBufferVelocity->GetFormat();
	PSODescriptor.RTVFormats[5] = GBufferEmissive->GetFormat();
	PSODescriptor.NumRenderTargets = GBufferCount;
	PSODescriptor.DepthStencilFormat = D3D12RHI->GetViewportInfo().DepthStencilFormat; 
	PSODescriptor._4xMsaaState = false; //can't use msaa in deferred rendering.

	// If don't find this PSO, create new PSO and PrimitiveBatch
	GraphicsPSOManager->TryCreatePSO(PSODescriptor);

	if (PSOPrimitiveBatchMap.find(PSODescriptor) == PSOPrimitiveBatchMap.end())
	{
		PSOPrimitiveBatchMap.emplace(std::make_pair(PSODescriptor, TPrimitiveBatch()));
		TPrimitiveBatch& PrimitiveBatch = PSOPrimitiveBatchMap[PSODescriptor];

		if (PrimitiveType == D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT)
		{
			PrimitiveBatch.PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
		}
		else if (PrimitiveType == D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE)
		{
			PrimitiveBatch.PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_LINELIST;
		}
		else if (PrimitiveType == D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE)
		{
			PrimitiveBatch.PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		}
		else
		{
			assert(0);
		}
	}

	// Update vertex buffer
	TPrimitiveBatch& PrimitiveBatch = PSOPrimitiveBatchMap[PSODescriptor];
	PrimitiveBatch.CurrentVertexNum = (int)Vertices.size();

	if (PrimitiveBatch.CurrentVertexNum > 0)
	{
		const UINT VbByteSize = (UINT)Vertices.size() * sizeof(TPrimitiveVertex);
		PrimitiveBatch.VertexBufferRef = D3D12RHI->CreateVertexBuffer(Vertices.data(), VbByteSize);
	}
	else
	{
		PrimitiveBatch.VertexBufferRef = nullptr;
	}
}

void TRender::PrimitivesPass()
{
	GatherAllPrimitiveBatchs();

	// Transit to render target state
	D3D12RHI->TransitionResource(GBufferBaseColor->GetTexture()->GetResource(), D3D12_RESOURCE_STATE_RENDER_TARGET);
	D3D12RHI->TransitionResource(GBufferNormal->GetTexture()->GetResource(), D3D12_RESOURCE_STATE_RENDER_TARGET);
	D3D12RHI->TransitionResource(GBufferWorldPos->GetTexture()->GetResource(), D3D12_RESOURCE_STATE_RENDER_TARGET);
	D3D12RHI->TransitionResource(GBufferORM->GetTexture()->GetResource(), D3D12_RESOURCE_STATE_RENDER_TARGET);
	D3D12RHI->TransitionResource(GBufferVelocity->GetTexture()->GetResource(), D3D12_RESOURCE_STATE_RENDER_TARGET);
	D3D12RHI->TransitionResource(GBufferEmissive->GetTexture()->GetResource(), D3D12_RESOURCE_STATE_RENDER_TARGET);

	// Specify the renderTargets we are going to render to.
	std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> RtvDescriptors;
	RtvDescriptors.push_back(GBufferBaseColor->GetRTV()->GetDescriptorHandle());
	RtvDescriptors.push_back(GBufferNormal->GetRTV()->GetDescriptorHandle());
	RtvDescriptors.push_back(GBufferWorldPos->GetRTV()->GetDescriptorHandle());
	RtvDescriptors.push_back(GBufferORM->GetRTV()->GetDescriptorHandle());
	RtvDescriptors.push_back(GBufferVelocity->GetRTV()->GetDescriptorHandle());
	RtvDescriptors.push_back(GBufferEmissive->GetRTV()->GetDescriptorHandle());

	auto DescriptorCache = D3D12RHI->GetDevice()->GetCommandContext()->GetDescriptorCache();
	CD3DX12_GPU_DESCRIPTOR_HANDLE GpuHandle;
	CD3DX12_CPU_DESCRIPTOR_HANDLE CpuHandle;
	DescriptorCache->AppendRtvDescriptors(RtvDescriptors, GpuHandle, CpuHandle);

	CommandList->OMSetRenderTargets(GBufferCount, &CpuHandle, true, &DepthStencilView());

	// Draw all PrimitiveBatchs
	for (const auto& Pair : PSOPrimitiveBatchMap)
	{
		const TGraphicsPSODescriptor& PSODescriptor = Pair.first;
		const TPrimitiveBatch& PrimitiveBatch = Pair.second;

		if (PrimitiveBatch.CurrentVertexNum > 0)
		{
			// Set PSO	
			CommandList->SetPipelineState(GraphicsPSOManager->GetPSO(PSODescriptor));

			// Set RootSignature
			TShader* Shader = PSODescriptor.Shader;
			CommandList->SetGraphicsRootSignature(Shader->RootSignature.Get()); //should before binding

			// Set paramters
			Shader->SetParameter("cbPass", BasePassCBRef);

			// Bind paramters
			Shader->BindParameters();

			// Set vertex buffer
			D3D12RHI->SetVertexBuffer(PrimitiveBatch.VertexBufferRef, 0, sizeof(TPrimitiveVertex), PrimitiveBatch.CurrentVertexNum * sizeof(TPrimitiveVertex));

			CommandList->IASetPrimitiveTopology(PrimitiveBatch.PrimitiveType);

			// Draw 
			CommandList->DrawInstanced(PrimitiveBatch.CurrentVertexNum, 1, 0, 0);
		}
	}

	// Transit to generic read state
	D3D12RHI->TransitionResource(GBufferBaseColor->GetTexture()->GetResource(), D3D12_RESOURCE_STATE_GENERIC_READ);
	D3D12RHI->TransitionResource(GBufferNormal->GetTexture()->GetResource(), D3D12_RESOURCE_STATE_GENERIC_READ);
	D3D12RHI->TransitionResource(GBufferWorldPos->GetTexture()->GetResource(), D3D12_RESOURCE_STATE_GENERIC_READ);
	D3D12RHI->TransitionResource(GBufferORM->GetTexture()->GetResource(), D3D12_RESOURCE_STATE_GENERIC_READ);
	D3D12RHI->TransitionResource(GBufferVelocity->GetTexture()->GetResource(), D3D12_RESOURCE_STATE_GENERIC_READ);
	D3D12RHI->TransitionResource(GBufferEmissive->GetTexture()->GetResource(), D3D12_RESOURCE_STATE_GENERIC_READ);
}

void TRender::TiledBaseLightCullingPass()
{
	// Set PSO
	CommandList->SetPipelineState(ComputePSOManager->GetPSO(TiledBaseLightCullingPSODescriptor));

	// Set RootSignature
	auto Shader = TiledBaseLightCullingPSODescriptor.Shader;
	CommandList->SetComputeRootSignature(Shader->RootSignature.Get()); // should before binding

	// Set parameters
	Shader->SetParameter("cbPass", BasePassCBRef);
	if (LightCount > 0)
	{
		Shader->SetParameter("Lights", LightShaderParametersBuffer->SRV.get());
	}
	else
	{
		Shader->SetParameter("Lights", StructuredBufferNullDescriptor.get());
	}
	Shader->SetParameter("LightCommonData", LightCommonDataBuffer);
	Shader->SetParameter("DepthTexture", D3D12RHI->GetViewport()->GetDepthShaderResourceView());
	Shader->SetParameter("TiledDepthDebugTexture", TiledDepthDebugTexture->UAVs[0].get());
	Shader->SetParameter("TileLightInfoList", TileLightInfoList->UAV.get());

	// Bind paramters
	Shader->BindParameters();

	UINT NumGroupsX = (UINT)ceilf(WindowWidth / float(TILE_BLOCK_SIZE));
	UINT NumGroupsY = (UINT)ceilf(WindowHeight / float(TILE_BLOCK_SIZE));
	CommandList->Dispatch(NumGroupsX, NumGroupsY, 1);
}

void TRender::UpdateDeferredLightingPassCB()
{
	DeferredLightingPassConstants DeferredLightPassCB;
	DeferredLightPassCB.EnableSSAO = RenderSettings.bEnableSSAO;

	DeferredLightPassCBRef = D3D12RHI->CreateConstantBuffer(&DeferredLightPassCB, sizeof(DeferredLightPassCB));
}

void TRender::DeferredLightingPass()
{
	UpdateDeferredLightingPassCB();

	// Indicate a state transition on the resource usage.
	D3D12RHI->TransitionResource(ColorTexture->GetResource(), D3D12_RESOURCE_STATE_RENDER_TARGET);

	// Set the viewport and scissor rect.
	D3D12_VIEWPORT ScreenViewport; 
	D3D12_RECT ScissorRect;
	D3D12RHI->GetViewport()->GetD3DViewport(ScreenViewport, ScissorRect);
	CommandList->RSSetViewports(1, &ScreenViewport);
	CommandList->RSSetScissorRects(1, &ScissorRect);

	// Clear the ColorTexture and depth buffer.
	float* ClearValue = ColorTexture->GetRTVClearValuePtr();
	D3D12_CPU_DESCRIPTOR_HANDLE RTVHandle = ColorTexture->RTVs[0]->GetDescriptorHandle();
	CommandList->ClearRenderTargetView(RTVHandle, ClearValue, 0, nullptr);

	// Specify the buffers we are going to render to.
	CommandList->OMSetRenderTargets(1, &RTVHandle, true, &DepthStencilView());

	// Set DeferredLighting PSO
	CommandList->SetPipelineState(GraphicsPSOManager->GetPSO(DeferredLightingPSODescriptor));

	// Set RootSignature
	TShader* Shader = DeferredLightingPSODescriptor.Shader;
	CommandList->SetGraphicsRootSignature(Shader->RootSignature.Get()); //should before binding

	auto& TextureMap = TTextureRepository::Get().TextureMap;

	//-------------------------------------Set paramters-------------------------------------------

	Shader->SetParameter("cbPass", BasePassCBRef);
	Shader->SetParameter("cbDeferredLighting", DeferredLightPassCBRef);
	Shader->SetParameter("LightCommonData", LightCommonDataBuffer);
	Shader->SetParameter("BaseColorGbuffer", GBufferBaseColor->GetTexture()->SRVs[0].get());
	Shader->SetParameter("NormalGbuffer", GBufferNormal->GetTexture()->SRVs[0].get());
	Shader->SetParameter("WorldPosGbuffer", GBufferWorldPos->GetTexture()->SRVs[0].get());
	Shader->SetParameter("OrmGbuffer", GBufferORM->GetTexture()->SRVs[0].get());
	Shader->SetParameter("EmissiveGbuffer", GBufferEmissive->GetTexture()->SRVs[0].get());
	Shader->SetParameter("SSAOBuffer", SSAOBuffer->GetTexture()->SRVs[0].get());
	Shader->SetParameter("LTC_LUT1", TextureMap["LtcMat_1"]->GetD3DTexture()->SRVs[0].get());
	Shader->SetParameter("LTC_LUT2", TextureMap["LtcMat_2"]->GetD3DTexture()->SRVs[0].get());

	if (LightCount > 0)
	{
		Shader->SetParameter("Lights", LightShaderParametersBuffer->SRV.get());
	}
	else
	{
		Shader->SetParameter("Lights", StructuredBufferNullDescriptor.get());
	}

	if (RenderSettings.bUseTBDR)
	{
		Shader->SetParameter("LightInfoList", TileLightInfoList->SRV.get());
	}

	if (bEnableIBLEnvLighting)
	{
		Shader->SetParameter("IBLIrradianceMap", IBLIrradianceMap->GetRTCube()->GetSRV());
		
		auto& BRDFIntegrationMapSRV = TextureMap["IBL_BRDF_LUT"]->GetD3DTexture()->SRVs[0];
		Shader->SetParameter("BrdfLUT", BRDFIntegrationMapSRV.get());

		std::vector<TD3D12ShaderResourceView*> IBLPrefilterEnvMapSRVs;
		for (UINT i = 0; i < IBLPrefilterMaxMipLevel; i++)
		{
			IBLPrefilterEnvMapSRVs.push_back(IBLPrefilterEnvMaps[i]->GetRTCube()->GetSRV());
		}
		Shader->SetParameter("IBLPrefilterEnvMaps", IBLPrefilterEnvMapSRVs);
	}
	else
	{
		Shader->SetParameter("IBLIrradianceMap", TextureCubeNullDescriptor.get());
		Shader->SetParameter("BrdfLUT", Texture2DNullDescriptor.get());

		std::vector<TD3D12ShaderResourceView*> NullSRVs;
		for (UINT i = 0; i < IBLPrefilterMaxMipLevel; i++)
		{
			NullSRVs.push_back(TextureCubeNullDescriptor.get());
		}
		Shader->SetParameter("IBLPrefilterEnvMaps", NullSRVs);
	}

	// Append null SRVs, make sure the size of SRVs is the same as shader binding count
	for (size_t i = ShadowMapSRVs.size(); i < MAX_SHADOW_MAP_2D_NUM; i++) 
	{
		ShadowMapSRVs.push_back(Texture2DNullDescriptor.get());
	}
	Shader->SetParameter("ShadowMaps", ShadowMapSRVs);

	// Append null SRVs, make sure the size of SRVs is the same as shader binding count
	for (size_t i = ShadowMapCubeSRVs.size(); i < MAX_SHADOW_MAP_CUBE_NUM; i++)
	{
		ShadowMapCubeSRVs.push_back(TextureCubeNullDescriptor.get());
	}
	Shader->SetParameter("ShadowMapCubes", ShadowMapCubeSRVs);

	// SDF texture
	auto& MeshMap = TMeshRepository::Get().MeshMap;

	std::vector<TD3D12ShaderResourceView*> SDFTextureSRVs;
	SDFTextureSRVs.resize(MAX_SDF_TEXTURE_COUNT);
	for (const auto& Pair : MeshSDFMap)
	{
		std::string MeshName = Pair.first;
		int SDFIndex = Pair.second;

		TMesh& Mesh = MeshMap[MeshName];
		auto SRV = Mesh.GetSDFTexture()->D3DTexture->SRVs[0].get();
		SDFTextureSRVs[SDFIndex] = SRV;
	}

	// Append null SRVs, make sure the size of SRVs is the same as shader binding count
	for (size_t i = MeshSDFMap.size(); i < MAX_SDF_TEXTURE_COUNT; i++)
	{
		SDFTextureSRVs[i] = Texture3DNullDescriptor.get();
	}

	Shader->SetParameter("SDFTextures", SDFTextureSRVs);

	if (MeshSDFBuffer)
	{
		Shader->SetParameter("MeshSDFDescriptors", MeshSDFBuffer->SRV.get());
	}
	else
	{
		Shader->SetParameter("MeshSDFDescriptors", StructuredBufferNullDescriptor.get());
	}

	if (ObjectSDFBuffer)
	{
		Shader->SetParameter("ObjectSDFDescriptors", ObjectSDFBuffer->SRV.get());
	}
	else
	{
		Shader->SetParameter("ObjectSDFDescriptors", StructuredBufferNullDescriptor.get());
	}

	Shader->SetParameter("cbSDF", SDFCBRef);

	//-------------------------------------------------------------------------------------------

	// Bind paramters
	Shader->BindParameters();

	// Draw ScreenQuad
	{
		const TMeshProxy& MeshProxy = MeshProxyMap.at("ScreenQuadMesh");

		// Set vertex buffer
		D3D12RHI->SetVertexBuffer(MeshProxy.VertexBufferRef, 0, MeshProxy.VertexByteStride, MeshProxy.VertexBufferByteSize);

		// Set index buffer
		D3D12RHI->SetIndexBuffer(MeshProxy.IndexBufferRef, 0, MeshProxy.IndexFormat, MeshProxy.IndexBufferByteSize);

		D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		CommandList->IASetPrimitiveTopology(PrimitiveType);

		// Draw 
		auto& SubMesh = MeshProxy.SubMeshs.at("Default");
		CommandList->DrawIndexedInstanced(SubMesh.IndexCount, 1, SubMesh.StartIndexLocation, SubMesh.BaseVertexLocation, 0);
	}

	// Transition to PRESENT state.
	D3D12RHI->TransitionResource(ColorTexture->GetResource(), D3D12_RESOURCE_STATE_PRESENT);
}

void TRender::UpdateSSRPassCB()
{

}

void TRender::SSRPass()
{
	UpdateSSRPassCB();

	// Copy ColorTexture to CacheColorTexture.
	D3D12RHI->TransitionResource(ColorTexture->GetResource(), D3D12_RESOURCE_STATE_COPY_SOURCE);

	D3D12RHI->TransitionResource(CacheColorTexture->GetResource(), D3D12_RESOURCE_STATE_COPY_DEST);

	D3D12RHI->CopyResource(CacheColorTexture->GetResource(), ColorTexture->GetResource());

	D3D12RHI->TransitionResource(ColorTexture->GetResource(), D3D12_RESOURCE_STATE_RENDER_TARGET);

	D3D12RHI->TransitionResource(CacheColorTexture->GetResource(), D3D12_RESOURCE_STATE_GENERIC_READ);

	// Set the viewport and scissor rect.
	D3D12_VIEWPORT ScreenViewport;
	D3D12_RECT ScissorRect;
	D3D12RHI->GetViewport()->GetD3DViewport(ScreenViewport, ScissorRect);
	CommandList->RSSetViewports(1, &ScreenViewport);
	CommandList->RSSetScissorRects(1, &ScissorRect);

	// Clear the back buffer.
	float* ClearValue = ColorTexture->GetRTVClearValuePtr();
	D3D12_CPU_DESCRIPTOR_HANDLE RTVHandle = ColorTexture->RTVs[0]->GetDescriptorHandle();
	CommandList->ClearRenderTargetView(RTVHandle, ClearValue, 0, nullptr);

	// Specify the buffers we are going to render to.
	CommandList->OMSetRenderTargets(1, &RTVHandle, true, nullptr);

	// Set PSO
	CommandList->SetPipelineState(GraphicsPSOManager->GetPSO(SSRPSODescriptor));

	// Set RootSignature
	CommandList->SetGraphicsRootSignature(SSRShader->RootSignature.Get()); //should before binding

	auto& TextureMap = TTextureRepository::Get().TextureMap;

	// Set paramters
	SSRShader->SetParameter("cbPass", BasePassCBRef);
	SSRShader->SetParameter("BaseColorGbuffer", GBufferBaseColor->GetTexture()->SRVs[0].get());
	SSRShader->SetParameter("NormalGbuffer", GBufferNormal->GetTexture()->SRVs[0].get());
	SSRShader->SetParameter("OrmGbuffer", GBufferORM->GetTexture()->SRVs[0].get());
	SSRShader->SetParameter("DepthGbuffer", D3D12RHI->GetViewport()->GetDepthShaderResourceView());
	SSRShader->SetParameter("ColorTexture", CacheColorTexture->SRVs[0].get());
	SSRShader->SetParameter("BlueNoiseTexture", TextureMap["BlueNoiseTex"]->GetD3DTexture()->SRVs[0].get());
	SSRShader->SetParameter("BackDepthTexture", BackDepth->GetSRV());

	// Bind paramters
	SSRShader->BindParameters();

	// Draw ScreenQuad
	{
		const TMeshProxy& MeshProxy = MeshProxyMap.at("ScreenQuadMesh");

		// Set vertex buffer
		D3D12RHI->SetVertexBuffer(MeshProxy.VertexBufferRef, 0, MeshProxy.VertexByteStride, MeshProxy.VertexBufferByteSize);

		// Set index buffer
		D3D12RHI->SetIndexBuffer(MeshProxy.IndexBufferRef, 0, MeshProxy.IndexFormat, MeshProxy.IndexBufferByteSize);

		D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		CommandList->IASetPrimitiveTopology(PrimitiveType);

		// Draw 
		auto& SubMesh = MeshProxy.SubMeshs.at("Default");
		CommandList->DrawIndexedInstanced(SubMesh.IndexCount, 1, SubMesh.StartIndexLocation, SubMesh.BaseVertexLocation, 0);
	}

	// Transition back-buffer to PRESENT state.
	D3D12RHI->TransitionResource(ColorTexture->GetResource(), D3D12_RESOURCE_STATE_PRESENT);
}

void TRender::TAAPass()
{
	if (FrameCount > 0)
	{
		// Copy ColorTexture to CacheColorTexture.
		D3D12RHI->TransitionResource(ColorTexture->GetResource(), D3D12_RESOURCE_STATE_COPY_SOURCE);

		D3D12RHI->TransitionResource(CacheColorTexture->GetResource(), D3D12_RESOURCE_STATE_COPY_DEST);

		D3D12RHI->CopyResource(CacheColorTexture->GetResource(), ColorTexture->GetResource());

		D3D12RHI->TransitionResource(ColorTexture->GetResource(), D3D12_RESOURCE_STATE_RENDER_TARGET);

		D3D12RHI->TransitionResource(CacheColorTexture->GetResource(), D3D12_RESOURCE_STATE_GENERIC_READ);

		// Set the viewport and scissor rect.
		D3D12_VIEWPORT ScreenViewport;
		D3D12_RECT ScissorRect;
		D3D12RHI->GetViewport()->GetD3DViewport(ScreenViewport, ScissorRect);
		CommandList->RSSetViewports(1, &ScreenViewport);
		CommandList->RSSetScissorRects(1, &ScissorRect);

		// Clear the back buffer.
		float* ClearValue = ColorTexture->GetRTVClearValuePtr();
		D3D12_CPU_DESCRIPTOR_HANDLE RTVHandle = ColorTexture->RTVs[0]->GetDescriptorHandle();
		CommandList->ClearRenderTargetView(RTVHandle, ClearValue, 0, nullptr);

		// Specify the buffers we are going to render to.
		CommandList->OMSetRenderTargets(1, &RTVHandle, true, nullptr);

		// Set PSO
		CommandList->SetPipelineState(GraphicsPSOManager->GetPSO(TAAPSODescriptor));

		// Set RootSignature
		CommandList->SetGraphicsRootSignature(TAAShader->RootSignature.Get()); //should before binding

		// Set paramters
		TAAShader->SetParameter("cbPass", BasePassCBRef);
		TAAShader->SetParameter("ColorTexture", CacheColorTexture->SRVs[0].get());
		TAAShader->SetParameter("PrevColorTexture", PrevColorTexture->SRVs[0].get());
		TAAShader->SetParameter("VelocityGBuffer", GBufferVelocity->GetTexture()->SRVs[0].get());
		TAAShader->SetParameter("DepthGbuffer", D3D12RHI->GetViewport()->GetDepthShaderResourceView());

		// Bind paramters
		TAAShader->BindParameters();

		// Draw ScreenQuad
		{
			const TMeshProxy& MeshProxy = MeshProxyMap.at("ScreenQuadMesh");

			// Set vertex buffer
			D3D12RHI->SetVertexBuffer(MeshProxy.VertexBufferRef, 0, MeshProxy.VertexByteStride, MeshProxy.VertexBufferByteSize);

			// Set index buffer
			D3D12RHI->SetIndexBuffer(MeshProxy.IndexBufferRef, 0, MeshProxy.IndexFormat, MeshProxy.IndexBufferByteSize);

			D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
			CommandList->IASetPrimitiveTopology(PrimitiveType);

			// Draw 
			auto& SubMesh = MeshProxy.SubMeshs.at("Default");
			CommandList->DrawIndexedInstanced(SubMesh.IndexCount, 1, SubMesh.StartIndexLocation, SubMesh.BaseVertexLocation, 0);
		}
	}

	// Copy back-buffer to PrevColorTexture.
	D3D12RHI->TransitionResource(ColorTexture->GetResource(), D3D12_RESOURCE_STATE_COPY_SOURCE);

	D3D12RHI->TransitionResource(PrevColorTexture->GetResource(), D3D12_RESOURCE_STATE_COPY_DEST);

	D3D12RHI->CopyResource(PrevColorTexture->GetResource(), ColorTexture->GetResource());

	D3D12RHI->TransitionResource(ColorTexture->GetResource(), D3D12_RESOURCE_STATE_RENDER_TARGET);

	D3D12RHI->TransitionResource(PrevColorTexture->GetResource(), D3D12_RESOURCE_STATE_GENERIC_READ);
}

void TRender::UpdateSpritePassCB()
{
	float xScale =  2.0f / WindowWidth;
	float yScale = 2.0f / WindowHeight;

	TMatrix ScreenToNDC
	{
		xScale, 0,    0,  0,
		0,   -yScale, 0,  0,
		0,     0,     1,  0,
		-1,    1,     0,  1
	};

	SpritePassConstants SPassCB;
	SPassCB.ScreenToNDC = ScreenToNDC.Transpose();

	SpritePassCBRef = D3D12RHI->CreateConstantBuffer(&SPassCB, sizeof(SPassCB));
}

void TRender::ConvertTextToSprites(std::vector<TSprite>& OutSprites)
{
	UIntPoint FontTextureSize(256, 232); //TODO

	//Calculate the scale factor 
	const float DesireCharacterSize = 10.0;
	const float CharacterDistanceFactor = 1.2f;

	const TSpriteFont::Glyph* TestGlyph = SpriteFont->FindGlyph('A');
	LONG TestCharacterWidth = TestGlyph->Subrect.right - TestGlyph->Subrect.left;
	float FontScale = DesireCharacterSize / float(TestCharacterWidth);
	float CharacterDistance = CharacterDistanceFactor * float(TestCharacterWidth);

	//Convert all text to sprites
	std::vector<TText> Texts;
	World->GetTexts(Texts);

	for (const TText& Text : Texts)
	{
		UIntPoint TextScreenPos = Text.ScreenPos;
		for (int Idx = 0; Idx < Text.Content.size(); Idx++)
		{
			char Character = Text.Content[Idx];
			if (Character == '\r' || Character == '\n')
			{
				continue;
			}

			const TSpriteFont::Glyph* Glyph = SpriteFont->FindGlyph(Character);
			if (Glyph) {
				RECT SourceRect = Glyph->Subrect;
				LONG CharacterWidth = Glyph->Subrect.right - Glyph->Subrect.left;
				LONG CharacterHeight = Glyph->Subrect.bottom - Glyph->Subrect.top;

				UIntPoint LeftTopOffset(uint32_t(CharacterDistance * Idx), uint32_t(Glyph->YOffset));
				UIntPoint LeftTopPos = TextScreenPos + LeftTopOffset * FontScale;

				UIntPoint RightBottomOffset(uint32_t(CharacterDistance * Idx + CharacterWidth), uint32_t(Glyph->YOffset + CharacterHeight));
				UIntPoint RightBottomPos = TextScreenPos + RightBottomOffset * FontScale;

				RECT Dest;
				Dest.left = LeftTopPos.x;
				Dest.right = RightBottomPos.x;
				Dest.top = LeftTopPos.y;
				Dest.bottom = RightBottomPos.y;
				OutSprites.emplace_back(TSprite(SpriteFont->GetFontTexture()->Name, FontTextureSize, SourceRect, Dest));
			}
		}
	}
}

void TRender::GatherAllSpriteBatchs()
{
	PSOSpriteBatchMap.clear();

	TGraphicsPSODescriptor PSODescriptor;
	PSODescriptor.InputLayoutName = std::string("PositionTexcoordInputLayout");
	PSODescriptor.Shader = SpriteShader.get();
	PSODescriptor.RasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	PSODescriptor.BlendDesc =
	{
		FALSE, // AlphaToCoverageEnable
		FALSE, // IndependentBlendEnable
		{ {
			TRUE, // BlendEnable
			FALSE, // LogicOpEnable
			D3D12_BLEND_ONE, // SrcBlend
			D3D12_BLEND_INV_SRC_ALPHA, // DestBlend
			D3D12_BLEND_OP_ADD, // BlendOp
			D3D12_BLEND_ONE, // SrcBlendAlpha
			D3D12_BLEND_INV_SRC_ALPHA, // DestBlendAlpha
			D3D12_BLEND_OP_ADD, // BlendOpAlpha
			D3D12_LOGIC_OP_NOOP,
			D3D12_COLOR_WRITE_ENABLE_ALL
		} }
	};
	PSODescriptor.DepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	PSODescriptor.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	PSODescriptor.RTVFormats[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;

	// If don't find this PSO, create new PSO and SpriteBatch
	GraphicsPSOManager->TryCreatePSO(PSODescriptor);

	if (PSOSpriteBatchMap.find(PSODescriptor) == PSOSpriteBatchMap.end())
	{
		PSOSpriteBatchMap.emplace(std::make_pair(PSODescriptor, TSpriteBatch()));
		TSpriteBatch& SpriteBatch = PSOSpriteBatchMap[PSODescriptor];
	}

	auto& TextureMap = TTextureRepository::Get().TextureMap;
	TSpriteBatch& SpriteBatch = PSOSpriteBatchMap[PSODescriptor];

	// Get spriteItems from world sprites
	const std::vector<TSprite>& WorldSprites = World->GetSprites();
	for (const TSprite& Sprite : WorldSprites)
	{
		TSpriteItem SpriteItem;
		TTexture* Texture = TextureMap[Sprite.TextureName].get();
		SpriteItem.SpriteSRV = Texture->GetD3DTexture()->SRVs[0].get();
		SpriteItem.TextureSize = Sprite.TextureSize;
		SpriteItem.SourceRect = Sprite.SourceRect;
		SpriteItem.DestRect = Sprite.DestRect;

		SpriteBatch.SpriteItems.push_back(SpriteItem);
	}

	if (RenderSettings.bDrawDebugText)
	{
		// Get spriteItems from text sprites
		std::vector<TSprite> TextSprites;
		ConvertTextToSprites(TextSprites);
		for (const TSprite& Sprite : TextSprites)
		{
			TSpriteItem SpriteItem;
			TTexture* Texture = SpriteFont->GetFontTexture().get();
			SpriteItem.SpriteSRV = Texture->GetD3DTexture()->SRVs[0].get();
			SpriteItem.TextureSize = Sprite.TextureSize;
			SpriteItem.SourceRect = Sprite.SourceRect;
			SpriteItem.DestRect = Sprite.DestRect;

			SpriteBatch.SpriteItems.push_back(SpriteItem);
		}
	}

	// Debug shadow map
	bool bDebugShadowMap = false;
	if (bDebugShadowMap)
	{
		TSpriteItem DebugShadowSpriteItem;
		TShadowMap2D* ShadowMap = nullptr; //TODO
		DebugShadowSpriteItem.SpriteSRV = ShadowMap->GetRT()->GetSRV();
		UINT Width = ShadowMap->GetWidth() / 7;
		UINT Height = ShadowMap->GetHeight() / 7;
		DebugShadowSpriteItem.TextureSize = UIntPoint(Width, Height);
		DebugShadowSpriteItem.SourceRect = { 0, 0, (LONG)Width, (LONG)Height };
		DebugShadowSpriteItem.DestRect = { 0, LONG(720 - Height), (LONG)Width, 720 };

		SpriteBatch.SpriteItems.push_back(DebugShadowSpriteItem);
	}

	// Record vertices and Indices 
	std::vector<TSpriteVertex> Vertices;
	std::vector<std::uint16_t> Indices;

	for (const TSpriteItem& SpriteItem : SpriteBatch.SpriteItems)
	{
		//--------------------------------
		// 0-------1
		// |    /  |
		// |   /   |
		// | /     |
		// 2-------3
		//-------------------------------

		// Add vertices for this sprite
		TVector3 Vec0(float(SpriteItem.DestRect.left), float(SpriteItem.DestRect.top), 0.0f);
		TVector3 Vec1(float(SpriteItem.DestRect.right), float(SpriteItem.DestRect.top), 0.0f);
		TVector3 Vec2(float(SpriteItem.DestRect.left), float(SpriteItem.DestRect.bottom), 0.0f);
		TVector3 Vec3(float(SpriteItem.DestRect.right), float(SpriteItem.DestRect.bottom), 0.0f);

		TVector2 TextureSize(float(SpriteItem.TextureSize.x), float(SpriteItem.TextureSize.y));
		TVector2 UV0 = TVector2(float(SpriteItem.SourceRect.left), float(SpriteItem.SourceRect.top)) / TextureSize;   //UV(0, 0)
		TVector2 UV1 = TVector2(float(SpriteItem.SourceRect.right), float(SpriteItem.SourceRect.top)) / TextureSize;   //UV(1, 0)   
		TVector2 UV2 = TVector2(float(SpriteItem.SourceRect.left), float(SpriteItem.SourceRect.bottom)) / TextureSize;   //UV(0, 1)
		TVector2 UV3 = TVector2(float(SpriteItem.SourceRect.right), float(SpriteItem.SourceRect.bottom)) / TextureSize;   //UV(1, 1)

		Vertices.emplace_back(Vec0, UV0);
		Vertices.emplace_back(Vec1, UV1);
		Vertices.emplace_back(Vec2, UV2);
		Vertices.emplace_back(Vec3, UV3);


		// Add indices for this sprite(we will draw sprite one by one)
		Indices.emplace_back(0);
		Indices.emplace_back(1);
		Indices.emplace_back(2);

		Indices.emplace_back(1);
		Indices.emplace_back(3);
		Indices.emplace_back(2);
	}


	// Update vertex and index buffer
	if (SpriteBatch.SpriteItems.size() > 0)
	{
		const UINT VbByteSize = (UINT)Vertices.size() * sizeof(TSpriteVertex);
		SpriteBatch.VertexBufferRef = D3D12RHI->CreateVertexBuffer(Vertices.data(), VbByteSize);

		const UINT IbByteSize = (UINT)Indices.size() * sizeof(TMesh::uint16);
		SpriteBatch.IndexBufferRef = D3D12RHI->CreateIndexBuffer(Indices.data(), IbByteSize);
	}
	else
	{
		SpriteBatch.VertexBufferRef = nullptr;
		SpriteBatch.IndexBufferRef = nullptr;
	}
}

void TRender::SpritePass()
{
	UpdateSpritePassCB();

	GatherAllSpriteBatchs();

	// Indicate a state transition on the resource usage.
	D3D12RHI->TransitionResource(ColorTexture->GetResource(), D3D12_RESOURCE_STATE_RENDER_TARGET);

	// Specify the buffers we are going to render to.
	D3D12_CPU_DESCRIPTOR_HANDLE RTVHandle = ColorTexture->RTVs[0]->GetDescriptorHandle();
	CommandList->OMSetRenderTargets(1, &RTVHandle, true, &DepthStencilView());

	// Draw all SpriteBatchs
	for (const auto& Pair : PSOSpriteBatchMap)
	{
		const TGraphicsPSODescriptor& PSODescriptor = Pair.first;
		const TSpriteBatch& SpriteBatch = Pair.second;

		if (SpriteBatch.SpriteItems.size() > 0)
		{
			// Set PSO	
			CommandList->SetPipelineState(GraphicsPSOManager->GetPSO(PSODescriptor));

			// Set RootSignature
			TShader* Shader = PSODescriptor.Shader;
			CommandList->SetGraphicsRootSignature(Shader->RootSignature.Get()); //should before binding

			auto& TextureMap = TTextureRepository::Get().TextureMap;

			int SpriteIdx = 0;
			for (int SpriteIdx = 0; SpriteIdx < SpriteBatch.SpriteItems.size(); SpriteIdx++)
			{
				const TSpriteItem& SpriteItem = SpriteBatch.SpriteItems[SpriteIdx];

				Shader->SetParameter("cbSpritePass", SpritePassCBRef);
				Shader->SetParameter("spriteTexture", SpriteItem.SpriteSRV);

				// Bind paramters
				Shader->BindParameters();

				// Set vertex buffer
				UINT VertexPerSprite = 4; 
				UINT VbOffset = VertexPerSprite * sizeof(TSpriteVertex) * SpriteIdx;
				D3D12RHI->SetVertexBuffer(SpriteBatch.VertexBufferRef, VbOffset, sizeof(TSpriteVertex), VertexPerSprite * sizeof(TSpriteVertex));

				// Set index buffer
				UINT IndexPerSprite = 6; 
				UINT IbOffset = IndexPerSprite * sizeof(std::uint16_t) * SpriteIdx;
				D3D12RHI->SetIndexBuffer(SpriteBatch.IndexBufferRef, IbOffset, DXGI_FORMAT_R16_UINT, IndexPerSprite * sizeof(std::uint16_t));

				D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
				CommandList->IASetPrimitiveTopology(PrimitiveType);

				// Draw 
				CommandList->DrawIndexedInstanced(IndexPerSprite, 1, 0, 0, 0);
			}
		}
	}

	// Transition to PRESENT state.
	D3D12RHI->TransitionResource(ColorTexture->GetResource(), D3D12_RESOURCE_STATE_PRESENT);
}

void TRender::UpdateSDFData()
{
	auto& MeshMap = TMeshRepository::Get().MeshMap;

	std::vector<TMeshSDFDescriptor> MeshSDFDescriptors;
	std::vector<TObjectSDFDescriptor> ObjectSDFDescriptors;

	MeshSDFMap.clear();

	for (const TMeshBatch& MeshBatch : MeshBatchs) // Add new ObjectSDFDescriptor
	{
		if (!MeshBatch.bUseSDF)
		{
			continue;
		}

		TMesh& Mesh = MeshMap[MeshBatch.MeshName];
		std::string& MeshName = Mesh.MeshName;

		if (MeshSDFMap.find(MeshName) == MeshSDFMap.end()) // Add new MeshSDFDescriptor
		{
			MeshSDFMap[MeshName] = (int)MeshSDFDescriptors.size();
			MeshSDFDescriptors.push_back(Mesh.SDFDescriptor);			
		}

		auto MeshComponent = MeshBatch.MeshComponent;
		TMatrix World = MeshComponent->GetWorldTransform().GetTransformMatrix();
		TMatrix CenterOffset = TMatrix::CreateTranslation(Mesh.BoundingBox.GetCenter());
		World = CenterOffset * World;

		TObjectSDFDescriptor ObjectSDFDescriptor;
		ObjectSDFDescriptor.ObjWorld = World.Transpose();
		ObjectSDFDescriptor.ObjInvWorld = World.Invert().Transpose();
		ObjectSDFDescriptor.ObjInvWorld_IT = World;
		ObjectSDFDescriptor.SDFIndex = MeshSDFMap[MeshName]; 

		ObjectSDFDescriptors.push_back(ObjectSDFDescriptor);
	}

	if (MeshSDFDescriptors.size() > 0)
	{
		MeshSDFBuffer = D3D12RHI->CreateStructuredBuffer(MeshSDFDescriptors.data(), (uint32_t)(sizeof(TMeshSDFDescriptor)),
			(uint32_t)(MeshSDFDescriptors.size()));
	}
	else
	{
		MeshSDFBuffer = nullptr;
	}

	if (ObjectSDFDescriptors.size() > 0)
	{
		ObjectSDFBuffer = D3D12RHI->CreateStructuredBuffer(ObjectSDFDescriptors.data(), (uint32_t)(sizeof(TObjectSDFDescriptor)),
			(uint32_t)(ObjectSDFDescriptors.size()));
	}
	else
	{
		ObjectSDFBuffer = nullptr;
	}

	SDFConstants Constants;
	Constants.ObjectCount =(UINT)ObjectSDFDescriptors.size();
	SDFCBRef = D3D12RHI->CreateConstantBuffer(&Constants, sizeof(Constants));
}

void TRender::DebugSDFScenePass()
{
	// Indicate a state transition on the resource usage.
	D3D12RHI->TransitionResource(ColorTexture->GetResource(), D3D12_RESOURCE_STATE_RENDER_TARGET);

	// Clear the ColorTexture and depth buffer.
	float* ClearValue = ColorTexture->GetRTVClearValuePtr();
	D3D12_CPU_DESCRIPTOR_HANDLE RTVHandle = ColorTexture->RTVs[0]->GetDescriptorHandle();
	CommandList->ClearRenderTargetView(RTVHandle, ClearValue, 0, nullptr);
	CommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	// Set PSO	
	CommandList->SetPipelineState(GraphicsPSOManager->GetPSO(DebugSDFScenePSODescriptor));

	// Set RootSignature
	CommandList->SetGraphicsRootSignature(DebugSDFSceneShader->RootSignature.Get()); //should before binding

	auto& MeshMap = TMeshRepository::Get().MeshMap;

	std::vector<TD3D12ShaderResourceView*> SDFTextureSRVs;
	SDFTextureSRVs.resize(MAX_SDF_TEXTURE_COUNT);
	for (const auto& Pair : MeshSDFMap)
	{
		std::string MeshName = Pair.first;
		int SDFIndex = Pair.second;

		TMesh& Mesh = MeshMap[MeshName];
		auto SRV = Mesh.GetSDFTexture()->D3DTexture->SRVs[0].get();
		SDFTextureSRVs[SDFIndex] = SRV;
	}

	// Append null SRVs, make sure the size of SRVs is the same as shader binding count
	auto& TextureMap = TTextureRepository::Get().TextureMap;
	for (size_t i = MeshSDFMap.size(); i < MAX_SDF_TEXTURE_COUNT; i++)
	{
		SDFTextureSRVs[i] = Texture3DNullDescriptor.get();
	}

	DebugSDFSceneShader->SetParameter("SDFTextures", SDFTextureSRVs);

	if (MeshSDFBuffer)
	{
		DebugSDFSceneShader->SetParameter("MeshSDFDescriptors", MeshSDFBuffer->SRV.get());
	}
	else
	{
		DebugSDFSceneShader->SetParameter("MeshSDFDescriptors", StructuredBufferNullDescriptor.get());
	}

	if (ObjectSDFBuffer)
	{
		DebugSDFSceneShader->SetParameter("ObjectSDFDescriptors", ObjectSDFBuffer->SRV.get());
	}
	else
	{
		DebugSDFSceneShader->SetParameter("ObjectSDFDescriptors", StructuredBufferNullDescriptor.get());
	}

	DebugSDFSceneShader->SetParameter("cbPass", BasePassCBRef);

	DebugSDFSceneShader->SetParameter("cbSDF", SDFCBRef);

	DebugSDFSceneShader->SetParameter("DepthGbuffer", D3D12RHI->GetViewport()->GetDepthShaderResourceView());

	// Bind paramters
	DebugSDFSceneShader->BindParameters();

	// Draw ScreenQuad
	{
		const TMeshProxy& MeshProxy = MeshProxyMap.at("ScreenQuadMesh");

		// Set vertex buffer
		D3D12RHI->SetVertexBuffer(MeshProxy.VertexBufferRef, 0, MeshProxy.VertexByteStride, MeshProxy.VertexBufferByteSize);

		// Set index buffer
		D3D12RHI->SetIndexBuffer(MeshProxy.IndexBufferRef, 0, MeshProxy.IndexFormat, MeshProxy.IndexBufferByteSize);

		D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		CommandList->IASetPrimitiveTopology(PrimitiveType);

		// Draw 
		auto& SubMesh = MeshProxy.SubMeshs.at("Default");
		CommandList->DrawIndexedInstanced(SubMesh.IndexCount, 1, SubMesh.StartIndexLocation, SubMesh.BaseVertexLocation, 0);
	}

	// Transition to PRESENT state.
	D3D12RHI->TransitionResource(ColorTexture->GetResource(), D3D12_RESOURCE_STATE_PRESENT);
}

void TRender::PostProcessPass()
{
	D3D12RHI->TransitionResource(ColorTexture->GetResource(), D3D12_RESOURCE_STATE_GENERIC_READ);

	D3D12RHI->TransitionResource(CurrentBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET);

	// Set the viewport and scissor rect.
	D3D12_VIEWPORT ScreenViewport;
	D3D12_RECT ScissorRect;
	D3D12RHI->GetViewport()->GetD3DViewport(ScreenViewport, ScissorRect);
	CommandList->RSSetViewports(1, &ScreenViewport);
	CommandList->RSSetScissorRects(1, &ScissorRect);

	// Clear the back buffer.
	float* ClearColor = CurrentBackBufferClearColor();
	CommandList->ClearRenderTargetView(CurrentBackBufferView(), ClearColor, 0, nullptr);

	// Specify the buffers we are going to render to.
	CommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, nullptr);

	// Set PSO
	CommandList->SetPipelineState(GraphicsPSOManager->GetPSO(PostProcessPSODescriptor));

	// Set RootSignature
	CommandList->SetGraphicsRootSignature(PostProcessShader->RootSignature.Get()); //should before binding

	// Set paramters
	//PostProcessShader->SetParameter("cbPass", BasePassCBRef);
	PostProcessShader->SetParameter("ColorTexture", ColorTexture->SRVs[0].get());

	// Bind paramters
	PostProcessShader->BindParameters();

	// Draw ScreenQuad
	{
		const TMeshProxy& MeshProxy = MeshProxyMap.at("ScreenQuadMesh");

		// Set vertex buffer
		D3D12RHI->SetVertexBuffer(MeshProxy.VertexBufferRef, 0, MeshProxy.VertexByteStride, MeshProxy.VertexBufferByteSize);

		// Set index buffer
		D3D12RHI->SetIndexBuffer(MeshProxy.IndexBufferRef, 0, MeshProxy.IndexFormat, MeshProxy.IndexBufferByteSize);

		D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		CommandList->IASetPrimitiveTopology(PrimitiveType);

		// Draw 
		auto& SubMesh = MeshProxy.SubMeshs.at("Default");
		CommandList->DrawIndexedInstanced(SubMesh.IndexCount, 1, SubMesh.StartIndexLocation, SubMesh.BaseVertexLocation, 0);
	}

	// Transition back-buffer to PRESENT state.
	D3D12RHI->TransitionResource(CurrentBackBuffer(), D3D12_RESOURCE_STATE_PRESENT);
}

void TRender::OnDestroy()
{
	D3D12RHI->FlushCommandQueue();
}

const TRenderSettings& TRender::GetRenderSettings()
{
	return RenderSettings;
}

void TRender::ToggleTAA()
{
	RenderSettings.bEnableTAA = !RenderSettings.bEnableTAA;
}

void TRender::ToggleSSR()
{
	RenderSettings.bEnableSSR = !RenderSettings.bEnableSSR;
}

void TRender::ToggleSSAO()
{
	RenderSettings.bEnableSSAO = !RenderSettings.bEnableSSAO;
}

void TRender::ToggleDebugSDF()
{
	RenderSettings.bDebugSDFScene = !RenderSettings.bDebugSDFScene;
}

