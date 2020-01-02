#include "stdafx.h"
#include "PickingApp.h"

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

	ThrowIfFailed(mCommandList->Reset(cmdAlloc, mPSOs["opaque"]->));


}

void PickingApp::OnMouseDown(WPARAM btnState, int x, int y)
{
	SetCapture(mhMainWnd);

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}

void PickingApp::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void PickingApp::OnMouseMove(WPARAM btnState, int x, int y)
{

}

void PickingApp::LoadTextures()
{

}

void PickingApp::BuildRootSignature()
{

}

void PickingApp::BuildDescriptorHeaps()
{

}

void PickingApp::BuildShadersAndInputLayout()
{

}

void PickingApp::LoadCarGeometry()
{

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


void PickingApp::OnKeyboardInputEvent()
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