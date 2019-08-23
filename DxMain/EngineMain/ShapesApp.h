#pragma once

#include "Common/D3DApp.h"
#include "Common/MathHelper.h"
#include "Common/UploadBuffer.h"
#include "Common/FrameResource.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;

const int gNumFrameResources = 3;

struct ShapeRenderItem
{
	ShapeRenderItem() = default;

	//world matrix of shape that descripes the object's local space
	//relative to the world space, which defines the position. orientation,
	//and scale of the object in the world
	DirectX::XMFLOAT4X4 World = MathHelper::Identity4x4();

	//dirty flat indicating the object data has changed and we need to update the constant buffer.
	//because we have an object cbuffer for each frameresource, we have to apply the update to each frameresource
	// Thus, when we modify obect data we should set 
	// NumFramesDirty = gNumFrameResources so that each frame resource gets the update.
	int NumFramesDirty = gNumFrameResources;

	//index into GPU constant buffer corresponding to the object CB for this render item
	UINT ObjCBIndex = -1;

	MeshGeometry* Geo = nullptr;

	//primitive topology.
	D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	//draw indexed instanced parameters
	UINT IndexCount = 0;
	UINT StartIndexLocation = 0;
	int BaseVertexLocation = 0;

};

class ShapeApp : public D3DApp
{
public:
	ShapeApp(HINSTANCE hInstance);
	ShapeApp(const ShapeApp& rhs) = delete;
	ShapeApp& operator=(const ShapeApp& rhs) = delete;
	~ShapeApp();
	
	virtual bool Initialize()override;

private:
	virtual void OnResize()override;
	virtual void Update(const GameTimer& gt)override;
	virtual void Draw(const GameTimer& gt)override;

	virtual void OnMouseDown(WPARAM btnState, int x, int y)override;
	virtual void OnMouseUp(WPARAM btnState, int x, int y)override;
	virtual void OnMouseMove(WPARAM btnState, int x, int y)override;

	void OnKeyboardInput(const GameTimer& gt);
	void UpdateCamera(const GameTimer& gt);
	void UpdataObjectCBs(const GameTimer& gt);
	void UpdateMainPassCBs(const GameTimer& gt);

	void BuildDescriptorHeaps();
	void BuildConstantBufferViews();
	void BuildRootSignature();
	void BuildShapesAndInputLayout();
	void BuildShapeGeometry();
	void BuildPSOs();
	void BuildFrameResources();
	void BuildRenderItems();
	void DrawRenderItems(ID3D12CommandList* cmdList, const std::vector<ShapeRenderItem*>& rItems);

private:
	std::vector<std::unique_ptr<FrameResource>> mFrameResources;
	FrameResource* mCurrentFrameResource = nullptr;
	int mCurrentFrameResourceIndex = 0;

	ComPtr<ID3D12RootSignature> mRootSignature = nullptr;
	ComPtr<ID3D12DescriptorHeap> mCbvHeap = nullptr;

	ComPtr<ID3D12DescriptorHeap> mSrvDescriptorHeap = nullptr;

	std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> mGeometries;
	std::unordered_map<std::string, ComPtr<ID3DBlob>> mShaders;
	std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> mPSOs;

	std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;

	//list all of render items
	std::vector<std::unique_ptr<ShapeRenderItem>> mAllItems;

	//render items divided by PSO.
	std::vector<ShapeRenderItem*> mOpaqueRItems;

	PassConstants mMainPassCB;

	UINT mPassCbnOffset = 0;

	bool mIsWireframe = false;

	XMFLOAT3 mEyePos = {0.0f, 0.0f, 0.0f};
	XMFLOAT4X4 mView = MathHelper::Identity4x4();
	XMFLOAT4X4 mProj = MathHelper::Identity4x4();

	float mTheta = 1.5f*XM_PI;
	float mPhi = 0.2f*XM_PI;
	float mRadius = 15.0f;

	POINT mLastMousePos;

};
