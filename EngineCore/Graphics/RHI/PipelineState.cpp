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
	ZeroMemory(&m_PSODesc, sizeof(m_PSODesc));
	m_PSODesc.NodeMask = 1;
	m_PSODesc.SampleMask = 0xFFFFFFFFu;
	m_PSODesc.SampleDesc.Count = 1;
	m_PSODesc.InputLayout.NumElements = 0;
}

void GraphicPSO::SetBlendState(const D3D12_BLEND_DESC& BlendDesc)
{
	m_PSODesc.BlendState = BlendDesc;
}

void GraphicPSO::SetDepthStencilState(const D3D12_DEPTH_STENCIL_DESC& DepthStencilDesc)
{
	m_PSODesc.DepthStencilState = DepthStencilDesc;
}

void GraphicPSO::SetRasterizerState(const D3D12_RASTERIZER_DESC& RasterizerDesc)
{
	m_PSODesc.RasterizerState = RasterizerDesc;
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
		m_PSODesc.RTVFormats[i] = RTVFormat[i];
	}

	for (UINT i = NumRts; i < m_PSODesc.NumRenderTargets; ++i)
	{
		m_PSODesc.RTVFormats[i] = DXGI_FORMAT_UNKNOWN;
	}

	m_PSODesc.NumRenderTargets = NumRts;
	m_PSODesc.DSVFormat = DSRFormat;
	m_PSODesc.SampleDesc.Count = MsaaCount;
	m_PSODesc.SampleDesc.Quality = MsaaQuality;
}

void GraphicPSO::SetSampleMask(UINT SampleMask)
{
	m_PSODesc.SampleMask = SampleMask;
}


void GraphicPSO::SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE TopologyType)
{
	m_PSODesc.PrimitiveTopologyType = TopologyType;
}

void GraphicPSO::SetInputLayout(UINT NumElements, const D3D12_INPUT_ELEMENT_DESC* pInputElementDesc)
{
	m_PSODesc.InputLayout.NumElements = NumElements;
	if (NumElements > 0)
	{
		D3D12_INPUT_ELEMENT_DESC* NewElements = (D3D12_INPUT_ELEMENT_DESC*)malloc(sizeof(D3D12_INPUT_ELEMENT_DESC) * NumElements);
		memcpy(NewElements, pInputElementDesc, sizeof(D3D12_INPUT_ELEMENT_DESC) * NumElements);
		m_InputLayouts.reset((const D3D12_INPUT_ELEMENT_DESC*)NewElements);
	}
	else
	{
		m_InputLayouts = nullptr;
	}
}

void GraphicPSO::SetPrimitiveRestart(D3D12_INDEX_BUFFER_STRIP_CUT_VALUE IBProps)
{
	m_PSODesc.IBStripCutValue = IBProps;
}

void GraphicPSO::Finalize()
{
	// set root signature
	m_PSODesc.pRootSignature = m_RootSignature->GetSignature();
	ASSERT(m_PSODesc.pRootSignature != nullptr);

	//set input layout
	m_PSODesc.InputLayout.pInputElementDescs = nullptr;

	//get hash code
	size_t HashCode = Utility::HashState(&m_PSODesc);
	HashCode = Utility::HashState(m_InputLayouts.get(), m_PSODesc.InputLayout.NumElements, HashCode);

	m_PSODesc.InputLayout.pInputElementDescs - m_InputLayouts.get();

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
		ASSERT(m_PSODesc.DepthStencilState.DepthEnable != (m_PSODesc.DSVFormat == DXGI_FORMAT_UNKNOWN));
		ASSERT_SUCCEEDED(g_Device->CreateGraphicsPipelineState(&m_PSODesc, MY_IID_PPV_ARGS(&m_PSO)));

		s_GraphicsPSOHashMap[HashCode].Attach(m_PSO);
		m_PSO->SetName(m_Name);
	}
	else
	{
		while (*PSORef == nullptr)
		{
			this_thread::yield();
		}

		m_PSO = *PSORef;
	}
}