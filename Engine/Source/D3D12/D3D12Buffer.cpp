#include "D3D12Buffer.h"
#include "D3D12RHI.h"

TD3D12ConstantBufferRef TD3D12RHI::CreateConstantBuffer(const void* Contents, uint32_t Size)
{
	TD3D12ConstantBufferRef ConstantBufferRef = std::make_shared<TD3D12ConstantBuffer>();

	auto UploadBufferAllocator = GetDevice()->GetUploadBufferAllocator();
	void* MappedData = UploadBufferAllocator->AllocUploadResource(Size, UPLOAD_RESOURCE_ALIGNMENT, ConstantBufferRef->ResourceLocation);

	memcpy(MappedData, Contents, Size);

	return ConstantBufferRef;
}

TD3D12StructuredBufferRef TD3D12RHI::CreateStructuredBuffer(const void* Contents, uint32_t ElementSize, uint32_t ElementCount)
{
	assert(Contents != nullptr && ElementSize > 0 && ElementCount > 0);

	TD3D12StructuredBufferRef StructuredBufferRef = std::make_shared<TD3D12StructuredBuffer>();

	auto UploadBufferAllocator = GetDevice()->GetUploadBufferAllocator();
	uint32_t DataSize = ElementSize * ElementCount;
	// Align to ElementSize
	void* MappedData = UploadBufferAllocator->AllocUploadResource(DataSize, ElementSize, StructuredBufferRef->ResourceLocation);

	memcpy(MappedData, Contents, DataSize);


	// Create SRV
	{
		TD3D12ResourceLocation& Location = StructuredBufferRef->ResourceLocation;
		const uint64_t Offset = Location.OffsetFromBaseOfResource;
		ID3D12Resource* BufferResource = Location.UnderlyingResource->D3DResource.Get();

		D3D12_SHADER_RESOURCE_VIEW_DESC SrvDesc = {};
		SrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		SrvDesc.Format = DXGI_FORMAT_UNKNOWN;
		SrvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		SrvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
		SrvDesc.Buffer.StructureByteStride = ElementSize;
		SrvDesc.Buffer.NumElements = ElementCount;
		SrvDesc.Buffer.FirstElement = Offset / ElementSize;

		StructuredBufferRef->SetSRV(std::make_unique<TD3D12ShaderResourceView>(GetDevice(), SrvDesc, BufferResource));
	}


	return StructuredBufferRef;
}

TD3D12RWStructuredBufferRef TD3D12RHI::CreateRWStructuredBuffer(uint32_t ElementSize, uint32_t ElementCount)
{
	TD3D12RWStructuredBufferRef RWStructuredBufferRef = std::make_shared<TD3D12RWStructuredBuffer>();

	uint32_t DataSize = ElementSize * ElementCount;
	// Align to ElementSize
	CreateDefaultBuffer(DataSize, ElementSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, RWStructuredBufferRef->ResourceLocation);

	TD3D12ResourceLocation& Location = RWStructuredBufferRef->ResourceLocation;
	const uint64_t Offset = Location.OffsetFromBaseOfResource;
	ID3D12Resource* BufferResource = Location.UnderlyingResource->D3DResource.Get();

	// Create SRV
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC SrvDesc = {};
		SrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		SrvDesc.Format = DXGI_FORMAT_UNKNOWN;
		SrvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		SrvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
		SrvDesc.Buffer.StructureByteStride = ElementSize;
		SrvDesc.Buffer.NumElements = ElementCount;
		SrvDesc.Buffer.FirstElement = Offset / ElementSize;

		RWStructuredBufferRef->SetSRV(std::make_unique<TD3D12ShaderResourceView>(GetDevice(), SrvDesc, BufferResource));
	}

	// Create UAV
	{
		D3D12_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {};
		UAVDesc.Format = DXGI_FORMAT_UNKNOWN;
		UAVDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		UAVDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
		UAVDesc.Buffer.StructureByteStride = ElementSize;
		UAVDesc.Buffer.NumElements = ElementCount;
		UAVDesc.Buffer.FirstElement = Offset / ElementSize;
		UAVDesc.Buffer.CounterOffsetInBytes = 0;

		RWStructuredBufferRef->SetUAV(std::make_unique<TD3D12UnorderedAccessView>(GetDevice(), UAVDesc, BufferResource));
	}

	return RWStructuredBufferRef;
}

TD3D12VertexBufferRef TD3D12RHI::CreateVertexBuffer(const void* Contents, uint32_t Size)
{
	TD3D12VertexBufferRef VertexBufferRef = std::make_shared<TD3D12VertexBuffer>();

	CreateAndInitDefaultBuffer(Contents, Size, DEFAULT_RESOURCE_ALIGNMENT, VertexBufferRef->ResourceLocation);

	return VertexBufferRef;
}

TD3D12IndexBufferRef TD3D12RHI::CreateIndexBuffer(const void* Contents, uint32_t Size)
{
	TD3D12IndexBufferRef IndexBufferRef = std::make_shared<TD3D12IndexBuffer>();

	CreateAndInitDefaultBuffer(Contents, Size, DEFAULT_RESOURCE_ALIGNMENT, IndexBufferRef->ResourceLocation);

	return IndexBufferRef;
}

TD3D12ReadBackBufferRef TD3D12RHI::CreateReadBackBuffer(uint32_t Size)
{
	TD3D12ReadBackBufferRef ReadBackBufferRef = std::make_shared<TD3D12ReadBackBuffer>();

	Microsoft::WRL::ComPtr<ID3D12Resource> Resource;

	HRESULT Hr = Device->GetD3DDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(Size),
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&Resource));

	ThrowIfFailed(Hr);

	TD3D12Resource* NewResource = new TD3D12Resource(Resource, D3D12_RESOURCE_STATE_COPY_DEST);
	ReadBackBufferRef->ResourceLocation.UnderlyingResource = NewResource;
	ReadBackBufferRef->ResourceLocation.SetType(TD3D12ResourceLocation::EResourceLocationType::StandAlone);

	return ReadBackBufferRef;
}

void TD3D12RHI::CreateDefaultBuffer(uint32_t Size, uint32_t Alignment, D3D12_RESOURCE_FLAGS Flags, TD3D12ResourceLocation& ResourceLocation)
{
	//Create default resource
	D3D12_RESOURCE_DESC ResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(Size, Flags);
	auto DefaultBufferAllocator = GetDevice()->GetDefaultBufferAllocator();
	DefaultBufferAllocator->AllocDefaultResource(ResourceDesc, Alignment, ResourceLocation);
}

void TD3D12RHI::CreateAndInitDefaultBuffer(const void* Contents, uint32_t Size, uint32_t Alignment, TD3D12ResourceLocation& ResourceLocation)
{
	//Create default resource
	CreateDefaultBuffer(Size, Alignment, D3D12_RESOURCE_FLAG_NONE, ResourceLocation);

	//Create upload resource 
	TD3D12ResourceLocation UploadResourceLocation;
	auto UploadBufferAllocator = GetDevice()->GetUploadBufferAllocator();
	void* MappedData = UploadBufferAllocator->AllocUploadResource(Size, UPLOAD_RESOURCE_ALIGNMENT, UploadResourceLocation);

	//Copy contents to upload resource
	memcpy(MappedData, Contents, Size);

	//Copy data from upload resource to default resource
	TD3D12Resource* DefaultBuffer = ResourceLocation.UnderlyingResource;
	TD3D12Resource* UploadBuffer = UploadResourceLocation.UnderlyingResource;

	TransitionResource(DefaultBuffer, D3D12_RESOURCE_STATE_COPY_DEST);
	CopyBufferRegion(DefaultBuffer, ResourceLocation.OffsetFromBaseOfResource, UploadBuffer, UploadResourceLocation.OffsetFromBaseOfResource, Size);
}


