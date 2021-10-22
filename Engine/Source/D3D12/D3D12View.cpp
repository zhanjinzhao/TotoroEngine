#include "D3D12View.h"
#include "D3D12Device.h"
#include <assert.h>


TD3D12View::TD3D12View(TD3D12Device* InDevice, D3D12_DESCRIPTOR_HEAP_TYPE InType, ID3D12Resource* InResource)
	:Device(InDevice),
	Type(InType),
	Resource(InResource)
{
	HeapSlotAllocator = Device->GetHeapSlotAllocator(Type);

	if (HeapSlotAllocator)
	{
		HeapSlot = HeapSlotAllocator->AllocateHeapSlot();
		assert(HeapSlot.Handle.ptr != 0);
	}
}

TD3D12View::~TD3D12View()
{
	Destroy();
}

void TD3D12View::Destroy()
{
	if (HeapSlotAllocator)
	{
		HeapSlotAllocator->FreeHeapSlot(HeapSlot);
	}
}

TD3D12ShaderResourceView::TD3D12ShaderResourceView(TD3D12Device* InDevice, const D3D12_SHADER_RESOURCE_VIEW_DESC& Desc, ID3D12Resource* InResource)
	:TD3D12View(InDevice, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, InResource)
{
	CreateShaderResourceView(Desc);
}

TD3D12ShaderResourceView::~TD3D12ShaderResourceView()
{

}

void TD3D12ShaderResourceView::CreateShaderResourceView(const D3D12_SHADER_RESOURCE_VIEW_DESC& Desc)
{
	 Device->GetD3DDevice()->CreateShaderResourceView(Resource, &Desc, HeapSlot.Handle);
}


TD3D12RenderTargetView::TD3D12RenderTargetView(TD3D12Device* InDevice, const D3D12_RENDER_TARGET_VIEW_DESC& Desc, ID3D12Resource* InResource)
	:TD3D12View(InDevice, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, InResource)
{
	CreateRenderTargetView(Desc);
}

TD3D12RenderTargetView::~TD3D12RenderTargetView()
{

}

void TD3D12RenderTargetView::CreateRenderTargetView(const D3D12_RENDER_TARGET_VIEW_DESC& Desc)
{
	Device->GetD3DDevice()->CreateRenderTargetView(Resource, &Desc, HeapSlot.Handle);
}


TD3D12DepthStencilView::TD3D12DepthStencilView(TD3D12Device* InDevice, const D3D12_DEPTH_STENCIL_VIEW_DESC& Desc, ID3D12Resource* InResource)
	:TD3D12View(InDevice, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, InResource)
{
	CreateDepthStencilView(Desc);
}

TD3D12DepthStencilView::~TD3D12DepthStencilView()
{

}

void TD3D12DepthStencilView::CreateDepthStencilView(const D3D12_DEPTH_STENCIL_VIEW_DESC& Desc)
{
	Device->GetD3DDevice()->CreateDepthStencilView(Resource, &Desc, HeapSlot.Handle);
}


TD3D12UnorderedAccessView::TD3D12UnorderedAccessView(TD3D12Device* InDevice, const D3D12_UNORDERED_ACCESS_VIEW_DESC& Desc, ID3D12Resource* InResource)
	:TD3D12View(InDevice, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, InResource)
{
	CreateUnorderedAccessView(Desc);
}

TD3D12UnorderedAccessView::~TD3D12UnorderedAccessView()
{

}

void TD3D12UnorderedAccessView::CreateUnorderedAccessView(const D3D12_UNORDERED_ACCESS_VIEW_DESC& Desc)
{
	Device->GetD3DDevice()->CreateUnorderedAccessView(Resource, nullptr, &Desc, HeapSlot.Handle);
}