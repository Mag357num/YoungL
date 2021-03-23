#pragma once
#include "../../pch.h"
#include "RootSignature.h"
class FRootSignature;

class FPSO
{
public:
	FPSO(const wchar_t* Name) : Name(Name), RootSignature(nullptr) {}

	static void DestroyAll(void);

	void SetRootSignature(const FRootSignature& BindMappings)
	{
		RootSignature = &BindMappings;
	}

	const FRootSignature& GetRootSignature(void) const
	{
		ASSERT(RootSignature != nullptr);
		return *RootSignature;
	}

	ID3D12PipelineState* GetPipelineStateObject(void) const { return PSO; }

protected:
	const wchar_t* Name;
	const FRootSignature* RootSignature;
	ID3D12PipelineState* PSO;
};

class FGraphicPSO : public FPSO
{
public:
	FGraphicPSO(const wchar_t* Name = L"Unnamed Graphics PSO");

	void SetBlendState(const D3D12_BLEND_DESC& BlendDesc);
	void SetRasterizerState(const D3D12_RASTERIZER_DESC& RasterizerDesc);
	void SetDepthStencilState(const D3D12_DEPTH_STENCIL_DESC& DepthStencilDesc);
	void SetSampleMask(UINT SampleMask);
	void SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE TopologyType);
	void SetDepthTargetFormat(DXGI_FORMAT DSVFormat, UINT MsaaCount = 1, UINT MsaaQuality = 0);
	void SetRenderTargetFormat(DXGI_FORMAT RTVFormat, DXGI_FORMAT DSVFormat, UINT MsaaCount = 1, UINT MsaaQuality = 0);
	void SetRenderTargetFormats(UINT NumRts, DXGI_FORMAT* RTVFormat, DXGI_FORMAT DSRFormat, UINT MsaaCount = 1, UINT MsaaQuality = 0);
	void SetInputLayout(UINT NumElements, const D3D12_INPUT_ELEMENT_DESC* pInputElementDesc);
	void SetPrimitiveRestart(D3D12_INDEX_BUFFER_STRIP_CUT_VALUE IBProps);

	void SetVertexShader(const void* Binary, size_t Size) { PSODesc.VS = CD3DX12_SHADER_BYTECODE(const_cast<void*>(Binary), Size); }
	void SetPixelShader(const void* Binary, size_t Size) { PSODesc.PS = CD3DX12_SHADER_BYTECODE(const_cast<void*>(Binary), Size); }
	void SetGeometryShader(const void* Binary, size_t Size) { PSODesc.GS = CD3DX12_SHADER_BYTECODE(const_cast<void*>(Binary), Size); }
	void SetHullShader(const void* Binary, size_t Size) { PSODesc.HS = CD3DX12_SHADER_BYTECODE(const_cast<void*>(Binary), Size); }
	void SetDomainShader(const void* Binary, size_t Size) { PSODesc.DS = CD3DX12_SHADER_BYTECODE(const_cast<void*>(Binary), Size); }

	void SetVertexShader(const D3D12_SHADER_BYTECODE& ByteCode) { PSODesc.VS = ByteCode; }
	void SetPixelShader(const D3D12_SHADER_BYTECODE& ByteCode) { PSODesc.PS = ByteCode; }
	void SetGeometryShader(const D3D12_SHADER_BYTECODE& ByteCode) { PSODesc.GS = ByteCode; }
	void SetHullShader(const D3D12_SHADER_BYTECODE& ByteCode) { PSODesc.HS = ByteCode; }
	void SetDomainShader(const D3D12_SHADER_BYTECODE& ByteCode) { PSODesc.DS = ByteCode; }

	void Finalize();

private:

	D3D12_GRAPHICS_PIPELINE_STATE_DESC PSODesc;
	std::shared_ptr<const D3D12_INPUT_ELEMENT_DESC> InputLayouts;

};
