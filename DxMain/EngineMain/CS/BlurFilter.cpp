#include "stdafx.h"
#include "BlurFilter.h"
#include "../Common/DXSampleHelper.h"

BlurFilter::BlurFilter(ID3D12Device* device, UINT Width, UINT Height, DXGI_FORMAT format)
{
	mWidth = Width;
	mHeight = Height;
	mFormat = format;

	md3dDevice = device;

	BuildResources();
}

ID3D12Resource* BlurFilter::Output()
{
	return mBlurMap0.Get();
}

void BlurFilter::BuildDescriptors(CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle, CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle, UINT descriptorSize)
{
	mBlur0CPUSrc = cpuHandle;
	mBlur0CPUUav = cpuHandle.Offset(1, descriptorSize);

	mBlur1CPUSrc = cpuHandle.Offset(1, descriptorSize);
	mBlur1CPUUav = cpuHandle.Offset(1, descriptorSize);

	mBlur0GPUSrc = gpuHandle;
	mBlur0GPUUav = gpuHandle.Offset(1, descriptorSize);
	mBlur1GPUSrc = gpuHandle.Offset(1, descriptorSize);
	mBlur1GPUUav = gpuHandle.Offset(1, descriptorSize);

	BuildDescriptors();
}

void BlurFilter::OnResize(UINT newWidth, UINT newHeight)
{
	if (mWidth != newWidth || newHeight != newHeight)
	{
		mWidth = newWidth;
		mHeight = newHeight;

		BuildResources();

		BuildDescriptors();
	}
}

void BlurFilter::Excute(ID3D12GraphicsCommandList* cmdList, ID3D12RootSignature* rootSig, ID3D12PipelineState* horZBlurPSO, ID3D12PipelineState* vertBlurPSO, ID3D12Resource* input, int blurCount)
{
	auto weights = CalcGauseWeights(2.5f);
	int blurRadius = (int)weights.size() / 2;

	cmdList->SetComputeRootSignature(rootSig);

	cmdList->SetComputeRoot32BitConstants(0, 1, &blurRadius, 0);
	cmdList->SetComputeRoot32BitConstants(0, (UINT)weights.size(), weights.data(), 1);

	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(input, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_SOURCE));
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mBlurMap0.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));
	cmdList->CopyResource(mBlurMap0.Get(), input);

	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mBlurMap0.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mBlurMap1.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));

	for (int i = 0; i < blurCount; i++)
	{
		//horizontal blur pass
		cmdList->SetPipelineState(horZBlurPSO);
		cmdList->SetComputeRootDescriptorTable(1, mBlur0GPUSrc);
		cmdList->SetComputeRootDescriptorTable(2, mBlur1GPUUav);

		UINT numGroupsX = (UINT)ceilf(mWidth / 256.0f);
		cmdList->Dispatch(numGroupsX, mHeight, 1);

		cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mBlurMap0.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
		cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mBlurMap1.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_GENERIC_READ));

		//vertical blur pass
		cmdList->SetPipelineState(vertBlurPSO);
		cmdList->SetComputeRootDescriptorTable(1, mBlur1GPUSrc);
		cmdList->SetComputeRootDescriptorTable(2, mBlur0GPUUav);

		UINT numGroupsY = (UINT)(ceilf(mHeight / 256.0f));
		cmdList->Dispatch(mWidth, numGroupsY, 1);

		cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mBlurMap0.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_GENERIC_READ));
		cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mBlurMap1.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
	}

}

std::vector<float> BlurFilter::CalcGauseWeights(float sigma)
{
	float twoSigma2 = 2.0f * sigma * sigma;

	int blurRadius = (int)ceil(2.0f*sigma);

	assert(blurRadius < MaxBlurSize);

	std::vector<float> weights;
	weights.reserve(2 * blurRadius + 1);

	float weightSum = 0.0f;

	for (int i = -blurRadius; i <= blurRadius; i++)
	{
		float x = (float)i;
		weights[i + blurRadius] = expf(-x*x/twoSigma2);

		weightSum += weights[i + blurRadius];
	}


	for (int i = 0; i < weights.size(); i++)
	{
		weights[i] /= weightSum;
	}

	return weights;
}

void BlurFilter::BuildResources()
{
	D3D12_RESOURCE_DESC ResourceDesc;
	
	ZeroMemory(&ResourceDesc, sizeof(D3D12_RESOURCE_DESC));
	ResourceDesc.Width = mWidth;
	ResourceDesc.Height = mHeight;
	ResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	ResourceDesc.Format = mFormat;
	ResourceDesc.Alignment = 0;
	ResourceDesc.DepthOrArraySize = 1;
	ResourceDesc.MipLevels = 1;
	ResourceDesc.SampleDesc.Count = 1;
	ResourceDesc.SampleDesc.Quality = 0;
	ResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	ResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	ThrowIfFailed(md3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&ResourceDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&mBlurMap0)
	));
	

	md3dDevice->CreateCommittedResource(
	&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&ResourceDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&mBlurMap1)
	);
}

void BlurFilter::BuildDescriptors()
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;

	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = mFormat;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	uavDesc.Format = mFormat;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2D.MipSlice = 0;

	md3dDevice->CreateShaderResourceView(mBlurMap0.Get(), &srvDesc, mBlur0CPUSrc);
	md3dDevice->CreateUnorderedAccessView(mBlurMap0.Get(), nullptr, &uavDesc, mBlur0CPUUav);

	md3dDevice->CreateShaderResourceView(mBlurMap1.Get(), &srvDesc, mBlur1CPUSrc);
	md3dDevice->CreateUnorderedAccessView(mBlurMap1.Get(), nullptr, &uavDesc, mBlur1CPUUav);
}