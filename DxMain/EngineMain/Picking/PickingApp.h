#pragma once

#include "../Common/D3DApp.h"
#include "../Common/MathHelper.h"
#include "../Common/d3dUtil.h"
#include "../Common/Camera.h"
#include "FrameResource_Picking.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::PackedVector;

const int gNumFrameResources_Picking = 3;

struct RenderItem_Picking
{
	RenderItem_Picking() = default;
	RenderItem_Picking(const RenderItem_Picking& rhs) = delete;

	bool Visible = true;

	BoundingBox Bounds;

	XMFLOAT4X4 World = MathHelper::Identity4x4();
	XMFLOAT4X4 TexTransform = MathHelper::Identity4x4();

	int gNumFramesDirty = gNumFrameResources_Picking;

	UINT ObjCBIndex = -1;

	Material* Mat = nullptr;
	MeshGeometry* Geo = nullptr;

	//Primitive topology
	D3D12_PRIMITIVE_TOPOLOGY PrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	UINT IndexCount = 0;
	UINT StartIndexLocation = 0;
	UINT BaseVertexLocation = 0;

};

enum class RenderLayer_Picking
{
	Opaque = 0,
	Highlight,
	Count
};

class PickingApp : public D3DApp
{
public:
	PickingApp(HINSTANCE hInstance);
	PickingApp(const PickingApp& rhs) = delete;
	PickingApp& operator=(const PickingApp& rhs) = delete;
	~PickingApp();

	virtual bool Initialize()override;

private:

	virtual void Update(const GameTimer& gt)override;
	virtual void Draw(const GameTimer& gt)override;
	virtual void OnResize()override;

	virtual void OnMouseDown(WPARAM btnState, int x, int y)override;
	virtual void OnMouseMove(WPARAM btnState, int x, int y)override;
	virtual void OnMouseUp(WPARAM btnState, int x, int y)override;

private:
	void LoadTextures();
	void BuildRootSignature();
	void BuildDescriptorHeaps();
	void BuildShadersAndInputLayout();

	void LoadCarGeometry();
	void BuildMaterials();
	void BuildRenderItems();
	void BuildFrameResources();
	void BuildPSOs();

	void OnKeyboardInputEvent();
	void UpdateObjCBs();
	void UpdatePassCBs();
	void UpdateMaterialCBs();

	std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> GetStaticSamples();

private:
	std::vector<std::unique_ptr<FrameResource_Picking>> mFrameResoruces;
	FrameResource_Picking* mCurrFrameResource = nullptr;
	int mCurrFrameResourceIndex = 0;

	UINT mCbvSrvDescriptorSize = 0;

	ComPtr<ID3D12RootSignature> mRootSignature = nullptr;
	ComPtr<ID3D12DescriptorHeap> mSrvDescriptorHeap = nullptr;

	std::unordered_map<std::string, std::unique_ptr<MaterialData_Picking>> mMaterials;
	std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> mGeometies;
	std::unordered_map<std::string, std::unique_ptr<Texture>> mTextures;
	std::unordered_map<std::string, ComPtr<ID3DBlob>> mShaders;
	std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> mPSOs;

	std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;

	std::vector<std::unique_ptr<RenderItem_Picking>> mAllRItems;

	std::vector<RenderItem_Picking*> mRitemLayer[(int)RenderLayer_Picking::Count];
	RenderItem_Picking* mPickedRitem = nullptr;

	PassConstants_Picking mMainPassCB;
	Camera mCamera;
	
	POINT mLastMousePos;

};
