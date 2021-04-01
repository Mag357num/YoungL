#include "pch.h"
#include "PipelineState.h"
#include "../GraphicsCore.h"
#include "../../Misc/Utility.h"
#include <map>

using namespace Graphics;
using namespace std;

static map<size_t, ComPtr<ID3D12PipelineState>> GraphicsPSOHashMap;
static map<size_t, ComPtr<ID3D12PipelineState>> ComputePSOHashMap;

void FPSO::DestroyAll()
{
	ComputePSOHashMap.clear();
	GraphicsPSOHashMap.clear();
}

FGraphicPSO::FGraphicPSO(const wchar_t* Name /* = L"Unnamed Graphics PSO" */)
	:FPSO(Name)
{
	ZeroMemory(&PSODesc, sizeof(PSODesc));
	PSODesc.NodeMask = 1;
	PSODesc.SampleMask = 0xFFFFFFFFu;
	PSODesc.SampleDesc.Count = 1;
	PSODesc.InputLayout.NumElements = 0;
}

void FGraphicPSO::SetBlendState(const D3D12_BLEND_DESC& BlendDesc)
{
	PSODesc.BlendState = BlendDesc;
}

void FGraphicPSO::SetDepthStencilState(const D3D12_DEPTH_STENCIL_DESC& DepthStencilDesc)
{
	PSODesc.DepthStencilState = DepthStencilDesc;
}

void FGraphicPSO::SetRasterizerState(const D3D12_RASTERIZER_DESC& RasterizerDesc)
{
	PSODesc.RasterizerState = RasterizerDesc;
}

void FGraphicPSO::SetDepthTargetFormat(DXGI_FORMAT DSVFormat, UINT MsaaCount /* = 1 */, UINT MsaaQuality /* = 0 */)
{
	SetRenderTargetFormats(0, nullptr, DSVFormat, MsaaCount, MsaaQuality);
}

void FGraphicPSO::SetRenderTargetFormat(DXGI_FORMAT RTVFormat, DXGI_FORMAT DSVFormat, UINT MsaaCount /* = 1 */, UINT MsaaQuality /* = 0 */)
{
	SetRenderTargetFormats(1, &RTVFormat, DSVFormat, MsaaCount, MsaaQuality);
}

void FGraphicPSO::SetRenderTargetFormats(UINT NumRts, DXGI_FORMAT* RTVFormat, DXGI_FORMAT DSRFormat, UINT MsaaCount /* = 1 */, UINT MsaaQuality /* = 0 */)
{
	ASSERT(NumRts == 0 || RTVFormat != nullptr, "Numm format array confilicts with non-zero length");
	for (UINT i =0; i< NumRts; ++i)
	{
		ASSERT(RTVFormat[i] != DXGI_FORMAT_UNKNOWN);
		PSODesc.RTVFormats[i] = RTVFormat[i];
	}

	for (UINT i = NumRts; i < PSODesc.NumRenderTargets; ++i)
	{
		PSODesc.RTVFormats[i] = DXGI_FORMAT_UNKNOWN;
	}

	PSODesc.NumRenderTargets = NumRts;
	PSODesc.DSVFormat = DSRFormat;
	PSODesc.SampleDesc.Count = MsaaCount;
	PSODesc.SampleDesc.Quality = MsaaQuality;
}

void FGraphicPSO::SetSampleMask(UINT SampleMask)
{
	PSODesc.SampleMask = SampleMask;
}


void FGraphicPSO::SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE TopologyType)
{
	PSODesc.PrimitiveTopologyType = TopologyType;
}

void FGraphicPSO::SetInputLayout(UINT NumElements, const D3D12_INPUT_ELEMENT_DESC* pInputElementDesc)
{
	PSODesc.InputLayout.NumElements = NumElements;
	if (NumElements > 0)
	{
		D3D12_INPUT_ELEMENT_DESC* NewElements = (D3D12_INPUT_ELEMENT_DESC*)malloc(sizeof(D3D12_INPUT_ELEMENT_DESC) * NumElements);
		memcpy(NewElements, pInputElementDesc, sizeof(D3D12_INPUT_ELEMENT_DESC) * NumElements);
		InputLayouts.reset((const D3D12_INPUT_ELEMENT_DESC*)NewElements);
	}
	else
	{
		InputLayouts = nullptr;
	}
}

void FGraphicPSO::SetPrimitiveRestart(D3D12_INDEX_BUFFER_STRIP_CUT_VALUE IBProps)
{
	PSODesc.IBStripCutValue = IBProps;
}

void FGraphicPSO::Finalize()
{
	// set root signature
	PSODesc.pRootSignature = RootSignature->GetSignature();
	ASSERT(PSODesc.pRootSignature != nullptr);

	//set input layout
	PSODesc.InputLayout.pInputElementDescs = nullptr;

	//get hash code
	size_t HashCode = Utility::HashState(&PSODesc);
	HashCode = Utility::HashState(InputLayouts.get(), PSODesc.InputLayout.NumElements, HashCode);

	PSODesc.InputLayout.pInputElementDescs - InputLayouts.get();

	//end for input layout

	ID3D12PipelineState** PSORef = nullptr;
	bool FirstCompile = false;

	{
		static std::mutex s_HashMapMutex;
		std::lock_guard<std::mutex> LockGuard(s_HashMapMutex);
		auto Iter = GraphicsPSOHashMap.find(HashCode);

		//if
		if (Iter == GraphicsPSOHashMap.end())
		{
			FirstCompile = true;
			PSORef = GraphicsPSOHashMap[HashCode].GetAddressOf();
		}
		else
		{
			PSORef = Iter->second.GetAddressOf();
		}
	}

	if (FirstCompile)
	{
		ASSERT(PSODesc.DepthStencilState.DepthEnable != (PSODesc.DSVFormat == DXGI_FORMAT_UNKNOWN));
		ASSERT_SUCCEEDED(g_Device->CreateGraphicsPipelineState(&PSODesc, IID_PPV_ARGS(&PSO)));

		GraphicsPSOHashMap[HashCode].Attach(PSO);
		PSO->SetName(Name);
	}
	else
	{
		while (*PSORef == nullptr)
		{
			this_thread::yield();
		}

		PSO = *PSORef;
	}
}