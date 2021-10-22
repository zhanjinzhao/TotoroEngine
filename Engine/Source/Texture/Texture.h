#pragma once

#include <string>
#include "Texture/TextureInfo.h"
#include "D3D12/D3D12Texture.h"
#include "D3D12/D3D12RHI.h"

struct TTextureResource
{
	TTextureInfo TextureInfo;

	std::vector<uint8_t> TextureData;

	std::vector<D3D12_SUBRESOURCE_DATA> InitData;
};

class TTexture
{
public:
	TTexture(const std::string& InName, ETextureType InType, bool InbSRGB, std::wstring InFilePath)
		:Name(InName), Type(InType), bSRGB(InbSRGB), FilePath(InFilePath)
	{}

	virtual ~TTexture()
	{}

	TTexture(const TTexture& Other) = delete;

	TTexture& operator=(const TTexture& Other) = delete;

public:
	void LoadTextureResourceFromFlie(TD3D12RHI* D3D12RHI);

	void SetTextureResourceDirectly(const TTextureInfo& InTextureInfo, const std::vector<uint8_t>& InTextureData, 
		const D3D12_SUBRESOURCE_DATA& InInitData);

	void CreateTexture(TD3D12RHI* D3D12RHI);

	TD3D12TextureRef GetD3DTexture() { return D3DTexture; }

private:
	static std::wstring GetExtension(std::wstring path);

	void LoadDDSTexture(TD3D12Device* Device);

	void LoadWICTexture(TD3D12Device* Device);

	void LoadHDRTexture(TD3D12Device* Device);

public:
	std::string Name;

	ETextureType Type;

	std::wstring FilePath;

	bool bSRGB = true;

	TTextureResource TextureResource;

	TD3D12TextureRef D3DTexture = nullptr;
};

class TTexture2D : public TTexture
{
public:
	TTexture2D(const std::string& InName, bool InbSRGB, std::wstring InFilePath);

	~TTexture2D();
};

class TTextureCube : public TTexture
{
public:
	TTextureCube(const std::string& InName, bool InbSRGB, std::wstring InFilePath);

	~TTextureCube();
};

class TTexture3D : public TTexture
{
public:
	TTexture3D(const std::string& InName, bool InbSRGB, std::wstring InFilePath);

	~TTexture3D();
};
