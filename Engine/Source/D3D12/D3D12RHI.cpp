#include "D3D12RHI.h"
#include <assert.h>

using Microsoft::WRL::ComPtr;

TD3D12RHI::TD3D12RHI()
{
}

TD3D12RHI::~TD3D12RHI()
{
	Destroy();
}

void TD3D12RHI::Initialize(HWND WindowHandle, int WindowWidth, int WindowHeight)
{
	// D3D12 debug
	UINT DxgiFactoryFlags = 0;
#if defined(DEBUG) || defined(_DEBUG) 
	{
		ComPtr<ID3D12Debug> DebugController;
		ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(DebugController.GetAddressOf())));
		DebugController->EnableDebugLayer();
	}

	ComPtr<IDXGIInfoQueue> DxgiInfoQueue;
	if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(DxgiInfoQueue.GetAddressOf()))))
	{
		DxgiFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;

		DxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, true);
		DxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, true);
	}

#endif

	// Create DxgiFactory
	ThrowIfFailed(CreateDXGIFactory2(DxgiFactoryFlags, IID_PPV_ARGS(DxgiFactory.GetAddressOf())));

	// Create Device
	Device = std::make_unique<TD3D12Device>(this);

	// Create Viewport
	ViewportInfo.WindowHandle = WindowHandle;
	ViewportInfo.BackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	ViewportInfo.DepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	ViewportInfo.bEnable4xMsaa = false;
	ViewportInfo.QualityOf4xMsaa = GetSupportMSAAQuality(ViewportInfo.BackBufferFormat);

	Viewport = std::make_unique<TD3D12Viewport>(this, ViewportInfo, WindowWidth, WindowHeight);

#ifdef _DEBUG
	LogAdapters();
#endif

}

void TD3D12RHI::Destroy()
{
	EndFrame();

	Viewport.reset();

	Device.reset();

//#ifdef _DEBUG
//	{
//		ComPtr<IDXGIDebug1> DxgiDebug;
//		if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&DxgiDebug))))
//		{
//			DxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_SUMMARY | DXGI_DEBUG_RLO_IGNORE_INTERNAL));
//		}
//	}
//#endif
}

const TD3D12ViewportInfo& TD3D12RHI::GetViewportInfo()
{
	return ViewportInfo;
}

IDXGIFactory4* TD3D12RHI::GetDxgiFactory()
{
	return DxgiFactory.Get();
}

void TD3D12RHI::LogAdapters()
{
	UINT i = 0;
	IDXGIAdapter* adapter = nullptr;
	std::vector<IDXGIAdapter*> adapterList;
	while (DxgiFactory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND)
	{
		DXGI_ADAPTER_DESC desc;
		adapter->GetDesc(&desc);

		std::wstring text = L"***Adapter: ";
		text += desc.Description;
		text += L"\n";

		OutputDebugString(text.c_str());

		adapterList.push_back(adapter);

		++i;
	}

	for (size_t i = 0; i < adapterList.size(); ++i)
	{
		LogAdapterOutputs(adapterList[i]);
		ReleaseCom(adapterList[i]);
	}
}

void TD3D12RHI::LogAdapterOutputs(IDXGIAdapter* adapter)
{
	UINT i = 0;
	IDXGIOutput* output = nullptr;
	while (adapter->EnumOutputs(i, &output) != DXGI_ERROR_NOT_FOUND)
	{
		DXGI_OUTPUT_DESC desc;
		output->GetDesc(&desc);

		std::wstring text = L"***Output: ";
		text += desc.DeviceName;
		text += L"\n";
		OutputDebugString(text.c_str());

		LogOutputDisplayModes(output, ViewportInfo.BackBufferFormat);

		ReleaseCom(output);

		++i;
	}
}

void TD3D12RHI::LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format)
{
	UINT count = 0;
	UINT flags = 0;

	// Call with nullptr to get list count.
	output->GetDisplayModeList(format, flags, &count, nullptr);

	std::vector<DXGI_MODE_DESC> modeList(count);
	output->GetDisplayModeList(format, flags, &count, &modeList[0]);

	for (auto& x : modeList)
	{
		UINT n = x.RefreshRate.Numerator;
		UINT d = x.RefreshRate.Denominator;
		std::wstring text =
			L"Width = " + std::to_wstring(x.Width) + L" " +
			L"Height = " + std::to_wstring(x.Height) + L" " +
			L"Refresh = " + std::to_wstring(n) + L"/" + std::to_wstring(d) +
			L"\n";

		::OutputDebugString(text.c_str());
	}
}

UINT TD3D12RHI::GetSupportMSAAQuality(DXGI_FORMAT BackBufferFormat)
{
	// Check 4X MSAA quality support for our back buffer format.
	// All Direct3D 11 capable devices support 4X MSAA for all render 
	// target formats, so we only need to check quality support.

	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
	msQualityLevels.Format = BackBufferFormat;
	msQualityLevels.SampleCount = 4;
	msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	msQualityLevels.NumQualityLevels = 0;
	ThrowIfFailed(Device->GetD3DDevice()->CheckFeatureSupport(
		D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
		&msQualityLevels,
		sizeof(msQualityLevels)));

	UINT QualityOf4xMsaa = msQualityLevels.NumQualityLevels;
	assert(QualityOf4xMsaa > 0 && "Unexpected MSAA quality level.");

	return QualityOf4xMsaa;
}


void TD3D12RHI::FlushCommandQueue()
{
	GetDevice()->GetCommandContext()->FlushCommandQueue();
}

void TD3D12RHI::ExecuteCommandLists()
{
	GetDevice()->GetCommandContext()->ExecuteCommandLists();
}

void TD3D12RHI::ResetCommandList()
{
	GetDevice()->GetCommandContext()->ResetCommandList();
}

void TD3D12RHI::ResetCommandAllocator()
{
	GetDevice()->GetCommandContext()->ResetCommandAllocator();
}

void TD3D12RHI::Present()
{
	GetViewport()->Present();
}

void TD3D12RHI::ResizeViewport(int NewWidth, int NewHeight)
{
	GetViewport()->OnResize(NewWidth, NewHeight);
}

void TD3D12RHI::TransitionResource(TD3D12Resource* Resource, D3D12_RESOURCE_STATES StateAfter)
{
	D3D12_RESOURCE_STATES StateBefore = Resource->CurrentState;

	if (StateBefore != StateAfter)
	{
		GetDevice()->GetCommandList()->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(Resource->D3DResource.Get(), StateBefore, StateAfter));

		Resource->CurrentState = StateAfter;
	}
}

void TD3D12RHI::CopyResource(TD3D12Resource* DstResource, TD3D12Resource* SrcResource)
{
	GetDevice()->GetCommandList()->CopyResource(DstResource->D3DResource.Get(), SrcResource->D3DResource.Get());
}

void TD3D12RHI::CopyBufferRegion(TD3D12Resource* DstResource, UINT64 DstOffset, TD3D12Resource* SrcResource, UINT64 SrcOffset, UINT64 Size)
{
	GetDevice()->GetCommandList()->CopyBufferRegion(DstResource->D3DResource.Get(), DstOffset, SrcResource->D3DResource.Get(), SrcOffset, Size);
}

void TD3D12RHI::CopyTextureRegion(const D3D12_TEXTURE_COPY_LOCATION* Dst, UINT DstX, UINT DstY, UINT DstZ, const D3D12_TEXTURE_COPY_LOCATION* Src, const D3D12_BOX* SrcBox)
{
	GetDevice()->GetCommandList()->CopyTextureRegion(Dst, DstX, DstY, DstZ, Src, SrcBox);
}

void TD3D12RHI::SetVertexBuffer(const TD3D12VertexBufferRef& VertexBuffer, UINT Offset, UINT Stride, UINT Size)
{
	// Transition resource state
	const TD3D12ResourceLocation& ResourceLocation = VertexBuffer->ResourceLocation;
	TD3D12Resource* Resource = ResourceLocation.UnderlyingResource;
	TransitionResource(Resource, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER | D3D12_RESOURCE_STATE_INDEX_BUFFER);

	// Set vertex buffer
	D3D12_VERTEX_BUFFER_VIEW VBV;
	VBV.BufferLocation = ResourceLocation.GPUVirtualAddress + Offset;
	VBV.StrideInBytes = Stride;
	VBV.SizeInBytes = Size;
	GetDevice()->GetCommandList()->IASetVertexBuffers(0, 1, &VBV);
}

void TD3D12RHI::SetIndexBuffer(const TD3D12IndexBufferRef& IndexBuffer, UINT Offset, DXGI_FORMAT Format, UINT Size)
{
	// Transition resource state
	const TD3D12ResourceLocation& ResourceLocation = IndexBuffer->ResourceLocation;
	TD3D12Resource* Resource = ResourceLocation.UnderlyingResource;
	TransitionResource(Resource, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER | D3D12_RESOURCE_STATE_INDEX_BUFFER);

	// Set vertex buffer
	D3D12_INDEX_BUFFER_VIEW IBV;
	IBV.BufferLocation = ResourceLocation.GPUVirtualAddress + Offset;
	IBV.Format = Format;
	IBV.SizeInBytes = Size;
	GetDevice()->GetCommandList()->IASetIndexBuffer(&IBV);
}

void TD3D12RHI::EndFrame()
{
	// Clean memory allocations
	GetDevice()->GetUploadBufferAllocator()->CleanUpAllocations();

	GetDevice()->GetDefaultBufferAllocator()->CleanUpAllocations();

	GetDevice()->GetTextureResourceAllocator()->CleanUpAllocations();

	// CommandContext
	GetDevice()->GetCommandContext()->EndFrame();
}