#include "../../pch.h"
#include "PipelineState.h"
#include "../GraphicsCore.h"
#include "../Misc/Utility.h"

using namespace Graphics;

static map<size_t, ComPtr<ID3D12PipelineState>> s_GraphicsPSOHashMap;
static map<size_t, ComPtr<ID3D12PipelineState>> s_ComputePSOHashMap;

void PSO::DestroyAll()
{
	s_ComputePSOHashMap.clear();
	s_GraphicsPSOHashMap.clear();
}

GraphicPSO::GraphicPSO(const wchar_t* Name /* = L"Unnamed Graphics PSO" */)
	:PSO(Name)
{
	ZeroMemory(&Y_PSODesc, sizeof(Y_PSODesc));
	Y_PSODesc.NodeMask = 1;
	Y_PSODesc.SampleMask = 0xFFFFFFFFu;
	Y_PSODesc.SampleDesc.Count = 1;
	Y_PSODesc.InputLayout.NumElements = 0;
}

void GraphicPSO::SetBlendState(const D3D12_BLEND_DESC& BlendDesc)
{
	Y_PSODesc.BlendState = BlendDesc;
}

void GraphicPSO::SetDepthStencilState(const D3D12_DEPTH_STENCIL_DESC& DepthStencilDesc)
{
	Y_PSODesc.DepthStencilState = DepthStencilDesc;
}

void GraphicPSO::SetRasterizerState(const D3D12_RASTERIZER_DESC& RasterizerDesc)
{
	Y_PSODesc.RasterizerState = RasterizerDesc;
}

void GraphicPSO::SetDepthTargetFormat(DXGI_FORMAT DSVFormat, UINT MsaaCount /* = 1 */, UINT MsaaQuality /* = 0 */)
{
	SetRenderTargetFormats(0, nullptr, DSVFormat, MsaaCount, MsaaQuality);
}

void GraphicPSO::SetRenderTargetFormat(DXGI_FORMAT RTVFormat, DXGI_FORMAT DSVFormat, UINT MsaaCount /* = 1 */, UINT MsaaQuality /* = 0 */)
{
	SetRenderTargetFormats(1, &RTVFormat, DSVFormat, MsaaCount, MsaaQuality);
}

void GraphicPSO::SetRenderTargetFormats(UINT NumRts, DXGI_FORMAT* RTVFormat, DXGI_FORMAT DSRFormat, UINT MsaaCount /* = 1 */, UINT MsaaQuality /* = 0 */)
{
	ASSERT(NumRts == 0 || RTVFormat != nullptr, "Numm format array confilicts with non-zero length");
	for (UINT i =0; i< NumRts; ++i)
	{
		ASSERT(RTVFormat[i] != DXGI_FORMAT_UNKNOWN);
		Y_PSODesc.RTVFormats[i] = RTVFormat[i];
	}

	for (UINT i = NumRts; i < Y_PSODesc.NumRenderTargets; ++i)
	{
		Y_PSODesc.RTVFormats[i] = DXGI_FORMAT_UNKNOWN;
	}

	Y_PSODesc.NumRenderTargets = NumRts;
	Y_PSODesc.DSVFormat = DSRFormat;
	Y_PSODesc.SampleDesc.Count = MsaaCount;
	Y_PSODesc.SampleDesc.Quality = MsaaQuality;
}

void GraphicPSO::SetSampleMask(UINT SampleMask)
{
	Y_PSODesc.SampleMask = SampleMask;
}


void GraphicPSO::SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE TopologyType)
{
	Y_PSODesc.PrimitiveTopologyType = TopologyType;
}

void GraphicPSO::SetInputLayout(UINT NumElements, const D3D12_INPUT_ELEMENT_DESC* pInputElementDesc)
{
	Y_PSODesc.InputLayout.NumElements = NumElements;
	if (NumElements > 0)
	{
		D3D12_INPUT_ELEMENT_DESC* NewElements = (D3D12_INPUT_ELEMENT_DESC*)malloc(sizeof(D3D12_INPUT_ELEMENT_DESC) * NumElements);
		memcpy(NewElements, pInputElementDesc, sizeof(D3D12_INPUT_ELEMENT_DESC) * NumElements);
		Y_InputLayouts.reset((const D3D12_INPUT_ELEMENT_DESC*)NewElements);
	}
	else
	{
		Y_InputLayouts = nullptr;
	}
}

void GraphicPSO::SetPrimitiveRestart(D3D12_INDEX_BUFFER_STRIP_CUT_VALUE IBProps)
{
	Y_PSODesc.IBStripCutValue = IBProps;
}

void GraphicPSO::Finalize()
{
	// set root signature
	Y_PSODesc.pRootSignature = Y_RootSignature->GetSignature();
	ASSERT(Y_PSODesc.pRootSignature != nullptr);

	//set input layout
	Y_PSODesc.InputLayout.pInputElementDescs = nullptr;

	//get hash code
	size_t HashCode = Utility::HashState(&Y_PSODesc);
	HashCode = Utility::HashState(Y_InputLayouts.get(), Y_PSODesc.InputLayout.NumElements, HashCode);

	Y_PSODesc.InputLayout.pInputElementDescs - Y_InputLayouts.get();

	//end for input layout

	ID3D12PipelineState** PSORef = nullptr;
	bool FirstCompile = false;

	{
		static mutex s_HashMapMutex;
		lock_guard<mutex> LockGuard(s_HashMapMutex);
		auto Iter = s_GraphicsPSOHashMap.find(HashCode);

		//if
		if (Iter == s_GraphicsPSOHashMap.end())
		{
			FirstCompile = true;
			PSORef = s_GraphicsPSOHashMap[HashCode].GetAddressOf();
		}
		else
		{
			PSORef = Iter->second.GetAddressOf();
		}
	}

	if (FirstCompile)
	{
		ASSERT(Y_PSODesc.DepthStencilState.DepthEnable != (Y_PSODesc.DSVFormat == DXGI_FORMAT_UNKNOWN));
		ASSERT_SUCCEEDED(g_Device->CreateGraphicsPipelineState(&Y_PSODesc, MY_IID_PPV_ARGS(&Y_PSO)));

		s_GraphicsPSOHashMap[HashCode].Attach(Y_PSO);
		Y_PSO->SetName(Y_Name);
	}
	else
	{
		while (*PSORef == nullptr)
		{
			this_thread::yield();
		}

		Y_PSO = *PSORef;
	}
}