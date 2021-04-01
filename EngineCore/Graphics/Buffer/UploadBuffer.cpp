#include "pch.h"
#include "UploadBuffer.h"
#include "../GraphicsCore.h"

using namespace Graphics;

void FUploadBuffer::Create(const std::wstring& Name, size_t BufferSize)
{
	Destroy();
	BufferSize = BufferSize;

	D3D12_HEAP_PROPERTIES HeapPro;
	HeapPro.Type = D3D12_HEAP_TYPE_UPLOAD;
	HeapPro.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	HeapPro.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	HeapPro.CreationNodeMask = 1;
	HeapPro.VisibleNodeMask = 1;


	D3D12_RESOURCE_DESC ResourceDesc = {};
	ResourceDesc.Width = BufferSize;
	ResourceDesc.Height = 1;
	ResourceDesc.DepthOrArraySize = 1;
	ResourceDesc.MipLevels = 1;
	ResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	ResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	ResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	ResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	ResourceDesc.SampleDesc.Count = 1;
	ResourceDesc.SampleDesc.Quality = 0;


	ASSERT_SUCCEEDED(g_Device->CreateCommittedResource(&HeapPro, D3D12_HEAP_FLAG_NONE, &ResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr, MY_IID_PPV_ARGS(&Resource)));

	GpuVirtualAddress = Resource->GetGPUVirtualAddress();

#ifdef RELEASE
	(Name)
#else
	Resource->SetName(Name.c_str());
#endif // RELEASE

}

void* FUploadBuffer::Map()
{
	void* Memory;
	CD3DX12_RANGE range(0, BufferSize);
	Resource->Map(0, &range, &Memory);

	return Memory;
}

void FUploadBuffer::Unmap(size_t Begin /* = 0 */, size_t End /* = -1 */)
{
	CD3DX12_RANGE Range(Begin, std::min(End, BufferSize));
	Resource->Unmap(0, &Range);
}