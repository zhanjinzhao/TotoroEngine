#pragma once

#include <unordered_map>
#include <memory>
#include <wrl/client.h>
#include "Shader/Shader.h"
#include "Actor/Actor.h"
#include "Actor/Light/LightActor.h"
#include "World/World.h"
#include "Component/MeshComponent.h"
#include "Component/CameraComponent.h"
#include "Texture/Texture.h"
#include "Material/Material.h"
#include "Engine/GameTimer.h"
#include "RenderProxy.h"
#include "InputLayout.h"
#include "PSO.h"
#include "MeshBatch.h"
#include "PrimitiveBatch.h"
#include "SpriteBatch.h"
#include "SpriteFont.h"
#include "RenderTarget.h"
#include "SceneCaptureCube.h"
#include "ShadowMap.h"
#include "D3D12/D3D12RHI.h"

// Link necessary d3d12 libraries.
#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

#define TAA_SAMPLE_COUNT 8

enum class ERenderPass
{
	SHADOWSPASS,
	BASEPASS,
	PRIMITIVEPASS,
	SPRITEPASS,
	IRRADIANCE,
	DEFERREDLIGHTING,
};

struct ComputeShaderTestData
{
	DirectX::XMFLOAT3 v1;
	DirectX::XMFLOAT2 v2;
};

struct TRenderSettings
{
	bool bUseTBDR = false;

	bool bEnableTAA = true;

	bool bEnableSSR = false;

	bool bEnableSSAO = true;

	EShadowMapImpl ShadowMapImpl = EShadowMapImpl::PCSS;

	bool bDebugSDFScene = false;

	bool bDrawDebugText = false;
};

class TRender
{
public:
	TRender();

	~TRender();

	bool IsInitialize();

	bool Initialize(int WindowWidth, int WindowHeight, TD3D12RHI* InD3D12RHI, TWorld* InWorld, const TRenderSettings& Settings);

	void OnResize(int NewWidth, int NewHeight);

	void Draw(const GameTimer& gt);

	void EndFrame();

	void OnDestroy();

private:
	float AspectRatio() const;

	TD3D12Resource* CurrentBackBuffer() const;

	D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView() const;

	float* CurrentBackBufferClearColor() const;

	D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView() const;

private:
	void CreateRenderResource();

	void CreateNullDescriptors();

	void CreateTextures();

	void CreateSceneCaptureCube();

	void CreateGBuffers();

	void CreateSSAOBuffer();

	void CreateBackDepth();

	void CreateColorTextures();

	void CreateMeshProxys();

	void CreateMeshSDFTexture(TMesh& Mesh);

	void BulidMeshSDF(TMesh& Mesh, std::vector<uint8_t>& OutMeshSDF);

	void CreateInputLayouts();

	void CreateGlobalShaders();

	void CreateGlobalPSO();

	void CreateComputePSO();

	void CreateComputeShaderResource();

private:
	void SetDescriptorHeaps();

	void GetSkyInfo();

	void UpdateIBLEnviromentPassCB();

	void CreateIBLEnviromentMap();

	void UpdateIBLIrradiancePassCB();

	void CreateIBLIrradianceMap();

	void UpdateIBLPrefilterEnvCB();

	void CreateIBLPrefilterEnvMap();

	void GatherAllMeshBatchs();

	TMatrix TextureTransform();

	void UpdateLightData();

	void UpdateShadowPassCB(const TSceneView& SceneView, UINT ShadowWidth, UINT ShadowHeight);

	void GetShadowPassMeshCommandMap(EShadowMapType Type);

	void ShadowPass();

	void GenerateSingleShadowMap(TShadowMap2D* ShadowMap);

	void GenerateOmniShadowMap(TShadowMapCube* ShadowMap);

	void GenerateVSMShadow(TShadowMap2D* ShadowMap, TD3D12TextureRef VSMTexture, TD3D12TextureRef VSMBlurTexture);

	void UpdateBasePassCB();

	void GetBasePassMeshCommandMap();

	void BasePass();

	void GetBackDepthPassMeshCommandMap();

	void BackDepthPass();

	void UpdateSSAOPassCB();

	void SSAOPass();

	void GatherLightDebugPrimitives(std::vector<TLine>& OutLines);

	void GatherAllPrimitiveBatchs();

	void GatherPrimitiveBatchs(const std::vector<TPrimitiveVertex>& Vertices, D3D12_PRIMITIVE_TOPOLOGY_TYPE PrimitiveType);

	void PrimitivesPass();

	void TiledBaseLightCullingPass();

	void UpdateDeferredLightingPassCB();

	void DeferredLightingPass();

	void UpdateSSRPassCB();

	void SSRPass();

	void TAAPass();

	void UpdateSpritePassCB();

	void ConvertTextToSprites(std::vector<TSprite>& OutSprites);

	void GatherAllSpriteBatchs();

	void SpritePass();

	void UpdateSDFData();

	void DebugSDFScenePass();

	void PostProcessPass();

public:
	const TRenderSettings& GetRenderSettings();

	void ToggleTAA();

	void ToggleSSR();

	void ToggleSSAO();

	void ToggleDebugSDF();

private:
	bool bInitialize;

	int WindowWidth;
	int WindowHeight;

	TWorld* World;

	ID3D12Device* D3DDevice;
	ID3D12GraphicsCommandList* CommandList;

	std::unique_ptr<TD3D12ShaderResourceView> Texture2DNullDescriptor = nullptr;
	std::unique_ptr<TD3D12ShaderResourceView> Texture3DNullDescriptor = nullptr;
	std::unique_ptr<TD3D12ShaderResourceView> TextureCubeNullDescriptor = nullptr;
	std::unique_ptr<TD3D12ShaderResourceView> StructuredBufferNullDescriptor = nullptr;

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> FontHeap;

	const int GBufferCount = 6;
	std::unique_ptr<TRenderTarget2D> GBufferBaseColor;
	std::unique_ptr<TRenderTarget2D> GBufferNormal;
	std::unique_ptr<TRenderTarget2D> GBufferWorldPos;
	std::unique_ptr<TRenderTarget2D> GBufferORM;  //OcclusionRoughnessMetallic
	std::unique_ptr<TRenderTarget2D> GBufferVelocity;
	std::unique_ptr<TRenderTarget2D> GBufferEmissive;

	std::unique_ptr<TRenderTarget2D> SSAOBuffer;

	TD3D12TextureRef ColorTexture = nullptr;

	TD3D12TextureRef CacheColorTexture = nullptr;

	TD3D12TextureRef PrevColorTexture = nullptr;

	std::unique_ptr<TRenderTarget2D> BackDepth = nullptr;

	UINT FrameCount = 0;

private:
	// Mesh
	std::unordered_map<std::string, TMeshProxy> MeshProxyMap;

	TD3D12StructuredBufferRef MeshSDFBuffer = nullptr;

	TD3D12StructuredBufferRef ObjectSDFBuffer = nullptr;

	TD3D12ConstantBufferRef SDFCBRef = nullptr;

	std::unordered_map<std::string/*MeshName*/, int/*SdfIndex*/> MeshSDFMap;

	// InputLayout
	TInputLayoutManager InputLayoutManager;

	// PassCB
	TD3D12ConstantBufferRef ShadowPassCBRef = nullptr;

	TD3D12ConstantBufferRef BasePassCBRef = nullptr;

	TD3D12ConstantBufferRef SSAOPassCBRef = nullptr;

	TD3D12ConstantBufferRef SpritePassCBRef = nullptr;

	TD3D12ConstantBufferRef IBLEnviromentPassCBRef[6];

	TD3D12ConstantBufferRef IBLIrradiancePassCBRef[6];

	const static UINT IBLPrefilterMaxMipLevel = 5;

	TD3D12ConstantBufferRef IBLPrefilterEnvPassCBRef[IBLPrefilterMaxMipLevel * 6];

	TD3D12ConstantBufferRef DeferredLightPassCBRef;

	// Computshader resource
	TD3D12TextureRef BlurMap0 = nullptr;

	TD3D12TextureRef BlurMap1 = nullptr;

	TD3D12ConstantBufferRef BlurSettingsCB;

	TD3D12ConstantBufferRef VSMBlurSettingsCB;

	const int TILE_BLOCK_SIZE = 16;

	TD3D12TextureRef TiledDepthDebugTexture = nullptr;

	TD3D12RWStructuredBufferRef TileLightInfoList = nullptr;

	// Global shader
	std::unique_ptr<TShader> SingleShadowMapShader = nullptr;

	std::unique_ptr<TShader> OmniShadowMapShader = nullptr;

	std::unique_ptr<TShader> BackDepthShader = nullptr;

	std::unique_ptr<TShader> IBLEnvironmentShader = nullptr;

	std::unique_ptr<TShader> IBLIrradianceShader = nullptr;

	std::unique_ptr<TShader> IBLPrefilterEnvShader = nullptr;

	std::unique_ptr<TShader> DeferredLightingShader = nullptr;

	std::unique_ptr<TShader> PrimitiveShader = nullptr;

	std::unique_ptr<TShader> SpriteShader = nullptr;

	std::unique_ptr<TShader> DebugSDFSceneShader = nullptr;

	std::unique_ptr<TShader> SSAOShader = nullptr;

	std::unique_ptr<TShader> SSRShader = nullptr;

	std::unique_ptr<TShader> PostProcessShader = nullptr;

	std::unique_ptr<TShader> TAAShader = nullptr;

	std::unique_ptr<TShader> HorzBlurShader = nullptr;

	std::unique_ptr<TShader> VertBlurShader = nullptr;

	std::unique_ptr<TShader> HorzBlurVSMShader = nullptr;

	std::unique_ptr<TShader> VertBlurVSMShader = nullptr;

	std::unique_ptr<TShader> GenerateVSMShader = nullptr;

	std::unique_ptr<TShader> TiledBaseLightCullingShader = nullptr;

	// PSO
	std::unique_ptr<TGraphicsPSOManager> GraphicsPSOManager;

	TGraphicsPSODescriptor IBLEnvironmentPSODescriptor;

	TGraphicsPSODescriptor IBLIrradiancePSODescriptor;

	TGraphicsPSODescriptor IBLPrefilterEnvPSODescriptor;

	TGraphicsPSODescriptor DeferredLightingPSODescriptor;

	TGraphicsPSODescriptor DebugSDFScenePSODescriptor;

	TGraphicsPSODescriptor SSAOPSODescriptor;

	TGraphicsPSODescriptor SSRPSODescriptor;

	TGraphicsPSODescriptor PostProcessPSODescriptor;

	TGraphicsPSODescriptor TAAPSODescriptor;

	std::unique_ptr<TComputePSOManager> ComputePSOManager;

	TComputePSODescriptor HorzBlurPSODescriptor;

	TComputePSODescriptor VertBlurPSODescriptor;

	TComputePSODescriptor HorzBlurVSMPSODescriptor;

	TComputePSODescriptor VertBlurVSMPSODescriptor;

	TComputePSODescriptor GenerateVSMPSODescriptor;

	TComputePSODescriptor TiledBaseLightCullingPSODescriptor;

	// MeshBatch and MeshCommand
	std::vector<TMeshBatch> MeshBatchs;

	std::unordered_map<TGraphicsPSODescriptor, TMeshCommandList> ShadowMeshCommandMap;

	std::unordered_map<TGraphicsPSODescriptor, TMeshCommandList> BaseMeshCommandMap;

	std::unordered_map<TGraphicsPSODescriptor, TMeshCommandList> BackDepthCommandMap;

	const int MaxRenderMeshCount = 100;

	// PrimitiveBatchs
	std::unordered_map<TGraphicsPSODescriptor, TPrimitiveBatch> PSOPrimitiveBatchMap;

	// SpriteBatchs
	std::unordered_map<TGraphicsPSODescriptor, TSpriteBatch> PSOSpriteBatchMap;

	// SpriteFont
	std::unique_ptr<TSpriteFont> SpriteFont = nullptr;

	const std::string FONT_TEXTURE_NAME = "FontTex";

	// Light
	TD3D12StructuredBufferRef LightShaderParametersBuffer = nullptr;

	TD3D12ConstantBufferRef LightCommonDataBuffer = nullptr;

	UINT LightCount = 0;

	// Shadow
	const UINT MAX_SHADOW_MAP_2D_NUM = 10;

	const UINT MAX_SHADOW_MAP_CUBE_NUM = 5;

	const UINT MAX_SDF_TEXTURE_COUNT = 20;

	const UINT ShadowSize = 4096;

	std::vector<TD3D12ShaderResourceView*> ShadowMapSRVs;

	std::vector<TD3D12ShaderResourceView*> ShadowMapCubeSRVs;

	// Sky
	TMeshComponent* SkyMeshComponent = nullptr;

	std::string SkyCubeTextureName;

	// PBR
	std::unique_ptr<TSceneCaptureCube> IBLEnvironmentMap;

	std::unique_ptr<TSceneCaptureCube> IBLIrradianceMap;

	std::vector<std::unique_ptr<TSceneCaptureCube>> IBLPrefilterEnvMaps;

	// Culling
	bool bEnableFrustumCulling = false;

	// D3D12RHI
	TD3D12RHI* D3D12RHI = nullptr;

	// Render settings
	TRenderSettings RenderSettings;

	bool bEnableIBLEnvLighting = false;
};

