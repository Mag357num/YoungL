#include "stdafx.h"
#include "BoxApp.h"

BoxApp::BoxApp(HINSTANCE hInstance)
	:D3DApp(hInstance)
{

}

BoxApp::~BoxApp()
{

}

bool BoxApp::Initialize()
{
	if (!D3DApp::Initialize())
	{
		return false;
	}

	ThrowIfFailed(mCommandList->Reset(mCommandAllocate.Get(), nullptr));

	BuildDescriptors();
	BuildConstantBuffers();
	BuildRootSignature();
	BuildShadersAndInputLayout();
	BuildBoxGeometry();
	BuildPSO();

	ThrowIfFailed(mCommandList->Close());
	ID3D12CommandList* cmdsLists[] = {mCommandList.Get()};
	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	//wait until initilization is complete
	FlushCommandQueue();

	return true;

}

void BoxApp::OnResize()
{
	D3DApp::OnResize();

	//the window resized , so update the aspect ratio recompute the projection matrix
	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f*MathHelper::PI, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&mProj, P);
}

void BoxApp::Update(const GameTimer& gt)
{

}

void BoxApp::Draw(const GameTimer& gt)
{

}

void BoxApp::OnMouseDown(WPARAM btnState, int x, int y)
{

}

void BoxApp::OnMouseMove(WPARAM btnState, int x, int y)
{

}

void BoxApp::OnMouseUp(WPARAM btnState, int x, int y)
{

}

void BoxApp::BuildDescriptors()
{

}

void BoxApp::BuildConstantBuffers()
{

}

void BoxApp::BuildRootSignature()
{

}

void BoxApp::BuildShadersAndInputLayout()
{

}

void BoxApp::BuildBoxGeometry()
{

}

void BoxApp::BuildPSO()
{

}