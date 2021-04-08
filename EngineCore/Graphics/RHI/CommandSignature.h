#pragma once

#include "../../pch.h"

class FRootSignature;

class FIndirectParam
{
public:
	FIndirectParam() { 
		IndirectParam.Type = (D3D12_INDIRECT_ARGUMENT_TYPE)0xFFFFFFFF; 
	}

	void Draw(void)
	{
		IndirectParam.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW;
	}

	void DrawIndex(void)
	{
		IndirectParam.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;
	}

	void Dispatch(void)
	{
		IndirectParam.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH;
	}

	void VertexBufferView(UINT Slot)
	{
		IndirectParam.Type = D3D12_INDIRECT_ARGUMENT_TYPE_VERTEX_BUFFER_VIEW;
		IndirectParam.VertexBuffer.Slot = Slot;
	}

	void IndexBufferView(void)
	{
		IndirectParam.Type = D3D12_INDIRECT_ARGUMENT_TYPE_INDEX_BUFFER_VIEW;
		
	}

	void Constant(UINT RootParamterIndex, UINT DestOffsetIn32BitValues, UINT Num32BitValuesToSet)
	{
		IndirectParam.Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT;
		IndirectParam.Constant.RootParameterIndex = RootParamterIndex;
		IndirectParam.Constant.DestOffsetIn32BitValues = DestOffsetIn32BitValues;
		IndirectParam.Constant.Num32BitValuesToSet = Num32BitValuesToSet;
	}

	void ConstantBufferView(UINT RootParameterIndex)
	{
		IndirectParam.Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT_BUFFER_VIEW;
		IndirectParam.ConstantBufferView.RootParameterIndex = RootParameterIndex;
	}

	void ShaderResourceView(UINT RootParameterIndex)
	{
		IndirectParam.Type = D3D12_INDIRECT_ARGUMENT_TYPE_SHADER_RESOURCE_VIEW;
		IndirectParam.ShaderResourceView.RootParameterIndex = RootParameterIndex;
	}

	void UnOrderedAccessView(UINT RootParameterIndex)
	{
		IndirectParam.Type = D3D12_INDIRECT_ARGUMENT_TYPE_UNORDERED_ACCESS_VIEW;
		IndirectParam.UnorderedAccessView.RootParameterIndex = RootParameterIndex;
	}

	D3D12_INDIRECT_ARGUMENT_DESC& GetDesc(void) { return IndirectParam; }

protected:
	D3D12_INDIRECT_ARGUMENT_DESC IndirectParam;
};

class FCommandSignature
{
public:
	FCommandSignature(UINT NumParams = 0)
		:Finalized(FALSE),
		NumParamters(NumParams)
	{
		Reset(NumParams);
	}

	void Reset(UINT NumParams)
	{
		if (NumParams > 0)
		{
			ParamArray.reset(new FIndirectParam[NumParams]);
		}
		else
			ParamArray = nullptr;

		NumParamters = NumParams;
	}

	void Destroy(void)
	{
		Signature = nullptr;
		ParamArray = nullptr;
	}

	FIndirectParam& operator[] (size_t EntryIndex)
	{
		ASSERT(EntryIndex < NumParamters);
		return ParamArray.get()[EntryIndex];

	}

	const FIndirectParam& operator[] (size_t EntryIndex) const
	{
		ASSERT(EntryIndex < NumParamters);
		return ParamArray.get()[EntryIndex];
	}

	void Finalize(const FRootSignature* RootSignature = nullptr);

	ID3D12CommandSignature* GetSignature() { return Signature.Get(); }

protected:
	BOOL Finalized;
	UINT NumParamters;
	std::unique_ptr<FIndirectParam[]> ParamArray;
	Microsoft::WRL::ComPtr<ID3D12CommandSignature> Signature;

};
