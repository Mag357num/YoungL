#pragma once
#include "../../pch.h"


class RootParameter
{
public:
	RootParameter() {
		Y_RootParam.ParameterType = (D3D12_ROOT_PARAMETER_TYPE)0xFFFFFFFF;
	}

	~RootParameter() {
		Clear();
	}

	void Clear()
	{
		if (Y_RootParam.ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE)
		{
			delete[] Y_RootParam.DescriptorTable.pDescriptorRanges;
		}

		Y_RootParam.ParameterType = (D3D12_ROOT_PARAMETER_TYPE)0xFFFFFFFF;
	}

	void InitAsConstants(UINT Register, UINT NumDwords, D3D12_SHADER_VISIBILITY Visibility= D3D12_SHADER_VISIBILITY_ALL, UINT Space = 0)
	{
		Y_RootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
		Y_RootParam.ShaderVisibility = Visibility;
		Y_RootParam.Constants.Num32BitValues = NumDwords;
		Y_RootParam.Constants.ShaderRegister = Register;
		Y_RootParam.Constants.RegisterSpace = Space;
	}

	void InitAsConstantBuffer(UINT Register, D3D12_SHADER_VISIBILITY Visibility= D3D12_SHADER_VISIBILITY_ALL, UINT Space = 0)
	{
		Y_RootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		Y_RootParam.ShaderVisibility = Visibility;
		Y_RootParam.Descriptor.ShaderRegister = Register;
		Y_RootParam.Descriptor.RegisterSpace = Space;

	}

	void InitAsBufferSRV(UINT Register, D3D12_SHADER_VISIBILITY Visibility D3D12_SHADER_VISIBILITY_ALL, UINT Space = 0)
	{
		Y_RootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
		Y_RootParam.ShaderVisibility = Visibility;
		Y_RootParam.Descriptor.ShaderRegister = Register;
		Y_RootParam.Descriptor.RegisterSpace = Space;

	}

	void InitAsBuferUAV(UINT Register, D3D12_SHADER_VISIBILITY Visibility = D3D12_SHADER_VISIBILITY_ALL, UINT Space = 0)
	{
		Y_RootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
		Y_RootParam.ShaderVisibility = Visibility;
		Y_RootParam.Descriptor.ShaderRegister = Register;
		Y_RootParam.Descriptor.RegisterSpace = Space;
	}

	void InitAsDescritporRange(D3D12_DESCRIPTOR_RANGE_TYPE Type, UINT Register, UINT Count, D3D12_SHADER_VISIBILITY Visibility = D3D12_SHADER_VISIBILITY_ALL, UINT Space = 0)
	{
		InitAsDescriptorTable(1, Visibility);
		SetTableRange(0, Type, Register, Count, Space);
	}

	void InitAsDescriptorTable(UINT RangeCount, D3D12_SHADER_VISIBILITY Visibility = D3D12_SHADER_VISIBILITY_ALL)
	{
		Y_RootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		Y_RootParam.ShaderVisibility = Visibility;
		Y_RootParam.DescriptorTable.NumDescriptorRanges = RangeCount;
		Y_RootParam.DescriptorTable.pDescriptorRanges = new D3D12_DESCRIPTOR_RANGE[RangeCount];
	}

	void SetTableRange(UINT RangeIndex, D3D12_DESCRIPTOR_RANGE_TYPE RangeType, UINT Register, UINT Count, UINT Space = 0)
	{
		D3D12_DESCRIPTOR_RANGE* Range = const_cast<D3D12_DESCRIPTOR_RANGE*>(Y_RootParam.DescriptorTable.pDescriptorRanges + RangeIndex);
		Range->RangeType = RangeType;
		Range->BaseShaderRegister = Register;
		Range->RegisterSpace = Space;
		Range->NumDescriptors = Count;
		Range->OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	}

	const D3D12_ROOT_PARAMETER& operator() (void) const { return Y_RootParam; }

protected:
	D3D12_ROOT_PARAMETER Y_RootParam;
};

class RootSignature
{
public:
	RootSignature(UINT NumrootParams = 0, UINT NumStaticSamplers = 0)
	{
		Reset(NumrootParams, NumStaticSamplers);
	}

	~RootSignature()
	{

	}

	static void DestroyAll(void);

	void Reset(UINT NumRootParams, UINT NumStaticSamplers)
	{
		if (NumRootParams > 0)
		{
			Y_ParamArray.reset(new RootParameter[NumRootParams]);
		}
		else
		{
			Y_ParamArray = nullptr;
		}

		if (NumStaticSamplers > 0)
		{
			Y_SamplerArray.reset(new D3D12_STATIC_SAMPLER_DESC[NumStaticSamplers]);
		}
		else
		{
			Y_SamplerArray = nullptr;
		}

		Y_NumSamplers = NumStaticSamplers;
		Y_NumInitializedStaticSamplers = 0;
	}

	RootParameter& operator[] (size_t EntryIndex)
	{
		ASSERT(EntryIndex < Y_NumParameter);
		return Y_ParamArray.get()[EntryIndex];
	}

	const RootParameter& operator[] (size_t EntryIndex) const
	{
		ASSERT(EntryIndex < Y_NumParameter);
		return Y_ParamArray.get()[EntryIndex];
	}

	void InitStaticSampler(UINT Register, const D3D12_SAMPLER_DESC& NonStaticSamplerDesc,
		D3D12_SHADER_VISIBILITY Visibility = D3D12_SHADER_VISIBILITY_ALL);
	void Finalize(const std::wstring& Name, D3D12_ROOT_SIGNATURE_FLAGS Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE);

	ID3D12RootSignature* GetSignature() const { return Y_Signature; }

protected:

	BOOL Y_Finalized;
	UINT Y_NumParameter;
	UINT Y_NumSamplers;
	UINT Y_NumInitializedStaticSamplers;
	uint32_t Y_DescriptorTableBitMap;
	uint32_t Y_SamplerTableBitMap;
	uint32_t Y_DescriptorTableSize[16];
	
	std::unique_ptr<RootParameter[]> Y_ParamArray;
	std::unique_ptr<D3D12_STATIC_SAMPLER_DESC[]> Y_SamplerArray;
	ID3D12RootSignature* Y_Signature;
};