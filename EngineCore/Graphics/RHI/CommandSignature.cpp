#include "../../pch.h"
#include "CommandSignature.h"
#include "RootSignature.h"

#include "../GraphicsCore.h"

using namespace Graphics;

void CommandSignature::Finalize(const RootSignature* RootSignature /* = nullptr */)
{
	if (Y_Finalized)
	{
		return;
	}

	UINT ByteStride = 0;
	bool RequiredRootSignature = false;

	for (UINT i = 0; i < Y_NumParamters; i++)
	{
		switch (Y_ParamArray[i].GetDesc().Type)
		{
		case D3D12_INDIRECT_ARGUMENT_TYPE::D3D12_INDIRECT_ARGUMENT_TYPE_DRAW:
			ByteStride += sizeof(D3D12_DRAW_ARGUMENTS);
			break;

		case D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED:
			ByteStride += sizeof(D3D12_DRAW_INDEXED_ARGUMENTS);
			break;

		case D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH:
			ByteStride += sizeof(D3D12_DISPATCH_ARGUMENTS);
			break;

		case D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT:
			ByteStride += Y_ParamArray[i].GetDesc().Constant.Num32BitValuesToSet * 4;
			break;

		case D3D12_INDIRECT_ARGUMENT_TYPE_VERTEX_BUFFER_VIEW:
			ByteStride += sizeof(D3D12_VERTEX_BUFFER_VIEW);
			break;

		case D3D12_INDIRECT_ARGUMENT_TYPE_INDEX_BUFFER_VIEW:
			ByteStride += sizeof(D3D12_DRAW_INDEXED_ARGUMENTS);
			break;

		case D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT_BUFFER_VIEW:
		case D3D12_INDIRECT_ARGUMENT_TYPE_SHADER_RESOURCE_VIEW:
		case D3D12_INDIRECT_ARGUMENT_TYPE_UNORDERED_ACCESS_VIEW:
			ByteStride += 8;
			RequiredRootSignature = true;
			break;

		}
	}

	D3D12_COMMAND_SIGNATURE_DESC CommandSignatureDesc;
	CommandSignatureDesc.ByteStride = ByteStride;
	CommandSignatureDesc.NumArgumentDescs = Y_NumParamters;
	CommandSignatureDesc.pArgumentDescs = (const D3D12_INDIRECT_ARGUMENT_DESC *)Y_ParamArray.get();
	CommandSignatureDesc.NodeMask = 1;

	Microsoft::WRL::ComPtr<ID3DBlob> pOutBlob, pErrorBlob;
	ID3D12RootSignature* pRootSig = RootSignature ? RootSignature->GetSignature() : nullptr;
	if (RequiredRootSignature)
	{
		ASSERT(pRootSig != nullptr);
	}
	else
	{
		pRootSig = nullptr;
	}

	ASSERT_SUCCEEDED(g_Device->CreateCommandSignature(&CommandSignatureDesc, pRootSig, MY_IID_PPV_ARGS(&Y_Signature)));
	Y_Signature->SetName(L"CommandSignature");
	Y_Finalized = TRUE;

}