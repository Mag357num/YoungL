#pragma once
#include "RHI/CommandListManager.h"
#include "RHI/CommandContext.h"
#include "RHI/DescriptorHeap.h"

namespace Graphics
{
#ifndef RELEASE
	extern const GUID WKPDID_D3DDebugObjectName;
#endif

	using namespace Microsoft::WRL;

	void Initialize(void);
	void Terminate(void);
	void Shutdown(void);

	extern ID3D12Device* g_Device;

	//reserved for command manager
	extern FCommandListmanager g_CommandManager;
	//reservced for context manager
	extern FContextManager g_ContextManager;

	extern D3D_FEATURE_LEVEL g_D3DFeatureLevel;
	extern bool g_bTypedUAVLoadSupport_R11G11B11_FlOAT;
	extern bool g_bEnableHDROutput;
	extern FCommandSignature DrawIndirectCommandSignature;

	//reserved for DescriptorAllocator
	extern FDescriptorAllocator g_DescriptorAllocator[];
	inline D3D12_CPU_DESCRIPTOR_HANDLE AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE Type, UINT Count = 1)
	{
		return g_DescriptorAllocator[Type].Allocate(Count);
	}

	
}