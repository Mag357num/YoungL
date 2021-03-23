#pragma once
#include "../../pch.h"


class FRootParameter
{
public:
	FRootParameter() {
		RootParam.ParameterType = (D3D12_ROOT_PARAMETER_TYPE)0xFFFFFFFF;
	}

	~FRootParameter() {
		Clear();
	}

	void Clear()
	{
		if (RootParam.ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE)
		{
			delete[] RootParam.DescriptorTable.pDescriptorRanges;
		}

		RootParam.ParameterType = (D3D12_ROOT_PARAMETER_TYPE)0xFFFFFFFF;
	}

	void InitAsConstants(UINT Register, UINT NumDwords, D3D12_SHADER_VISIBILITY Visibility= D3D12_SHADER_VISIBILITY_ALL, UINT Space = 0)
	{
		RootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
		RootParam.ShaderVisibility = Visibility;
		RootParam.Constants.Num32BitValues = NumDwords;
		RootParam.Constants.ShaderRegister = Register;
		RootParam.Constants.RegisterSpace = Space;
	}

	void InitAsConstantBuffer(UINT Register, D3D12_SHADER_VISIBILITY Visibility= D3D12_SHADER_VISIBILITY_ALL, UINT Space = 0)
	{
		RootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		RootParam.ShaderVisibility = Visibility;
		RootParam.Descriptor.ShaderRegister = Register;
		RootParam.Descriptor.RegisterSpace = Space;

	}

	void InitAsBufferSRV(UINT Register, D3D12_SHADER_VISIBILITY Visibility=D3D12_SHADER_VISIBILITY_ALL, UINT Space = 0)
	{
		RootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
		RootParam.ShaderVisibility = Visibility;
		RootParam.Descriptor.ShaderRegister = Register;
		RootParam.Descriptor.RegisterSpace = Space;

	}

	void InitAsBuferUAV(UINT Register, D3D12_SHADER_VISIBILITY Visibility = D3D12_SHADER_VISIBILITY_ALL, UINT Space = 0)
	{
		RootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
		RootParam.ShaderVisibility = Visibility;
		RootParam.Descriptor.ShaderRegister = Register;
		RootParam.Descriptor.RegisterSpace = Space;
	}

	void InitAsDescritporRange(D3D12_DESCRIPTOR_RANGE_TYPE Type, UINT Register, UINT Count, D3D12_SHADER_VISIBILITY Visibility = D3D12_SHADER_VISIBILITY_ALL, UINT Space = 0)
	{
		InitAsDescriptorTable(1, Visibility);
		SetTableRange(0, Type, Register, Count, Space);
	}

	void InitAsDescriptorTable(UINT RangeCount, D3D12_SHADER_VISIBILITY Visibility = D3D12_SHADER_VISIBILITY_ALL)
	{
		RootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		RootParam.ShaderVisibility = Visibility;
		RootParam.DescriptorTable.NumDescriptorRanges = RangeCount;
		RootParam.DescriptorTable.pDescriptorRanges = new D3D12_DESCRIPTOR_RANGE[RangeCount];
	}

	void SetTableRange(UINT RangeIndex, D3D12_DESCRIPTOR_RANGE_TYPE RangeType, UINT Register, UINT Count, UINT Space = 0)
	{
		D3D12_DESCRIPTOR_RANGE* Range = const_cast<D3D12_DESCRIPTOR_RANGE*>(RootParam.DescriptorTable.pDescriptorRanges + RangeIndex);
		Range->RangeType = RangeType;
		Range->BaseShaderRegister = Register;
		Range->RegisterSpace = Space;
		Range->NumDescriptors = Count;
		Range->OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	}

	const D3D12_ROOT_PARAMETER& operator() (void) const { return RootParam; }

protected:
	D3D12_ROOT_PARAMETER RootParam;
};

class FRootSignature
{
public:
	FRootSignature(UINT NumrootParams = 0, UINT NumStaticSamplers = 0)
	{
		Reset(NumrootParams, NumStaticSamplers);
	}

	~FRootSignature()
	{

	}

	static void DestroyAll(void);

	void Reset(UINT NumRootParams, UINT NumStaticSamplers)
	{
		if (NumRootParams > 0)
		{
			ParamArray.reset(new FRootParameter[NumRootParams]);
		}
		else
		{
			ParamArray = nullptr;
		}

		if (NumStaticSamplers > 0)
		{
			SamplerArray.reset(new D3D12_STATIC_SAMPLER_DESC[NumStaticSamplers]);
		}
		else
		{
			SamplerArray = nullptr;
		}

		NumSamplers = NumStaticSamplers;
		NumInitializedStaticSamplers = 0;
	}

	FRootParameter& operator[] (size_t EntryIndex)
	{
		ASSERT(EntryIndex < NumParameter);
		return ParamArray.get()[EntryIndex];
	}

	const FRootParameter& operator[] (size_t EntryIndex) const
	{
		ASSERT(EntryIndex < NumParameter);
		return ParamArray.get()[EntryIndex];
	}

	void InitStaticSampler(UINT Register, const D3D12_SAMPLER_DESC& NonStaticSamplerDesc,
		D3D12_SHADER_VISIBILITY Visibility = D3D12_SHADER_VISIBILITY_ALL);
	void Finalize(const std::wstring& Name, D3D12_ROOT_SIGNATURE_FLAGS Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE);

	ID3D12RootSignature* GetSignature() const { return Signature; }

protected:

	BOOL Finalized;
	UINT NumParameter;
	UINT NumSamplers;
	UINT NumInitializedStaticSamplers;
	uint32_t DescriptorTableBitMap;
	uint32_t SamplerTableBitMap;
	uint32_t DescriptorTableSize[16];
	
	std::unique_ptr<FRootParameter[]> ParamArray;
	std::unique_ptr<D3D12_STATIC_SAMPLER_DESC[]> SamplerArray;
	ID3D12RootSignature* Signature;
};