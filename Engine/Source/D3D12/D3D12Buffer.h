#pragma once

#include "D3D12Resource.h"
#include "D3D12View.h"

class TD3D12Buffer
{
public:
	TD3D12Buffer() {}

	virtual ~TD3D12Buffer() {}

	TD3D12Resource* GetResource() { return ResourceLocation.UnderlyingResource; }

public:
	TD3D12ResourceLocation ResourceLocation;
};

class TD3D12ConstantBuffer : public TD3D12Buffer
{

};
typedef std::shared_ptr<TD3D12ConstantBuffer> TD3D12ConstantBufferRef;


class TD3D12StructuredBuffer : public TD3D12Buffer
{
public:
	TD3D12ShaderResourceView* GetSRV()
	{
		return SRV.get();
	}

	void SetSRV(std::unique_ptr<TD3D12ShaderResourceView>& InSRV)
	{
		SRV = std::move(InSRV);
	}

private:
	std::unique_ptr<TD3D12ShaderResourceView> SRV = nullptr;
};
typedef std::shared_ptr<TD3D12StructuredBuffer> TD3D12StructuredBufferRef;


class TD3D12RWStructuredBuffer : public TD3D12Buffer
{
public:
	TD3D12ShaderResourceView* GetSRV()
	{
		return SRV.get();
	}

	void SetSRV(std::unique_ptr<TD3D12ShaderResourceView>& InSRV)
	{
		SRV = std::move(InSRV);
	}

	TD3D12UnorderedAccessView* GetUAV()
	{
		return UAV.get();
	}

	void SetUAV(std::unique_ptr<TD3D12UnorderedAccessView>& InUAV)
	{
		UAV = std::move(InUAV);
	}

private:
	std::unique_ptr<TD3D12ShaderResourceView> SRV = nullptr;

	std::unique_ptr<TD3D12UnorderedAccessView> UAV = nullptr;
};
typedef std::shared_ptr<TD3D12RWStructuredBuffer> TD3D12RWStructuredBufferRef;


class TD3D12VertexBuffer : public TD3D12Buffer
{

};
typedef std::shared_ptr<TD3D12VertexBuffer> TD3D12VertexBufferRef;


class TD3D12IndexBuffer : public TD3D12Buffer
{

};
typedef std::shared_ptr<TD3D12IndexBuffer> TD3D12IndexBufferRef;


class TD3D12ReadBackBuffer : public TD3D12Buffer
{

};
typedef std::shared_ptr<TD3D12ReadBackBuffer> TD3D12ReadBackBufferRef;