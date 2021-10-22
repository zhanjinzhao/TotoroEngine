#pragma once

#include "D3D12CommandContext.h"
#include "D3D12MemoryAllocator.h"
#include "D3D12HeapSlotAllocator.h"

class TD3D12RHI;

class TD3D12Device
{
public:
	TD3D12Device(TD3D12RHI* InD3D12RHI);

	~TD3D12Device();

public:
	ID3D12Device* GetD3DDevice() { return D3DDevice.Get(); }

	TD3D12CommandContext* GetCommandContext() { return CommandContext.get(); }

	ID3D12CommandQueue* GetCommandQueue() { return CommandContext->GetCommandQueue(); }

	ID3D12GraphicsCommandList* GetCommandList() { return CommandContext->GetCommandList(); }

	TD3D12UploadBufferAllocator* GetUploadBufferAllocator() { return UploadBufferAllocator.get(); }

	TD3D12DefaultBufferAllocator* GetDefaultBufferAllocator() { return DefaultBufferAllocator.get(); }

	TD3D3TextureResourceAllocator* GetTextureResourceAllocator() { return TextureResourceAllocator.get(); }

	TD3D12HeapSlotAllocator* GetHeapSlotAllocator(D3D12_DESCRIPTOR_HEAP_TYPE HeapType);

private:
	void Initialize();

private:
	TD3D12RHI* D3D12RHI = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Device> D3DDevice = nullptr;

	std::unique_ptr<TD3D12CommandContext> CommandContext = nullptr;

private:
	std::unique_ptr<TD3D12UploadBufferAllocator> UploadBufferAllocator = nullptr;

	std::unique_ptr<TD3D12DefaultBufferAllocator> DefaultBufferAllocator = nullptr;

	std::unique_ptr<TD3D3TextureResourceAllocator> TextureResourceAllocator = nullptr;

	std::unique_ptr<TD3D12HeapSlotAllocator> RTVHeapSlotAllocator = nullptr;

	std::unique_ptr<TD3D12HeapSlotAllocator> DSVHeapSlotAllocator = nullptr;

	std::unique_ptr<TD3D12HeapSlotAllocator> SRVHeapSlotAllocator = nullptr;
	
};