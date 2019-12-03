#pragma once

#include "FrameResrouce_Instance.h"
#include "../Common/d3dx12.h"
#include "../Common/D3DApp.h"
#include "../Common/Camera.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

const int gNumFrameResouces_InstanceAndCull = 3;

struct RenderItem_InstanceAndCull
{
	RenderItem_InstanceAndCull() = default;
	RenderItem_InstanceAndCull(const RenderItem_InstanceAndCull& rhs) = delete;

	XMFLOAT4X4 World = MathHelper::Identity4x4();
	XMFLOAT4X4 TexTransfrom = MathHelper::Identity4x4();

	int NumFramesDirty = gNumFrameResouces_InstanceAndCull;

	UINT ObjCBIndex = -1;

	Material* Mat = nullptr;
	MeshGeometry* Geo = nullptr;

	D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	BoundingBox Bounds;
	std::vector<InstanceData> Instantces;

	UINT IndexCount = 0;
	UINT InstanceCount = 0;
	UINT StartIndexLocation = 0;
	UINT BaseVertexLocation = 0;

};

class InstanceAndCullApp : public D3DApp
{
public:
	InstanceAndCullApp(HINSTANCE hInstance);
	InstanceAndCullApp(const InstanceAndCullApp& rhs) = delete;
	InstanceAndCullApp& operator=(const InstanceAndCullApp& rhs) = delete;
	~InstanceAndCullApp();

	virtual bool Initialize()override;
	virtual void Update(const GameTimer& gt)override;
	virtual void Draw(const GameTimer& gt)override;
	virtual void OnResize()override;

	virtual void OnMouseDown(WPARAM btnState, int x, int y)override;
	virtual void OnMouseUp(WPARAM btnState, int x, int y)override;
	virtual void OnMouseMove(WPARAM btnState, int x, int y)override;

private:
	void LoadTextures();
	void BuildRootSignature();
	void BuildDescriptorHeap();
	void BuildShadersAndInputLayout();

	void LoadSkullGeometry();
	void BuildMaterials();
	void BuildRenderItems();

	void BuildFrameResources();
	void BuildPSos();

	std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> GetStaticSamples();
	void DrawRenderItems(ID3D12GraphicsCommandList* cmdList, std::vector<RenderItem_InstanceAndCull*> mRitems);

	void OnKeyboardInput(const GameTimer& gt);
	void AnimateMaterials(const GameTimer& gt);
	void UpdateMainPassCBs(const GameTimer& gt);
	void UpdateMaterialBuffers(const GameTimer& gt);
	void UpdateInstanceBufferse(const GameTimer& gt);

private:
	std::vector<std::unique_ptr<FrameResource_Instance>> mFrameResources;
	FrameResource_Instance* mCurrentFrameResource;
	int mCurrentFrameResourceIndex;

	UINT mSrvCbvDescriptorSize;

	ComPtr<ID3D12RootSignature> mRootSignature;
	ComPtr<ID3D12DescriptorHeap> mSrvDescriptorHeap;

	std::unordered_map<std::string, std::unique_ptr<Texture>> mTextures;
	std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> mGeometries;
	std::unordered_map<std::string, std::unique_ptr<Material>> mMaterials;

	std::unordered_map<std::string, ComPtr<ID3DBlob>> mShaders;
	std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> mPSOs;

	std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;

	std::vector<std::unique_ptr<RenderItem_InstanceAndCull>> mAllRitems;
	std::vector<RenderItem_InstanceAndCull*> mOpaqueRitems;

	UINT mInstantceCount = 0;

	bool mFrustumCullingEnabled = true;
	BoundingFrustum mCamFrustum;

	PassConstant_Instance mMainPassCB;

	Camera mCamera;

	POINT mLastMousePos;

};

