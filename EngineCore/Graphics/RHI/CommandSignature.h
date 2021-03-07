#pragma once

#include "../../pch.h"

class RootSignature;

class IndirectParam
{
public:
	IndirectParam() { m_IndirectParam.Type = (D3D12_INDIRECT_ARGUMENT_DESC)0xFFFFFFFF; }

	void Draw(void)
	{
		m_IndirectParam.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW;
	}

	void DrawIndex(void)
	{
		m_IndirectParam.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;
	}

	void Dispatch(void)
	{
		m_IndirectParam.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH;
	}

	void VertexBufferView(UINT Slot)
	{
		m_IndirectParam.Type = D3D12_INDIRECT_ARGUMENT_TYPE_VERTEX_BUFFER_VIEW;
		m_IndirectParam.VertexBuffer.Slot = Slot;
	}

	void IndexBuferView(UINT Slot)
	{
		m_IndirectParam.Type = D3D12_INDIRECT_ARGUMENT_TYPE_INDEX_BUFFER_VIEW;
		
	}

	void Constant(UINT RootParamterIndex, UINT DestOffsetIn32BitValues, UINT Num32BitValuesToSet)
	{
		m_IndirectParam.Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT;
		m_IndirectParam.Constant.RootParameterIndex = RootParamterIndex;
		m_IndirectParam.Constant.DestOffsetIn32BitValues = DestOffsetIn32BitValues;
		m_IndirectParam.Constant.Num32BitValuesToSet = Num32BitValuesToSet;
	}

	void ConstantBufferView(UINT RootParameterIndex)
	{
		m_IndirectParam.Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT_BUFFER_VIEW;
		m_IndirectParam.ConstantBufferView.RootParameterIndex = RootParameterIndex;
	}

	void ShaderResourceView(UINT RootParameterIndex)
	{
		m_IndirectParam.Type = D3D12_INDIRECT_ARGUMENT_TYPE_SHADER_RESOURCE_VIEW;
		m_IndirectParam.ShaderResourceView.RootParameterIndex = RootParameterIndex;
	}

	void UnOrderedAccessView(UINT RootParameterIndex)
	{
		m_IndirectParam.Type = D3D12_INDIRECT_ARGUMENT_TYPE_UNORDERED_ACCESS_VIEW;
		m_IndirectParam.UnorderedAccessView.RootParameterIndex = RootParameterIndex;
	}

	D3D12_INDIRECT_ARGUMENT_DESC& GetDesc(void) { return m_IndirectParam; }

protected:
	D3D12_INDIRECT_ARGUMENT_DESC m_IndirectParam;
};

class CommandSignature
{
public:
	CommandSignature(UINT NumParams = 0)
		:m_Finalized(FALSE),
		m_NumParamters(NumParams)
	{
		Reset(NumParams);
	}

	void Reset(UINT NumParams)
	{
		if (NumParams > 0)
		{
			m_ParamArray.reset(new IndirectParam[NumParams]);
		}
		else
			m_ParamArray = nullptr;

		m_NumParamters = NumParams;
	}

	void Destroy(void)
	{
		m_Signature = nullptr;
		m_ParamArray = nullptr;
	}

	IndirectParam& operator[] (size_t EntryIndex)
	{
		ASSERT(EntryIndex < m_NumParamters);
		return m_ParamArray.get()[EntryIndex];

	}

	const IndirectParam& operator[] (size_t EntryIndex) const
	{
		ASSERT(EntryIndex < m_NumParamters);
		return m_ParamArray.get()[EntryIndex];
	}

	void Finalize(const RootSignature* RootSignature = nullptr);

	ID3D12CommandSignature* GetSignature() { return m_Signature.Get(); }

protected:
	BOOL m_Finalized;
	UINT m_NumParamters;
	std::unique_ptr<IndirectParam[]> m_ParamArray;
	Microsoft::WRL::ComPtr<ID3D12CommandSignature> m_Signature;

};
