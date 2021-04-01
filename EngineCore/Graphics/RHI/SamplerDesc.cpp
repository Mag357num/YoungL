#include "pch.h"
#include "SamplerDesc.h"
#include "../GraphicsCore.h"
#include <map>

using namespace std;

namespace
{
	map<size_t, D3D12_CPU_DESCRIPTOR_HANDLE> SamplerCache;
}

D3D12_CPU_DESCRIPTOR_HANDLE FSamplerDesc::CreateDescriptor(void)
{
	size_t HashValue= Utility::HashState(this);
	auto Iter = SamplerCache.find(HashValue);
	if (Iter != SamplerCache.end())
	{
		return Iter->second;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE Handle = Graphics::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
	Graphics::g_Device->CreateSampler(this, Handle);

	return Handle;
}

void FSamplerDesc::CreateDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE& Handle)
{
	Graphics::g_Device->CreateSampler(this, Handle);
}