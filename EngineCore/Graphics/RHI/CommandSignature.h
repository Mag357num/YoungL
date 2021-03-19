#pragma once

#include "../../pch.h"

class RootSignature;

class IndirectParam
{
public:
	IndirectParam() { Y_IndirectParam.Type = (D3D12_INDIRECT_ARGUMENT_DESC)0xFFFFFFFF; }

	void Draw(void)
	{
		Y_IndirectParam.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW;
	}

	void DrawIndex(void)
	{
		Y_IndirectParam.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;
	}

	void Dispatch(void)
	{
		Y_IndirectParam.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH;
	}

	void VertexBufferView(UINT Slot)
	{
		Y_IndirectParam.Type = D3D12_INDIRECT_ARGUMENT_TYPE_VERTEX_BUFFER_VIEW;
		Y_IndirectParam.VertexBuffer.Slot = Slot;
	}

	void IndexBuferView(UINT Slot)
	{
		Y_IndirectParam.Type = D3D12_INDIRECT_ARGUMENT_TYPE_INDEX_BUFFER_VIEW;
		
	}

	void Constant(UINT RootParamterIndex, UINT DestOffsetIn32BitValues, UINT Num32BitValuesToSet)
	{
		Y_IndirectParam.Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT;
		Y_IndirectParam.Constant.RootParameterIndex = RootParamterIndex;
		Y_IndirectParam.Constant.DestOffsetIn32BitValues = DestOffsetIn32BitValues;
		Y_IndirectParam.Constant.Num32BitValuesToSet = Num32BitValuesToSet;
	}

	void ConstantBufferView(UINT RootParameterIndex)
	{
		Y_IndirectParam.Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT_BUFFER_VIEW;
		Y_IndirectParam.ConstantBufferView.RootParameterIndex = RootParameterIndex;
	}

	void ShaderResourceView(UINT RootParameterIndex)
	{
		Y_IndirectParam.Type = D3D12_INDIRECT_ARGUMENT_TYPE_SHADER_RESOURCE_VIEW;
		Y_IndirectParam.ShaderResourceView.RootParameterIndex = RootParameterIndex;
	}

	void UnOrderedAccessView(UINT RootParameterIndex)
	{
		Y_IndirectParam.Type = D3D12_INDIRECT_ARGUMENT_TYPE_UNORDERED_ACCESS_VIEW;
		Y_IndirectParam.UnorderedAccessView.RootParameterIndex = RootParameterIndex;
	}

	D3D12_INDIRECT_ARGUMENT_DESC& GetDesc(void) { return Y_IndirectParam; }

protected:
	D3D12_INDIRECT_ARGUMENT_DESC Y_IndirectParam;
};

class CommandSignature
{
public:
	CommandSignature(UINT NumParams = 0)
		:Y_Finalized(FALSE),
		Y_NumParamters(NumParams)
	{
		Reset(NumParams);
	}

	void Reset(UINT NumParams)
	{
		if (NumParams > 0)
		{
			Y_ParamArray.reset(new IndirectParam[NumParams]);
		}
		else
			Y_ParamArray = nullptr;

		Y_NumParamters = NumParams;
	}

	void Destroy(void)
	{
		Y_Signature = nullptr;
		Y_ParamArray = nullptr;
	}

	IndirectParam& operator[] (size_t EntryIndex)
	{
		ASSERT(EntryIndex < Y_NumParamters);
		return Y_ParamArray.get()[EntryIndex];

	}

	const IndirectParam& operator[] (size_t EntryIndex) const
	{
		ASSERT(EntryIndex < Y_NumParamters);
		return Y_ParamArray.get()[EntryIndex];
	}

	void Finalize(const RootSignature* RootSignature = nullptr);

	ID3D12CommandSignature* GetSignature() { return Y_Signature.Get(); }

protected:
	BOOL Y_Finalized;
	UINT Y_NumParamters;
	std::unique_ptr<IndirectParam[]> Y_ParamArray;
	Microsoft::WRL::ComPtr<ID3D12CommandSignature> Y_Signature;

};
