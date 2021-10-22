#include "D3D12Device.h"
#include "D3D12RHI.h"

using Microsoft::WRL::ComPtr;

TD3D12Device::TD3D12Device(TD3D12RHI* InD3D12RHI)
	:D3D12RHI(InD3D12RHI)
{
	Initialize();
}

TD3D12Device::~TD3D12Device()
{

}

void TD3D12Device::Initialize()
{
	// Try to create hardware device.
	HRESULT HardwareResult = D3D12CreateDevice(nullptr/*default adapter*/, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&D3DDevice));

	// Fallback to WARP device.
	if (FAILED(HardwareResult))
	{
		ComPtr<IDXGIAdapter> WarpAdapter;
		ThrowIfFailed(D3D12RHI->GetDxgiFactory()->EnumWarpAdapter(IID_PPV_ARGS(&WarpAdapter)));

		ThrowIfFailed(D3D12CreateDevice(WarpAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&D3DDevice)));
	}

	//Create CommandContext
	CommandContext = std::make_unique<TD3D12CommandContext>(this);

	//Create memory allocator
	UploadBufferAllocator = std::make_unique<TD3D12UploadBufferAllocator>(D3DDevice.Get());

	DefaultBufferAllocator = std::make_unique<TD3D12DefaultBufferAllocator>(D3DDevice.Get());

	TextureResourceAllocator = std::make_unique<TD3D3TextureResourceAllocator>(D3DDevice.Get());

	//Create heapSlot allocator
	RTVHeapSlotAllocator = std::make_unique<TD3D12HeapSlotAllocator>(D3DDevice.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 200);

	DSVHeapSlotAllocator = std::make_unique<TD3D12HeapSlotAllocator>(D3DDevice.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 200);

	SRVHeapSlotAllocator = std::make_unique<TD3D12HeapSlotAllocator>(D3DDevice.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 200);
}

TD3D12HeapSlotAllocator* TD3D12Device::GetHeapSlotAllocator(D3D12_DESCRIPTOR_HEAP_TYPE HeapType)
{
	switch (HeapType)
	{
	case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
		return SRVHeapSlotAllocator.get();
		break;

	//case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:
	//	break;

	case D3D12_DESCRIPTOR_HEAP_TYPE_RTV:
		return RTVHeapSlotAllocator.get();
		break;

	case D3D12_DESCRIPTOR_HEAP_TYPE_DSV:
		return DSVHeapSlotAllocator.get();
		break;

	default:
		return nullptr;
	}
}
