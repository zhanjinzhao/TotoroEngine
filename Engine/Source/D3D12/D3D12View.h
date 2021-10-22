#pragma once

#include "D3D12/D3D12HeapSlotAllocator.h"

class TD3D12Device;

class TD3D12View
{
public:
	TD3D12View(TD3D12Device* InDevice, D3D12_DESCRIPTOR_HEAP_TYPE InType, ID3D12Resource* InResource);

	virtual ~TD3D12View();

	D3D12_CPU_DESCRIPTOR_HANDLE GetDescriptorHandle() const { return HeapSlot.Handle; }

private:
	void Destroy();

protected:
	TD3D12Device* Device = nullptr;

	TD3D12HeapSlotAllocator* HeapSlotAllocator = nullptr;

	ID3D12Resource* Resource = nullptr;

	TD3D12HeapSlotAllocator::HeapSlot HeapSlot;

	D3D12_DESCRIPTOR_HEAP_TYPE Type;
};

class TD3D12ShaderResourceView : public TD3D12View
{
public:
	TD3D12ShaderResourceView(TD3D12Device* InDevice, const D3D12_SHADER_RESOURCE_VIEW_DESC& Desc, ID3D12Resource* InResource);

	virtual ~TD3D12ShaderResourceView();

protected:
	void CreateShaderResourceView(const D3D12_SHADER_RESOURCE_VIEW_DESC& Desc);
};

class TD3D12RenderTargetView : public TD3D12View
{
public:
	TD3D12RenderTargetView(TD3D12Device* InDevice, const D3D12_RENDER_TARGET_VIEW_DESC& Desc, ID3D12Resource* InResource);

	virtual ~TD3D12RenderTargetView();

protected:
	void CreateRenderTargetView(const D3D12_RENDER_TARGET_VIEW_DESC& Desc);
};


class TD3D12DepthStencilView : public TD3D12View
{
public:
	TD3D12DepthStencilView(TD3D12Device* InDevice, const D3D12_DEPTH_STENCIL_VIEW_DESC& Desc, ID3D12Resource* InResource);

	virtual ~TD3D12DepthStencilView();

protected:
	void CreateDepthStencilView(const D3D12_DEPTH_STENCIL_VIEW_DESC& Desc);
};

class TD3D12UnorderedAccessView : public TD3D12View
{
public:
	TD3D12UnorderedAccessView(TD3D12Device* InDevice, const D3D12_UNORDERED_ACCESS_VIEW_DESC& Desc, ID3D12Resource* InResource);

	virtual ~TD3D12UnorderedAccessView();

protected:
	void CreateUnorderedAccessView(const D3D12_UNORDERED_ACCESS_VIEW_DESC& Desc);
};