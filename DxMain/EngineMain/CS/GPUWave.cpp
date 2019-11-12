#include "stdafx.h"
#include "GPUWave.h"
#include "../Common/DXSampleHelper.h"

GPUWave::GPUWave(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, int m, int n, float dx, float dt, float speed, float damping)
{
	md3dDevice = device;
	mNumRows = m;
	mNumColumns = n;
	assert((m*n)%256 == 0);

	mVertexCount = m * n;
	mTriangleCount = (m - 1)*(n - 1) * 2;

	mTimeStep = dt;
	mSpatialStep = dx;
	
	float d = damping * dt + 2.0f;
	float e = (speed* speed)*(dt*dt) / (dx*dx);

	mK[0] = ((damping*dt) - 2.0f) / d;
	mK[1] = (4.0f - 8.0f*e) / d;
	mK[2] = (2.0f*e) / d;

	//BuildResources(cmdList);
}

UINT GPUWave::RowCount()const
{
	return mNumRows;
}

UINT GPUWave::ColumnCount()const
{
	return mNumColumns;
}

UINT GPUWave::VertexCount()const
{
	return mVertexCount;
}

UINT GPUWave::TriangleCount()const
{
	return mTriangleCount;
}

float GPUWave::Width()const
{
	return mNumColumns * mSpatialStep;
}

float GPUWave::Depth()const
{
	return mNumRows * mSpatialStep;
}

float GPUWave::SpatialStep()const
{
	return mSpatialStep;
}

CD3DX12_GPU_DESCRIPTOR_HANDLE GPUWave::DisplacementMap()const
{
	return mCurrSolSrv;
}

UINT GPUWave::DescriptorCount()const
{
	//num of descriptors in heap to reserve for gpuwave
	return 6;
}

void GPUWave::BuildResources(ID3D12GraphicsCommandList* cmdList)
{
	D3D12_RESOURCE_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(D3D12_RESOURCE_DESC));
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texDesc.Alignment = 0;
	texDesc.Width = mNumColumns;
	texDesc.Height = mNumRows;
	texDesc.DepthOrArraySize = 1;
	texDesc.MipLevels = 1;
	texDesc.Format = DXGI_FORMAT_R32_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	ThrowIfFailed(md3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&mPrevSol)
	));

	ThrowIfFailed(md3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&mCurrSol)
	));

	ThrowIfFailed(md3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&mNextSol)
	));

	const UINT num2DSubresources = texDesc.DepthOrArraySize * texDesc.MipLevels;
	const UINT64 uploadBufferSize = GetRequiredIntermediateSize(mCurrSol.Get(), 0, num2DSubresources);

	ThrowIfFailed(md3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(mPrevUploadBuffer.GetAddressOf())
	));

	ThrowIfFailed(md3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(mCurrUploadBuffer.GetAddressOf())
	));

	std::vector<float> initData(mNumRows*mNumColumns, 0.0f);
	for (int i = 0; i < initData.size(); i++)
	{
		initData[i] = 0.0f;
	}

	D3D12_SUBRESOURCE_DATA subResourceData = {};
	subResourceData.pData = initData.data();
	subResourceData.RowPitch = mNumColumns * sizeof(float);
	subResourceData.SlicePitch = subResourceData.RowPitch * mNumRows;

	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mPrevSol.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));
	UpdateSubresources(cmdList, mPrevSol.Get(), mPrevUploadBuffer.Get(), 0, 0, num2DSubresources, &subResourceData);
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mPrevSol.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));

	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mCurrSol.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));
	UpdateSubresources(cmdList, mCurrSol.Get(), mCurrUploadBuffer.Get(), 0, 0, num2DSubresources, &subResourceData);
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mCurrSol.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));

	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mNextSol.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));

}

void GPUWave::BuildDescriptorHeap(CD3DX12_CPU_DESCRIPTOR_HANDLE hCpuDescriptor, CD3DX12_GPU_DESCRIPTOR_HANDLE hGpuDescriptor, UINT descriptorSize)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = DXGI_FORMAT_R32_FLOAT;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2D.MipSlice = 0;

	md3dDevice->CreateShaderResourceView(mPrevSol.Get(), &srvDesc, hCpuDescriptor);
	md3dDevice->CreateShaderResourceView(mCurrSol.Get(), &srvDesc, hCpuDescriptor.Offset(1, descriptorSize));
	md3dDevice->CreateShaderResourceView(mNextSol.Get(), &srvDesc, hCpuDescriptor.Offset(1, descriptorSize));

	md3dDevice->CreateUnorderedAccessView(mPrevSol.Get(), nullptr, &uavDesc, hCpuDescriptor.Offset(1, descriptorSize));
	md3dDevice->CreateUnorderedAccessView(mCurrSol.Get(), nullptr, &uavDesc, hCpuDescriptor.Offset(1, descriptorSize));
	md3dDevice->CreateUnorderedAccessView(mNextSol.Get(), nullptr, &uavDesc, hCpuDescriptor.Offset(1, descriptorSize));

	mPrevSolSrv = hGpuDescriptor;
	mCurrSolSrv = hGpuDescriptor.Offset(1, descriptorSize);
	mNextSolSrv = hGpuDescriptor.Offset(1, descriptorSize);

	mPrevSolUav = hGpuDescriptor.Offset(1, descriptorSize);
	mCurrSolUav = hGpuDescriptor.Offset(1, descriptorSize);
	mNextSolUav = hGpuDescriptor.Offset(1, descriptorSize);

}

void GPUWave::Update(const GameTimer& gt, ID3D12GraphicsCommandList* cmdList, ID3D12RootSignature* rootSig, ID3D12PipelineState* pso)
{

}

void GPUWave::Disturb(ID3D12GraphicsCommandList* cmdList, ID3D12RootSignature* rootSig, ID3D12PipelineState* pso, UINT i, UINT j, float Magnitude)
{

}