#include "pch.h"
#include "RootSignature.h"
#include <map>
#include "../GraphicsCore.h"

using namespace Graphics;
using Microsoft::WRL::ComPtr;

static std::map<size_t, ComPtr<ID3D12RootSignature>> RootSignatureHashMap;

void FRootSignature::DestroyAll()
{
	RootSignatureHashMap.clear();
}

void FRootSignature::InitStaticSampler(UINT Register, const D3D12_SAMPLER_DESC& NonStaticSamplerDesc, D3D12_SHADER_VISIBILITY Visibility /* = D3D12_SHADER_VISIBILITY_ALL */)
{
	ASSERT(NumInitializedStaticSamplers < NumSamplers);

	D3D12_STATIC_SAMPLER_DESC& StaticSamplerDesc = SamplerArray[NumInitializedStaticSamplers++];

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

		if (NonStaticSamplerDesc.BorderColor[3] == 1.0f)
		{
			if (NonStaticSamplerDesc.BorderColor[0] == 1.0f)
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

void FRootSignature::Finalize(const std::wstring& Name, D3D12_ROOT_SIGNATURE_FLAGS Flags /* = D3D12_ROOT_SIGNATURE_FLAG_NONE */)
{
	if (Finalized)
	{
		return;
	}

	ASSERT(NumInitializedStaticSamplers == NumSamplers);

	D3D12_ROOT_SIGNATURE_DESC RootDesc;
	RootDesc.Flags = Flags;
	RootDesc.NumParameters = NumParameter;
	RootDesc.pParameters = (const D3D12_ROOT_PARAMETER*)ParamArray.get();
	RootDesc.NumStaticSamplers = NumSamplers;
	RootDesc.pStaticSamplers = (const D3D12_STATIC_SAMPLER_DESC*)SamplerArray.get();

	DescriptorTableBitMap = 0;
	SamplerTableBitMap = 0;

	size_t HashCode = Utility::HashState(&RootDesc.Flags);
	HashCode = Utility::HashState(RootDesc.pStaticSamplers, NumSamplers, HashCode);

	for (UINT Param = 0; Param < NumParameter; Param++)
	{
		const D3D12_ROOT_PARAMETER& RootParam = RootDesc.pParameters[Param];
		DescriptorTableSize[Param] = 0;

		if (RootParam.ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE)
		{
			ASSERT(RootParam.DescriptorTable.pDescriptorRanges != nullptr);
			HashCode = Utility::HashState(RootParam.DescriptorTable.pDescriptorRanges, RootParam.DescriptorTable.NumDescriptorRanges, HashCode);

			if (RootParam.DescriptorTable.pDescriptorRanges->RangeType == D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER)
			{
				SamplerTableBitMap |= (1 << Param);
			}
			else
			{
				DescriptorTableBitMap |= (1 << Param);
			}

			for (UINT TableRange = 0; TableRange < RootParam.DescriptorTable.NumDescriptorRanges; TableRange++)
			{
				DescriptorTableSize[Param] += RootParam.DescriptorTable.pDescriptorRanges[TableRange].NumDescriptors;
			}
		}
		else
		{
			HashCode = Utility::HashState(&RootParam, 1, HashCode);
		}
	}

	ID3D12RootSignature** RsRef = nullptr;
	bool FirstCompile = false;
	{
		static std::mutex s_HasMapMutex;
		std::lock_guard<std::mutex> CS(s_HasMapMutex);
		auto Iter = RootSignatureHashMap.find(HashCode);

		if (Iter == RootSignatureHashMap.end())
		{
			RsRef = RootSignatureHashMap[HashCode].GetAddressOf();
			FirstCompile = true;
		}
		else
		{
			RsRef = Iter->second.GetAddressOf();
		}
	}

	if (FirstCompile)
	{
		ComPtr<ID3DBlob> OutBlob, ErrorBlob;
		ASSERT_SUCCEEDED(D3D12SerializeRootSignature(&RootDesc, D3D_ROOT_SIGNATURE_VERSION_1,
			OutBlob.GetAddressOf(), ErrorBlob.GetAddressOf()));

		ASSERT_SUCCEEDED(Graphics::g_Device->CreateRootSignature(1, OutBlob->GetBufferPointer(), OutBlob->GetBufferSize(), MY_IID_PPV_ARGS(&Signature)));
		Signature->SetName(Name.c_str());
		RootSignatureHashMap[HashCode].Attach(Signature);
		ASSERT(*RsRef == Signature);
	}
	else
	{
		while (*RsRef == nullptr)
		{
			std::this_thread::yield();
		}

		Signature = *RsRef;
	}

	Finalized = TRUE;

}