#pragma once
#include "../Common/D3DApp.h"
#include "../Common/d3dUtil.h"
#include "../Common/GameTimer.h"
#include "../Common/DXSampleHelper.h"

#include "FrameResource_CS.h"
#include "BlurFilter.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

const int gNumFrameResources_BlurCS = 3;

struct RenderItem_BlurCS
{
	RenderItem_BlurCS() = default;

	XMFLOAT4X4 World = MathHelper::Identity4x4();
	XMFLOAT4X4 TexTransform = MathHelper::Identity4x4();

	int NumFramesDirty = gNumFrameResources_BlurCS;

	UINT ObjCBIndex = -1;
	Material* Mat = nullptr;
	MeshGeometry* Geo = nullptr;

	D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	UINT IndexCount = 0;
	UINT StartIndexLocation = 0;
	UINT BaseVertexLocation = 0;
};

enum class RenderLayer_BlurCS
{
	Opaque = 0,
	AlphaTested,
	Count

};


class BlurCSApp : public D3DApp
{
public:
	BlurCSApp(HINSTANCE hInstance);
	BlurCSApp(const BlurCSApp& rhs) = delete;
	BlurCSApp& operator=(const BlurCSApp& rhs) = delete;

	~BlurCSApp();

	virtual bool Initialize()override;

private:
	virtual void Update(const GameTimer& gt)override;
	virtual void OnResize()override;
	virtual void Draw(const GameTimer& gt)override;

private:
	virtual void OnMouseDown(WPARAM btnState, int x, int y)override;
	virtual void OnMouseUp(WPARAM btnState, int x, int y)override;
	virtual void OnMouseMove(WPARAM btnState, int x, int y)override;

private:
	void OnKeyboardInput();
	void UpdateCamer();

	void UpdateObjConstants();
	void UpdateMatConstants();
	void UpdateMainpassConstants(const GameTimer& gt);

	void DrawRenderItems(ID3D12GraphicsCommandList* cmdList, std::vector<RenderItem_BlurCS*>& rItems);

private:
	void LoadTextures();
	std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> GetStaticSamples();
	void BuildRootSignatures();
	void BuildBlurRootSignatures();
	void BuildDescriptorHeap();
	void BuildShadersAndInputLayout();
	void BuildLandGeometry();
	void BuildBoxGeometry();
	void BuildMaterials();
	void BuildRenderItems();
	void BuildFrameResources();
	void BuildPSos();

	float GetHillsHeight(float X, float Z);
	XMFLOAT3 GetHillsNormal(float X, float Z)const;

private:
	std::vector<std::unique_ptr<FrameResource_CS>> mFrameResources;
	FrameResource_CS* mCurrFrameResource;
	int mCurrFrameResourceIndex;

	ComPtr<ID3D12RootSignature> mRootSignature = nullptr;
	ComPtr<ID3D12RootSignature> mBlurRootSignature = nullptr;

	UINT mSrvDescriptorSize;
	ComPtr<ID3D12DescriptorHeap> mCbvSrvUavDescriptorHeap = nullptr;

	std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;

	std::unordered_map<std::string, std::unique_ptr<Material>> mMaterials;
	std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> mGeometries;
	std::unordered_map<std::string, std::unique_ptr<Texture>> mTextures;

	std::unordered_map<std::string, ComPtr<ID3DBlob>> mShaders;
	std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> mPSOs;

	std::vector<std::unique_ptr<RenderItem_BlurCS>> mAllrItems;
	std::vector<RenderItem_BlurCS*> mAllRItemLayers[(int)RenderLayer_BlurCS::Count];

	std::unique_ptr<BlurFilter> mBlurFilter;
	PassConstants_CS mMainPassCB;

	XMFLOAT3 mEyePosW = {0.0f, 0.0f, 0.0f};
	XMFLOAT4X4 mView = MathHelper::Identity4x4();
	XMFLOAT4X4 mProj = MathHelper::Identity4x4();

	float mTheta = 1.5f* XM_PI;
	float mPhi = XM_PIDIV2 - 0.1f;
	float mRadius = 50.0f;

	POINT mLastMousePosition;
};