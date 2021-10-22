#pragma once

#include "D3D12Utils.h"
#include <list>

class TD3D12HeapSlotAllocator
{
public:
	typedef D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHandle;
	typedef decltype(DescriptorHandle::ptr) DescriptorHandleRaw;

	struct HeapSlot
	{
		uint32_t HeapIndex;
		D3D12_CPU_DESCRIPTOR_HANDLE Handle;
	};

private:
	struct FreeRange 
	{ 
		DescriptorHandleRaw Start;
		DescriptorHandleRaw End;
	};

	struct HeapEntry
	{
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> Heap = nullptr;
		std::list<TD3D12HeapSlotAllocator::FreeRange> FreeList;

		HeapEntry() { }
	};

public:
	TD3D12HeapSlotAllocator(ID3D12Device* InDevice, D3D12_DESCRIPTOR_HEAP_TYPE Type, uint32_t NumDescriptorsPerHeap);

	~TD3D12HeapSlotAllocator();

	HeapSlot AllocateHeapSlot();

	void FreeHeapSlot(const HeapSlot& Slot);

private:
	D3D12_DESCRIPTOR_HEAP_DESC CreateHeapDesc(D3D12_DESCRIPTOR_HEAP_TYPE Type, uint32_t NumDescriptorsPerHeap);

	void AllocateHeap();

private:
	ID3D12Device* D3DDevice;

	const D3D12_DESCRIPTOR_HEAP_DESC HeapDesc;

	const uint32_t DescriptorSize;

	std::vector<HeapEntry> HeapMap;
};