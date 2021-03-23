#include "../../pch.h"
#include "CommandSignature.h"
#include "RootSignature.h"

#include "../GraphicsCore.h"

using namespace Graphics;

void FCommandSignature::Finalize(const FRootSignature* RootSignature /* = nullptr */)
{
	if (Finalized)
	{
		return;
	}

	UINT ByteStride = 0;
	bool RequiredRootSignature = false;

	for (UINT i = 0; i < NumParamters; i++)
	{
		switch (ParamArray[i].GetDesc().Type)
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
			ByteStride += ParamArray[i].GetDesc().Constant.Num32BitValuesToSet * 4;
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
	CommandSignatureDesc.NumArgumentDescs = NumParamters;
	CommandSignatureDesc.pArgumentDescs = (const D3D12_INDIRECT_ARGUMENT_DESC *)ParamArray.get();
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

	ASSERT_SUCCEEDED(g_Device->CreateCommandSignature(&CommandSignatureDesc, pRootSig, MY_IID_PPV_ARGS(&Signature)));
	Signature->SetName(L"CommandSignature");
	Finalized = TRUE;

}