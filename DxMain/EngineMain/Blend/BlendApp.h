#pragma once
#include "../Common/D3DApp.h"
#include "../Common/MathHelper.h"
#include "FrameResource_Blend.h"
#include "../Common/Waves.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::PackedVector;

const int gNumFrameResources_Blend = 3;

struct RenderItem_Blend
{
	RenderItem_Blend() = default;

	XMFLOAT4X4 World = MathHelper::Identity4x4();
	XMFLOAT4X4 TexTransform = MathHelper::Identity4x4();

	int NumFramesDirty = gNumFrameResources_Blend;

	UINT ObjCBIndex = -1;
	Material* Mat = nullptr;
	MeshGeometry* Geo = nullptr;

	D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	UINT IndexCount = 0;
	UINT StartIndexLocation = 0;
	int BaseVertexLocation = 0;
};

enum class RenderLayer_Blend : int
{
	Opaque = 0,
	Transparent,
	AlphaTested,
	Count
};

class BlendApp : public D3DApp
{
public:
	BlendApp(HINSTANCE hInstance);
	BlendApp(const BlendApp& rhs) = delete;
	BlendApp& operator=(const BlendApp& rhs) = delete;
	~BlendApp();

	virtual bool Initialize()override;


private:
	void LoadTextures();
	void BuildRootSignature();
	void BuildDescriptorHeaps();
	void BuildShaderInputAndLayout();
	void BuildLandGeometry();
	void BuildWavesGeometry();
	void BuildBoxGeometry();
	void BuildMaterials();
	void BuildRenderItems();
	void BuildFrameResources();
	void BuildPSOs();

	std::array<CD3DX12_STATIC_SAMPLER_DESC, 6> GetStaticSamples();
	float GetHillsHeight(float x, float z);
	XMFLOAT3 GetHillsNormal(float x, float z);

	virtual void Draw(const GameTimer& gt)override;
	void DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem_Blend*> &rItems);

	virtual void OnResize()override;
	virtual void Update(const GameTimer& gt)override;
	void OnKeyboardInput(const GameTimer& gt);
	void UpdateCamera(const GameTimer& gt);
	void AnimationMaterials(const GameTimer& gt);
	void UpdateObjectConstants(const GameTimer& gt);
	void UpdateMarialConstants(const GameTimer& gt);
	void UpdateWaves(const GameTimer& gt);
	void UpdateMainPassCB(const GameTimer& gt);

	virtual void OnMouseDown(WPARAM btnState, int x, int y)override;
	virtual void OnMouseMove(WPARAM btnState, int x, int y)override;
	virtual void OnMouseUp(WPARAM btnState, int x, int y)override;


private:
	std::vector<std::unique_ptr<FrameResource_Blend>> mFrameResources;
	FrameResource_Blend* CurrFrameResource;
	int CurrFrameResourceIndex;

	UINT mCbvSrcDescriptorSize = 0;

	ComPtr<ID3D12RootSignature> mRootSignature = nullptr;
	ComPtr<ID3D12DescriptorHeap> mSrvDescriptorHeap = nullptr;

	std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> mGeometries;
	std::unordered_map<std::string, std::unique_ptr<Material>> mMaterials;
	std::unordered_map<std::string, std::unique_ptr<Texture>> mTextures;
	std::unordered_map<std::string, ComPtr<ID3DBlob>> mShaders;
	std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> mPSOs;

	std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;

	RenderItem_Blend* mWavesRitem = nullptr;
	std::vector<std::unique_ptr<RenderItem_Blend>> mAllRitems;

	std::vector<RenderItem_Blend*> mRitemLayer[(int)RenderLayer_Blend::Count];
	std::unique_ptr<Waves> mWaves;

	PassConstants_Blend mMainPassCB;

	XMFLOAT3 mEyePos = {0.0f, 0.0f, 0.0f };
	XMFLOAT4X4 mView = MathHelper::Identity4x4();
	XMFLOAT4X4 mProj = MathHelper::Identity4x4();

	float mTheta = 1.5f * XM_PI;
	float mPhi = XM_PIDIV2 - 0.1f;
	float mRadius = 50.0f;

	POINT mLastMousePos;
};

