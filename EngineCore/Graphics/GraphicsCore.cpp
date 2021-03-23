#include "GraphicsCore.h"
#include "RHI/CommandListManager.h"
namespace Graphics
{
	//class FCommandListmanager;

	ID3D12Device* g_Device = nullptr;
	FCommandListmanager g_CommandManager;
	FContextManager g_ContextManager;

	uint32_t g_DisplayWidth;
	uint32_t g_DisplayHeight;

	FDescriptorAllocator g_DescriptorAllocator[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES] =
	{
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
		D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
		D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
		D3D12_DESCRIPTOR_HEAP_TYPE_DSV
	};


}

void Graphics::Initialize(void)
{

}

void Graphics::Resize(uint32_t width, uint32_t height)
{

}

void Graphics::Terminate(void)
{

}

void Graphics::Shutdown(void)
{

}

void Graphics::Present(void)
{

}

uint64_t Graphics::GetFrameCount(void)
{
	return 0;
}

float Graphics::GetFrameRate(void)
{
	return 0.0f;
}

float Graphics::GetFrameTime(void)
{
	return 0.0f;
}