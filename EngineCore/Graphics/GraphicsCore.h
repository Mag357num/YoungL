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
	void Resize(uint32_t width, uint32_t height);
	void Terminate(void);
	void Shutdown(void);
	void Present(void);

	extern uint32_t g_DisplayWidth;
	extern uint32_t g_DisplayHeight;

	//return the number of elapsed frames since app start
	uint64_t GetFrameCount(void);

	float GetFrameTime(void);

	float GetFrameRate(void);


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

	//for root signature
	extern FRootSignature g_GeneratedMipsRS;
	//extern FComputePSO g_GeneratedMipsLinearPSO[4];
	//extern FComputePSO g_GeneratedMipsGammaPSO[4];

	enum eResolution { K720p, K900p, K1080p, K1440p, K1800p, K2160p };

	//extern BoolVar s_EnableVSync;
	//extern EnumVar TargetResolution;
	extern uint32_t g_DisplayWidth;
	extern uint32_t g_DisplayHeight;
}