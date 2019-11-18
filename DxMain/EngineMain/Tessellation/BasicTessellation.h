#pragma once

#include "FrameResource_Tessellation.h"
#include "../Common/MathHelper.h"
#include "../Common/D3DApp.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

const int gNumFrameResources_Tessellation = 3;

struct RenderItem_BasicTess
{
	RenderItem_BasicTess() = default;
	XMFLOAT4X4 World = MathHelper::Identity4x4();
	XMFLOAT4X4 TexTransform = MathHelper::Identity4x4();

	Material* mat = nullptr;
	MeshGeometry* Geo = nullptr;

	UINT ObjCBIndex = -1;

	UINT StartIndexLocation = 0;
	int BaseVertexLocation = 0;
	D3D_PRIMITIVE_TOPOLOGY Primitivetype;
	UINT IndexCount = 0;

	int NumFramesDirty = gNumFrameResources_Tessellation;
};

enum class RenderLayer_BasicTess
{
	Opaque,
	Count,

};

class BasicTessellation : public D3DApp
{
public:
	BasicTessellation(HINSTANCE hInstance);
	BasicTessellation(const BasicTessellation& rhs) = delete;
	BasicTessellation& operator=(const BasicTessellation& rhs) = delete;

	~BasicTessellation();

public:
	
	virtual bool Initialize()override;

	virtual void Update(const GameTimer& gt)override;
	virtual void Draw(const GameTimer& gt)override;
	virtual void OnResize()override;

	virtual void OnMouseDown(WPARAM btnState, int x, int y)override;
	virtual void OnMouseUp(WPARAM btnState, int x, int y)override;
	virtual void OnMouseMove(WPARAM btnState, int x, int y)override;

private:
	void OnKeyboardInput(const GameTimer& gt);
	void UpdateCamera(const GameTimer& gt);

	void AnimateMaterial();
	void UpdateObjectCBs();
	void UpdateMaterialCBs();
	void UpdatePassCBs(const GameTimer& gt);

private:

	void LoadTextures();
	void BuildRootSignature();
	void BuildDescriptorHeaps();
	void BuildShadersAndInputLayout();
	
	void BuildQuadPatchGeometry();
	void BuildMaterials();
	void BuildRenderItems();
	void BuildFrameResources();
	void BuildPSOs();

	void DrawRenderItems(ID3D12GraphicsCommandList* cmdList, std::vector<RenderItem_BasicTess*>& rItems);

	std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> GetStaticSamplers();

private:
	std::vector<std::unique_ptr<FrameResource_Tessellation>> mFrameResources;
	int mCurrFrameResourceIndex;
	FrameResource_Tessellation* mCurrFrameResource;

	UINT mSrvDescriptorSize = 0;

	ComPtr<ID3D12DescriptorHeap> mSrvDescriptorHeap = nullptr;
	ComPtr<ID3D12RootSignature> mRootSignature = nullptr;
	std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;

	std::unordered_map<std::string, std::unique_ptr<Material>> mMaterials;
	std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> mGeometries;
	std::unordered_map<std::string, std::unique_ptr<Texture>> mTextures;
	std::unordered_map<std::string, ComPtr<ID3DBlob>> mShaders;
	std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> mPSOs;

	std::vector<std::unique_ptr<RenderItem_BasicTess>> mAllrItems;
	std::vector<RenderItem_BasicTess*> mAllrItemLayers[(int)RenderLayer_BasicTess::Count];

	PassConstants_Tessellation mMainPassCB;
	XMFLOAT3 mEyePos = {0.0f, 1.0f, -5.0f};
	XMFLOAT4X4 mView = MathHelper::Identity4x4();
	XMFLOAT4X4 mProj = MathHelper::Identity4x4();

	float mTheta = 1.24f*XM_PI;
	float mPhi = 0.42f*XM_PI;
	float mRadius = 12.0f;

	POINT mLastMousePos;



};

