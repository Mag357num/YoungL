#include "LinearAllocator.h"
#include "../GraphicsCore.h"
#include "../../Math/Common.h"

using namespace Graphics;
using namespace std;

ELinearAllocatorType FLinearAllocatorPagemanager::AutoType = EGpuExlusive;

FLinearAllocatorPagemanager::FLinearAllocatorPagemanager()
{
	AllocationType = AutoType;
	AutoType = (ELinearAllocatorType)(AutoType + 1);
	ASSERT(AutoType < ENumAllocatorTypes);
}

FLinearAllocatorPagemanager FLinearAllocator::PageManager[2];

FLinearAllocationPage* FLinearAllocatorPagemanager::RequestPage()
{
	std::lock_guard<std::mutex> LockGuard(AllocateMutex);

	while (!RetiredPages.empty() && g_CommandManager.IsFenceComplete(RetiredPages.front().first))
	{
		AvailablePages.push(RetiredPages.front().second);
		RetiredPages.pop();
	}

	FLinearAllocationPage* PagePtr = nullptr;
	
	if (!AvailablePages.empty())
	{
		PagePtr = AvailablePages.front();
		AvailablePages.pop();
	}
	else
	{
		PagePtr = CreateNewPage();
		PagePool.emplace_back(PagePtr);
	}

	return PagePtr;
}

FLinearAllocationPage* FLinearAllocatorPagemanager::CreateNewPage(size_t PageSize/* = 0 */)
{
	D3D12_HEAP_PROPERTIES HeapProperties;
	HeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	HeapProperties.CreationNodeMask = 1;
	HeapProperties.VisibleNodeMask = 1;
	HeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	D3D12_RESOURCE_DESC ResourceDesc;
	ResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	ResourceDesc.Alignment = 0;
	ResourceDesc.Height = 1;
	ResourceDesc.DepthOrArraySize = 1;
	ResourceDesc.MipLevels = 1;
	ResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	ResourceDesc.SampleDesc.Count = 1;
	ResourceDesc.SampleDesc.Quality = 0;
	ResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;


	D3D12_RESOURCE_STATES UseState;

	if (AllocationType == EGpuExlusive)
	{
		HeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
		ResourceDesc.Width = PageSize == 0 ? EGpuAllocatorPageSize : PageSize;
		ResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		UseState = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
	}
	else
	{
		HeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
		ResourceDesc.Width = PageSize == 0 ? ECpuAllocaterPageSize : PageSize;
		ResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
		UseState = D3D12_RESOURCE_STATE_GENERIC_READ;
	}


	ID3D12Resource* Buffer;
	ASSERT_SUCCEEDED(g_Device->CreateCommittedResource(&HeapProperties, D3D12_HEAP_FLAG_NONE, &ResourceDesc, UseState, nullptr, MY_IID_PPV_ARGS(&Buffer)));

	Buffer->SetName(L"Allocation page");
	return new FLinearAllocationPage(Buffer, UseState);
}

void FLinearAllocatorPagemanager::DiscardPages(uint64_t FenceID, const std::vector<FLinearAllocationPage*>& Pages)
{
	std::lock_guard<std::mutex> LockGuard(AllocateMutex);
	for (auto Iter = Pages.begin(); Iter != Pages.end() ; ++Iter)
	{
		RetiredPages.push(std::make_pair(FenceID, *Iter));
	}
}

void FLinearAllocatorPagemanager::FreeLargePools(uint64_t FenceID, const std::vector<FLinearAllocationPage*>& Pages)
{
	std::lock_guard<std::mutex> LockGuard(AllocateMutex);

	while (!DeletionQueue.empty() && g_CommandManager.IsFenceComplete(DeletionQueue.front().first))
	{
		delete DeletionQueue.front().second;
		DeletionQueue.pop();
	}

	for (auto It = Pages.begin(); It != Pages.end() ; ++It)
	{
		(*It)->Unmap();
		DeletionQueue.push(std::make_pair(FenceID, *It));
	}

}

FDynAlloc FLinearAllocator::AllocateLargePage(size_t SizeInBytes)
{
	FLinearAllocationPage* OneOff = PageManager[AllocationType].CreateNewPage(SizeInBytes);
	LargePageList.push_back(OneOff);

	FDynAlloc Ret(*OneOff, 0, SizeInBytes);
	Ret.DataPtr = OneOff->CpuVirtualAddress;
	Ret.GpuAddress = OneOff->GpuVirtualAddress;

	return Ret;
}

FDynAlloc FLinearAllocator::Allocate(size_t SizeInBytes, size_t Alignment /* = DEFAULT_ALIGNMENT */)
{
	const size_t AlignmentMask = Alignment - 1;
	ASSERT((AlignmentMask & Alignment) == 0);

	//
	const size_t AlignedSize = Math::AlignUpWithMask(SizeInBytes, AlignmentMask);

	if (AlignedSize > PageSize)
	{
		return AllocateLargePage(AlignedSize);
	}

	CurOffset = Math::AlignUp(CurOffset, Alignment);

	if (CurOffset + AlignedSize > PageSize)
	{
		ASSERT(CurPage != nullptr);
		RetiredPages.push_back(CurPage);
		CurPage = nullptr;
	}

	if (CurPage == nullptr)
	{
		CurPage = PageManager[AllocationType].RequestPage();
		CurOffset = 0;
	}

	FDynAlloc Ret(*CurPage, CurOffset, AlignedSize);
	Ret.DataPtr =(uint8_t*) CurPage->CpuVirtualAddress + CurOffset;
	Ret.GpuAddress = CurPage->GpuVirtualAddress + CurOffset;

	CurOffset += AlignedSize;

	return Ret;
}

