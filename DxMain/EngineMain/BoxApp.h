#pragma once
#include "Common/D3DApp.h"
#include "Common/DXSampleHelper.h"
#include "Common/UploadBuffer.h"

struct Vertex
{
	XMFLOAT3 Pos;
	XMFLOAT4 Color;
};

struct ObjectContants
{

};

class BoxApp : public D3DApp
{
public:
	BoxApp(HINSTANCE hInstance);
	BoxApp(const BoxApp& rhs) = delete;
	BoxApp& operato(const BoxApp& rhs) = delete;
	~BoxApp();

	virtual bool Initialize()override;

private:
	virtual void OnResize()override;
	virtual void Update(const GameTimer& gt)override;
	virtual Draw(const GameTimer& gt)override;

	virtual void OnMouseDown(WPARAM btnState, int x, int y)override;
	virtual void OnMouseUp(WPARAM btnState, int x, int y)override;
	virtual void OnMouseMove(WPARAM btnState, int x, int y)override;

	void BuildDescriptors();
	void BuildConstantBuffers();
	void BuildRootSignature();
	void BuildShadersAndInputLayout();
	void BuildBoxGeometry();
	void BuildPSO();

private:
	ComPtr<ID3D12RootSignature> mRootSignature = nullptr;
	ComPtr<ID3D12DescriptorHeap> mCbvHeap = nullptr;

	std::unique_ptr<UploadBuffer<ObjectConstants>> mObjectCB = nullptr;

};