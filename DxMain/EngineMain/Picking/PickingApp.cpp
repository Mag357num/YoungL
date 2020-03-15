#include "stdafx.h"
#include "PickingApp.h"
#include "../Common/DDSTextureLoader.h"

PickingApp::PickingApp(HINSTANCE hInstance)
	:D3DApp(hInstance)
{

}

PickingApp::~PickingApp()
{
	if (md3dDevice!=nullptr)
	{
		FlushCommandQueue();
	}
}

bool PickingApp::Initialize()
{
	if (!D3DApp::Initialize())
	{
		return false;
	}

	ThrowIfFailed(mCommandList->Reset(mCommandAllocate.Get(), nullptr));

	mCbvSrvDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	mCamera.LookAt(
	XMFLOAT3(5.0f, 4.0f, -15.0f),
		XMFLOAT3(0.0f, 1.0f ,0.0f),
		XMFLOAT3(0.0f, 1.0f, 0.0f)
	);


	ThrowIfFailed(mCommandList->Close());
	ID3D12CommandList* cmdLists[] = { mCommandList.Get()};
	mCommandQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);

	FlushCommandQueue();

	return true;
}

void PickingApp::OnResize()
{
	D3DApp::OnResize();

	mCamera.SetLens(0.25f*MathHelper::PI, AspectRatio(), 1.0f, 1000.0f);
}

void PickingApp::Update(const GameTimer& gt)
{
	OnKeyboardInputEvent();

	mCurrFrameResourceIndex = (mCurrFrameResourceIndex + 1) % gNumFrameResources_Picking;
	mCurrFrameResource = mFrameResoruces[mCurrFrameResourceIndex].get();

	if (mCurrFrameResource->Fence != 0 &&  md3d12Fence->GetCompletedValue() < mCurrFrameResource->Fence)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
		md3d12Fence->SetEventOnCompletion(mCurrFrameResource->Fence, eventHandle);
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}


	UpdateObjCBs();
	UpdatePassCBs();
	UpdateMaterialCBs();

}

void PickingApp::Draw(const GameTimer& gt)
{
	auto cmdAlloc = mCurrFrameResource->CmdListAlloc.Get();
	ThrowIfFailed(cmdAlloc->Reset());

	ThrowIfFailed(mCommandList->Reset(cmdAlloc, mPSOs["opaque"].Get()));

	mCommandList->RSSetViewports(1, &mScreenViewport);
	mCommandList->RSSetScissorRects(1, &mScissorRect);

	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	mCommandList->ClearRenderTargetView(CurrentBackBufferView(), Colors::LightSteelBlue, 0, nullptr);
	mCommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	mCommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());

	ID3D12DescriptorHeap* descriptorHeaps[] = {
		mSrvDescriptorHeap.Get()
	};
	mCommandList->SetDescriptorHeaps(sizeof(descriptorHeaps), descriptorHeaps);

	mCommandList->SetGraphicsRootSignature(mRootSignature.Get());

	
	ThrowIfFailed(mCommandList->Close());
	ID3D12CommandList* cmdLists[] = {
		mCommandList.Get()
	};

	
	mCommandQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);
	
	ThrowIfFailed(mSwapChain->Present(0, 0));
	mCurrBackBuffer = (mCurrBackBuffer + 1) % SwapChainBufferCount;

	mCurrFrameResource->Fence = ++mCurrentFence;
	mCommandQueue->Signal(md3d12Fence.Get(), mCurrentFence);


}

void PickingApp::OnMouseDown(WPARAM btnState, int x, int y)
{
	if ((btnState & MK_LBUTTON) != 0)
	{
		mLastMousePos.x = x;
		mLastMousePos.y = y;
		SetCapture(mhMainWnd);
	}
	else if((btnState & MK_RBUTTON) != 0)
	{
		
	}
	
}

void PickingApp::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void PickingApp::OnMouseMove(WPARAM btnState, int x, int y)
{
	if (btnState&MK_LBUTTON != 0)
	{
		float dx = XMConvertToRadians(0.25f*static_cast<float>(x - mLastMousePos.x));
		float dy = XMConvertToRadians(0.25f*static_cast<float>(y - mLastMousePos.y));
		
		mCamera.Pitch(dy);
		mCamera.RotateY(dx);
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}

void PickingApp::OnKeyboardInputEvent(const GameTimer& gt)
{
	const float dt = gt.DeltaTime();
	if (GetAsyncKeyState('W') & 0x8000)
	{
		mCamera.Walk(10.0f*dt);
	}

	if (GetAsyncKeyState('S')& 0x8000)
	{
		mCamera.Walk(-10.0f*dt);
	}

	if (GetAsyncKeyState('A') & 0x8000)
	{
		mCamera.Strafe(-10.0f*dt);
	}

	if (GetAsyncKeyState('D') & 0x8000)
	{
		mCamera.Strafe(10.0f*dt);
	}

	mCamera.UpdateViewMatrix();

}

void PickingApp::LoadTextures()
{
	auto defaultDiffuseTex = std::make_unique<Texture>();
	defaultDiffuseTex->Name = "defaultDiffuseTex";
	defaultDiffuseTex->FileName = L"Textures/white1x1.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(), mCommandList.Get(), defaultDiffuseTex->FileName.c_str(),
		defaultDiffuseTex->Resource, defaultDiffuseTex->UploadHeap));

	mTextures[defaultDiffuseTex->Name] = std::move(defaultDiffuseTex);
}

void PickingApp::BuildRootSignature()
{
	CD3DX12_DESCRIPTOR_RANGE texTable;
	texTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 4, 0, 0);

	CD3DX12_ROOT_PARAMETER slootRootParameter[4];
	slootRootParameter[0].InitAsConstantBufferView(0);
	slootRootParameter[1].InitAsConstantBufferView(1);
	slootRootParameter[2].InitAsShaderResourceView(0, 1);
	slootRootParameter[3].InitAsDescriptorTable(1, &texTable, D3D12_SHADER_VISIBILITY_PIXEL);

	auto staticSamplers = GetStaticSamples();

	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(4, slootRootParameter,
		(UINT)staticSamplers.size(), staticSamplers.data(),
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());
	ThrowIfFailed(hr);
	ThrowIfFailed(md3dDevice->CreateRootSignature(0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(mRootSignature.GetAddressOf())));
}

void PickingApp::BuildDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = 1;
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	ThrowIfFailed(md3dDevice->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&mSrvDescriptorHeap)));

	CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(mSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	auto defaultDiffuseTex = mTextures["defaultDiffuseTex"]->Resource;

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = defaultDiffuseTex->GetDesc().Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = defaultDiffuseTex->GetDesc().MipLevels;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	md3dDevice->CreateShaderResourceView(defaultDiffuseTex.Get, &srvDesc, hDescriptor);
}

void PickingApp::BuildShadersAndInputLayout()
{
	const D3D_SHADER_MACRO alphaTestDefines[] =
	{
		"ALPHA_TEST", "1",
		NULL, NULL
	};

	mShaders["standardVS"] = d3dUtil::CompileShader(L"Shaders\\Default.hlsl", nullptr, "VS", "vs_5_1");
	mShaders["opaquePS"] = d3dUtil::CompileShader(L"Shaders\\default.hlsl", nullptr, "PS", "ps_5_1");

	mInputLayout = 
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	};
}

void PickingApp::LoadCarGeometry()
{
	std::ifstream fin("Models/car.txt");
	if (!fin)
	{
		MessageBox(0, L"Models/car.txt not found", 0, 0);
		return;
	}

	UINT vcount = 0;
	UINT tcount = 0;
	std::string ignore;

	fin >> ignore >> vcount;
	fin >> ignore >> tcount;
	fin >> ignore >> ignore >> ignore >> ignore;

	XMFLOAT3 vMinf3(+MathHelper::Infinity, +MathHelper::Infinity, +MathHelper::Infinity);
}

void PickingApp::BuildMaterials()
{

}

void PickingApp::BuildRenderItems()
{

}

void PickingApp::BuildFrameResources()
{

}

void PickingApp::BuildPSOs()
{

}


void PickingApp::UpdateObjCBs()
{

}

void PickingApp::UpdatePassCBs()
{

}

void PickingApp::UpdateMaterialCBs()
{

}