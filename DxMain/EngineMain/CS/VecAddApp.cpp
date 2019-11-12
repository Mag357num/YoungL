#include "stdafx.h"
#include "VecAddApp.h"
#include "../Common/DXSampleHelper.h"

VecAddApp::VecAddApp(HINSTANCE hInstance)
	:D3DApp(hInstance)
{

}

VecAddApp::~VecAddApp()
{
	if (md3dDevice != nullptr)
	{
		FlushCommandQueue();
	}
}

bool VecAddApp::Initialize()
{
	if (!D3DApp::Initialize())
	{
		return false;
	}

	ThrowIfFailed(mCommandList->Reset(mCommandAllocate.Get(), nullptr));

	BuildBuffers();
	BuildRootSignature();
	BuildDescriptorHeaps();
	BuildShadersAndInputLayout();
	BuildFrameResources();
	BuildPSOs();

	ThrowIfFailed(mCommandList->Close());
	ID3D12CommandList* cmdLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);

	FlushCommandQueue();

	//do compute work
	DoComputerWork();

	return true;
}

void VecAddApp::DoComputerWork()
{
	ThrowIfFailed(mCommandAllocate->Reset());

	ThrowIfFailed(mCommandList->Reset(mCommandAllocate.Get(), mPSOs["vecAdd"].Get()));
	mCommandList->SetComputeRootSignature(mRootSignature.Get());

	mCommandList->SetComputeRootShaderResourceView(0, InputVecBufferA->GetGPUVirtualAddress());
	mCommandList->SetComputeRootShaderResourceView(1, InputVecBufferB->GetGPUVirtualAddress());
	mCommandList->SetComputeRootUnorderedAccessView(2, OutputBuffer->GetGPUVirtualAddress());

	mCommandList->Dispatch(1, 1, 1);

	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(OutputBuffer.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_SOURCE));
	mCommandList->CopyResource(ReadBackBuffer.Get(), OutputBuffer.Get());
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(OutputBuffer.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COMMON));

	ThrowIfFailed(mCommandList->Close());
	ID3D12CommandList* cmdsList[] = {mCommandList.Get()};
	mCommandQueue->ExecuteCommandLists(_countof(cmdsList), cmdsList);
	FlushCommandQueue();

	Data* mappedData = nullptr;
	ReadBackBuffer->Map(0, nullptr, reinterpret_cast<void**>(&mappedData));

	std::ofstream fout("result.txt");

	for (int i = 0; i < NumDataElements; i++)
	{
		fout << "(" << mappedData[i].v1.x << "," << mappedData[i].v1.y << "," << mappedData[i].v1.z << "," << mappedData[i].v2.x << "," << mappedData[i].v2.y << ")" << std::endl;
	}
	ReadBackBuffer->Unmap(0, nullptr);

}

void VecAddApp::Draw(const GameTimer& gt)
{
	auto cmdListAlloc = mCurrFrameResource->CmdListAlloc;
	ThrowIfFailed(cmdListAlloc->Reset());

	ThrowIfFailed(mCommandList->Reset(cmdListAlloc.Get(), mPSOs["opaque"].Get()));

	mCommandList->RSSetViewports(1, &mScreenViewport);
	mCommandList->RSSetScissorRects(1, &mScissorRect);

	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	mCommandList->ClearRenderTargetView(CurrentBackBufferView(), Colors::Blue, 0, nullptr);
	mCommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
	mCommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());

	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	ThrowIfFailed(mCommandList->Close());

	ID3D12CommandList* cmdList[] = {mCommandList.Get()};
	mCommandQueue->ExecuteCommandLists(_countof(cmdList), cmdList);

	mSwapChain->Present(0, 0);
	mCurrBackBuffer = (mCurrBackBuffer + 1) % SwapChainBufferCount;

	mCurrFrameResource->Fence = ++mCurrentFence;
	mCommandQueue->Signal(md3d12Fence.Get(), mCurrentFence);
}

void VecAddApp::Update(const GameTimer& gt)
{
	mCurrFrameResourceIndex = (mCurrFrameResourceIndex) % gNumFrameResource_VecAdd;
	mCurrFrameResource = mFrameResources[mCurrFrameResourceIndex].get();

	if (mCurrFrameResource->Fence!=0&&md3d12Fence->GetCompletedValue()<mCurrFrameResource->Fence)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
		md3d12Fence->SetEventOnCompletion(mCurrFrameResource->Fence, eventHandle);
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}
	
}

void VecAddApp::OnResize()
{
	D3DApp::OnResize();
}

void VecAddApp::BuildBuffers()
{
	std::vector<Data> InputA(NumDataElements);
	std::vector<Data> InputB(NumDataElements);
	for (int i = 0; i < NumDataElements; i++)
	{
		InputA[i].v1 = XMFLOAT3(i*1.0f , i*1.0f, i*1.0f);
		InputA[i].v2 = XMFLOAT2(i*1.0f, 0);

		InputB[i].v1 = XMFLOAT3(-i * 1.0f, i*1.0f, 0.0f);
		InputB[i].v2 = XMFLOAT2(0, -i * 1.0f);
	}

	UINT64 BufferSize = InputA.size() * sizeof(Data);

	InputVecBufferA = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), InputA.data(), BufferSize, InputVecBufferUploadA);
	InputVecBufferB = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), InputB.data(), BufferSize, InputVecBufferUploadB);

	ThrowIfFailed(md3dDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(BufferSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS),
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		nullptr,
		IID_PPV_ARGS(&OutputBuffer)));

	ThrowIfFailed(md3dDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(BufferSize),
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&ReadBackBuffer)));


}

void VecAddApp::BuildRootSignature()
{
	CD3DX12_ROOT_PARAMETER slotRootparamaters[3];
	slotRootparamaters[0].InitAsShaderResourceView(0);
	slotRootparamaters[1].InitAsShaderResourceView(1);
	slotRootparamaters[2].InitAsUnorderedAccessView(0);

	CD3DX12_ROOT_SIGNATURE_DESC signatureDesc(3, slotRootparamaters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_NONE);

	ComPtr<ID3DBlob> serializedBlob = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;

	HRESULT hr = D3D12SerializeRootSignature(&signatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, serializedBlob.GetAddressOf(), errorBlob.GetAddressOf());
	if (errorBlob != nullptr)
	{
		OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}
	ThrowIfFailed(hr);

	ThrowIfFailed(md3dDevice->CreateRootSignature(0, serializedBlob->GetBufferPointer(), serializedBlob->GetBufferSize(), IID_PPV_ARGS(&mRootSignature)));
}

void VecAddApp::BuildDescriptorHeaps()
{
	
}

void VecAddApp::BuildShadersAndInputLayout()
{
	mShaders["vecAddCS"] = d3dUtil::CompileShader(L"shaders\\CS_VecAdd.hlsl", nullptr, "CS", "cs_5_0");
}

void VecAddApp::BuildPSOs()
{
	D3D12_COMPUTE_PIPELINE_STATE_DESC csPipelineDesc = {};
	csPipelineDesc.pRootSignature = mRootSignature.Get();
	csPipelineDesc.CS = {
		reinterpret_cast<BYTE*>(mShaders["vecAddCS"]->GetBufferPointer()),
		mShaders["vecAddCS"]->GetBufferSize(),
	};
	csPipelineDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

	md3dDevice->CreateComputePipelineState(&csPipelineDesc, IID_PPV_ARGS(&mPSOs["vecAdd"]));
}

void VecAddApp::BuildFrameResources()
{
	for (int i = 0; i < gNumFrameResource_VecAdd; i++)
	{
		mFrameResources.push_back(std::make_unique<FrameResource_CS>(md3dDevice.Get(), 0, 0, 0));
	}
}

std::array<D3D12_STATIC_SAMPLER_DESC, 6> VecAddApp::GetStaticSamples()
{
	// Applications usually only need a handful of samplers.  So just define them all up front
	// and keep them available as part of the root signature.  

	const CD3DX12_STATIC_SAMPLER_DESC pointWrap(
		0, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC pointClamp(
		1, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC linearWrap(
		2, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC linearClamp(
		3, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicWrap(
		4, // shaderRegister
		D3D12_FILTER_ANISOTROPIC, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressW
		0.0f,                             // mipLODBias
		8);                               // maxAnisotropy

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicClamp(
		5, // shaderRegister
		D3D12_FILTER_ANISOTROPIC, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressW
		0.0f,                              // mipLODBias
		8);                                // maxAnisotropy

	return {
		pointWrap, pointClamp,
		linearWrap, linearClamp,
		anisotropicWrap, anisotropicClamp };
}

