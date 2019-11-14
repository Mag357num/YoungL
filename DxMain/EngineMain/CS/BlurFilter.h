#pragma once
#include "../Common/d3dUtil.h"

class BlurFilter
{
public:
	BlurFilter(ID3D12Device* device, UINT Width, UINT Height, DXGI_FORMAT format);
	BlurFilter(const BlurFilter& rhs) = delete;
	BlurFilter& operator=(const BlurFilter& rhs) = delete;
	~BlurFilter() = default;

	ID3D12Resource* Output();

	void BuildDescriptors(
		CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle,
		CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle,
		UINT descriptorSize
	);

	void OnResize(UINT newWidth, UINT newHeight);

	void Excute(
	ID3D12GraphicsCommandList* cmdList,
		ID3D12RootSignature* rootSig,
		ID3D12PipelineState* horZBlurPSO,
		ID3D12PipelineState* vertBlurPSO,
		ID3D12Resource* input,
		int blurCount
	);

private:
	std::vector<float> CalcGauseWeights(float sigma);
	void BuildDescriptors();
	void BuildResources();

private:
	const int MaxBlurSize = 5;

	ID3D12Device* md3dDevice = nullptr;

	UINT mWidth = 0;
	UINT mHeight = 0;
	DXGI_FORMAT mFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

	CD3DX12_CPU_DESCRIPTOR_HANDLE mBlur0CPUSrc;
	CD3DX12_CPU_DESCRIPTOR_HANDLE mBlur0CPUUav;

	CD3DX12_CPU_DESCRIPTOR_HANDLE mBlur1CPUSrc;
	CD3DX12_CPU_DESCRIPTOR_HANDLE mBlur1CPUUav;

	CD3DX12_GPU_DESCRIPTOR_HANDLE mBlur0GPUSrc;
	CD3DX12_GPU_DESCRIPTOR_HANDLE mBlur0GPUUav;

	CD3DX12_GPU_DESCRIPTOR_HANDLE mBlur1GPUSrc;
	CD3DX12_GPU_DESCRIPTOR_HANDLE mBlur1GPUUav;

	Microsoft::WRL::ComPtr<ID3D12Resource> mBlurMap0 = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> mBlurMap1 = nullptr;

};
