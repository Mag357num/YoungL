#pragma once
#include "../../pch.h"

class RootSignature;

class PSO
{
public:
	PSO(const wchar_t* Name) : Y_Name(Name), Y_RootSignature(nullptr) {}

	static void DestroyAll(void);

	void SetRootSignature(const RootSignature& BindMappings)
	{
		Y_RootSignature = &BindMappings;
	}

	const RootSignature& GetRootSignature(void) const
	{
		ASSERT(Y_RootSignature != nullptr);
		return *Y_RootSignature;
	}

	ID3D12PipelineState* GetPipelineStateObject(void) const { return Y_PSO; }

protected:
	const wchar_t* Y_Name;
	const RootSignature* Y_RootSignature;
	ID3D12PipelineState* Y_PSO;
};

class GraphicPSO : public PSO
{
public:
	GraphicPSO(const wchar_t* Name = L"Unnamed Graphics PSO");

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

	void SetVertexShader(const void* Binary, size_t Size) { Y_PSODesc.VS = D3D12_SHADER_BYTECODE(const_cast<void*>(Binary), Size); }
	void SetPixelShader(const void* Binary, size_t Size) { Y_PSODesc.PS = D3D12_SHADER_BYTECODE(const_cast<void*>(Binary), Size); }
	void SetGeometryShader(const void* Binary, size_t Size) { Y_PSODesc.GS = D3D12_SHADER_BYTECODE(const_cast<void*>(Binary), Size); }
	void SetHullShader(const void* Binary, size_t Size) { Y_PSODesc.HS = D3D12_SHADER_BYTECODE(const_cast<void*>(Binary), Size); }
	void SetDomainShader(const void* Binary, size_t Size) { Y_PSODesc.DS = D3D12_SHADER_BYTECODE(const_cast<void*>(Binary), Size); }

	void SetVertexShader(const D3D12_SHADER_BYTECODE& ByteCode) { Y_PSODesc.VS = ByteCode; }
	void SetPixelShader(const D3D12_SHADER_BYTECODE& ByteCode) { Y_PSODesc.PS = ByteCode; }
	void SetGeometryShader(const D3D12_SHADER_BYTECODE& ByteCode) { Y_PSODesc.GS = ByteCode; }
	void SetHullShader(const D3D12_SHADER_BYTECODE& ByteCode) { Y_PSODesc.HS = ByteCode; }
	void SetDomainShader(const D3D12_SHADER_BYTECODE& ByteCode) { Y_PSODesc.DS = ByteCode; }

	void Finalize();

private:

	D3D12_GRAPHICS_PIPELINE_STATE_DESC Y_PSODesc;
	std::shared_ptr<const D3D12_INPUT_ELEMENT_DESC> Y_InputLayouts;

};
