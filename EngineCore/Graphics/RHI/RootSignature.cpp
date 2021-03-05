#include "pch.h"

using namespace Graphics;
using namespace std;
using Microsoft::WRL::ComPtr;

static std::map<size_t, ComPtr<ID3D12RootSignature>> s_RootSignatureHashMap;

void RootSignature::DestroyAll()
{
	s_RootSignatureHashMap.clear();
}

void RootSignature::InitStaticSampler(UINT Register, const D3D12_SAMPLER_DESC& NonStaticSamplerDesc, D3D12_SHADER_VISIBILITY Visibility /* = D3D12_SHADER_VISIBILITY_ALL */)
{
	ASSERT(m_NumInitializedStaticSamplers < m_NumSamplers);

	D3D12_STATIC_SAMPLER_DESC& StaticSamplerDesc = m_SamplerArray[m_NumInitializedStaticSamplers++];

	StaticSamplerDesc.Filter = NonStaticSamplerDesc.Filter;
	StaticSamplerDesc.AddressU = NonStaticSamplerDesc.AddressU;
	StaticSamplerDesc.AddressV = NonStaticSamplerDesc.AddressV;
	StaticSamplerDesc.AddressW = NonStaticSamplerDesc.AddressW;
	StaticSamplerDesc.MipLODBias = NonStaticSamplerDesc.MipLODBias;
	StaticSamplerDesc.MaxAnisotropy = NonStaticSamplerDesc.MaxAnisotropy;
	StaticSamplerDesc.ComparisonFunc = NonStaticSamplerDesc.ComparisonFunc;
	StaticSamplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
	StaticSamplerDesc.MinLOD = NonStaticSamplerDesc.MinLOD;
	StaticSamplerDesc.MaxLOD = NonStaticSamplerDesc.MaxLOD;
	StaticSamplerDesc.ShaderRegister = Register;
	StaticSamplerDesc.RegisterSpace = 0;
	StaticSamplerDesc.ShaderVisibility = Visibility;

	if (StaticSamplerDesc.AddressU == D3D12_TEXTURE_ADDRESS_MODE_BORDER ||
		StaticSamplerDesc.AddressV == D3D12_TEXTURE_ADDRESS_MODE_BORDER ||
		StaticSamplerDesc.AddressW == D3D12_TEXTURE_ADDRESS_MODE_BORDER)
	{
		WARN_ONCE_IF_NOT(
			// Transparent Black
			NonStaticSamplerDesc.BorderColor[0] == 0.0f &&
			NonStaticSamplerDesc.BorderColor[1] == 0.0f &&
			NonStaticSamplerDesc.BorderColor[2] == 0.0f &&
			NonStaticSamplerDesc.BorderColor[3] == 0.0f ||
			// Opaque Black
			NonStaticSamplerDesc.BorderColor[0] == 0.0f &&
			NonStaticSamplerDesc.BorderColor[1] == 0.0f &&
			NonStaticSamplerDesc.BorderColor[2] == 0.0f &&
			NonStaticSamplerDesc.BorderColor[3] == 1.0f ||
			// Opaque White
			NonStaticSamplerDesc.BorderColor[0] == 1.0f &&
			NonStaticSamplerDesc.BorderColor[1] == 1.0f &&
			NonStaticSamplerDesc.BorderColor[2] == 1.0f &&
			NonStaticSamplerDesc.BorderColor[3] == 1.0f,
			"Sampler border color does not match static sampler limitations"
		
		);

		if (NonStaticSamplerDesc.BorderColor[3] = 1.0f)
		{
			if (NonStaticSamplerDesc.BorderColor[0] = 1.0f)
			{
				StaticSamplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
			}
			else
			{
				StaticSamplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
			}
		}
		else
		{
			StaticSamplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
		}
	}

}

void RootSignature::Finalize(const std::wstring& name, D3D12_ROOT_SIGNATURE_FLAGS Flags /* = D3D12_ROOT_SIGNATURE_FLAG_NONE */)
{
	if (m_Finalized)
	{
		return;
	}
}