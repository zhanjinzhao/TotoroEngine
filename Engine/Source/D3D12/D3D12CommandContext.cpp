#include "D3D12CommandContext.h"
#include "D3D12Device.h"

TD3D12CommandContext::TD3D12CommandContext(TD3D12Device* InDevice)
	:Device(InDevice)
{
	CreateCommandContext();

	DescriptorCache = std::make_unique<TD3D12DescriptorCache>(Device);
}

TD3D12CommandContext::~TD3D12CommandContext()
{
	DestroyCommandContext();
}

void TD3D12CommandContext::CreateCommandContext()
{
	//Create fence
	ThrowIfFailed(Device->GetD3DDevice()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&Fence)));

	//Create direct type commandQueue
	D3D12_COMMAND_QUEUE_DESC QueueDesc = {};
	QueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	QueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	ThrowIfFailed(Device->GetD3DDevice()->CreateCommandQueue(&QueueDesc, IID_PPV_ARGS(&CommandQueue)));

	//Create direct type commandAllocator
	ThrowIfFailed(Device->GetD3DDevice()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(CommandListAlloc.GetAddressOf())));

	//Create direct type commandList
	ThrowIfFailed(Device->GetD3DDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, CommandListAlloc.Get(),
		nullptr, IID_PPV_ARGS(CommandList.GetAddressOf())));

	// Start off in a closed state. 
	// This is because the first time we refer to the command list we will Reset it,
	// and it needs to be closed before calling Reset.
	ThrowIfFailed(CommandList->Close());
}

void TD3D12CommandContext::DestroyCommandContext()
{
	//Don't need to do anything
	//Microsoft::WRL::ComPtr will destroy resource automatically
}

void TD3D12CommandContext::ResetCommandAllocator()
{
	// Command list allocators can only be reset when the associated command lists have finished execution on the GPU.
	// So we should use fences to determine GPU execution progress(see FlushCommandQueue()).
	ThrowIfFailed(CommandListAlloc->Reset());
}

void TD3D12CommandContext::ResetCommandList()
{
	// Reusing the command list can reuses memory.
	// A command list can be reset after it has been added to the command queue via ExecuteCommandList.
	// Before an app calls Reset, the command list must be in the "closed" state. 
	// After Reset succeeds, the command list is left in the "recording" state. 
	ThrowIfFailed(CommandList->Reset(CommandListAlloc.Get(), nullptr));
}

void TD3D12CommandContext::ExecuteCommandLists()
{
	// Done recording commands.
	ThrowIfFailed(CommandList->Close());

	// Add the command list to the queue for execution.
	ID3D12CommandList* cmdsLists[] = { CommandList.Get() };
	CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
}

void TD3D12CommandContext::FlushCommandQueue()
{
	// Advance the fence value to mark commands up to this fence point.
	CurrentFenceValue++;
	
	// Add an instruction to the command queue to set a new fence point.  
	// Because we are on the GPU timeline, the new fence point won't be set until the GPU finishes
	// processing all the commands prior to this Signal().
	ThrowIfFailed(CommandQueue->Signal(Fence.Get(), CurrentFenceValue));
	
	// Wait until the GPU has completed commands up to this fence point.
	if (Fence->GetCompletedValue() < CurrentFenceValue)
	{
		HANDLE eventHandle = CreateEvent(nullptr, false, false, nullptr);
	
		// Fire event when GPU hits current fence.  
		ThrowIfFailed(Fence->SetEventOnCompletion(CurrentFenceValue, eventHandle));
	
		// Wait until the GPU hits current fence event is fired.
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}
}

void TD3D12CommandContext::EndFrame()
{
	DescriptorCache->Reset();
}

