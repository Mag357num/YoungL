#pragma once
#include "../Common/D3DApp.h"
#include "../Common/MathHelper.h"
#include "../Common/UploadBuffer.h"
#include "../Common/GeometryGenerator.h"

#include "FrameResource_Stencil.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

const int gNumFrameResrouces_Stencil = 3;

struct RenderItem_Stencil
{
	RenderItem_Stencil() = default;

	XMFLOAT4X4 World = MathHelper::Identity4x4();
	XMFLOAT4X4 TexTransform = MathHelper::Identity4x4();

	int NumFramesDirty = gNumFrameResrouces_Stencil;

	UINT ObjCBIndex = -1;

	Material* Mat = nullptr;
	MeshGeometry* Geo = nullptr;

	D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	UINT IndexCount = 0;
	UINT StartIndexLocation = 0;
	int BaseVertexLocation = 0;
};

enum class RenderLayer_Stencil:int
{
	Opaque = 0,
	Mirrors,
	Reflected,
	Transparent,
	Shadow,
	Count

};

class StencilApp:public D3DApp
{
public:
	StencilApp(HINSTANCE hInstance);
	StencilApp(const StencilApp& rhs) = delete;
	StencilApp& operator=(const StencilApp& rhs) = delete;
	~StencilApp();

	virtual bool Initialize()override;

private:
	void LoadTextures();
	void BuildRootSignature();
	void BuildDescriptorHeaps();
	void BuildShadersAndInputlayout();
	void BuildRoomGeometry();
	void BuildSkullGeometry();
	void BuildMaterials();
	void BuildRenderItems();
	void BuildFrameResource();
	void BuildPSOs();

	std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6>  GetStaticSamplers();

private:
	std::vector<std::unique_ptr<FrameResource_Stencil>> mFrameResources;
	FrameResource_Stencil* CurrentFrameResource;
	int mCurrentFrameResourceIndex = 0;

	UINT mCbvSrvHeapDescriptorSize = 0;


	ComPtr<ID3D12RootSignature> mRootSignature = nullptr;

	ComPtr<ID3D12DescriptorHeap> mSrvDesctiptorHeap = nullptr;

	std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> mGeometries;
	std::unordered_map<std::string, std::unique_ptr<Material>> mMaterials;
	std::unordered_map<std::string, std::unique_ptr<Texture>> mTextures;
	std::unordered_map<std::string, ComPtr<ID3DBlob>> mShaders;
	std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> mPSOs;

	std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;

	RenderItem_Stencil* mSkullRitem = nullptr;
	RenderItem_Stencil* mReflectedSkullRitem = nullptr;
	RenderItem_Stencil* mShadowedSkillRitem = nullptr;

	std::vector<std::unique_ptr<RenderItem_Stencil>> mAllRitems;

	std::vector<RenderItem_Stencil*> mRitemLayer[(int)RenderLayer_Stencil::Count];

	PassConstants_Stencil mMainPassCB;
	PassConstants_Stencil mReflectedPassCB;

	XMFLOAT3 mSkullTranslation = {0.0f, 1.0f, -5.0f};

	XMFLOAT3 mEyePos = {0.0f, 0.0f, 0.0f};
	XMFLOAT4X4 mView = MathHelper::Identity4x4();
	XMFLOAT4X4 mProj = MathHelper::Identity4x4();

	float mTheta = 1.24f*XM_PI;
	float mPhi = 0.42f*XM_PI;
	float mRadiu = 12.0f;

	POINT mLastMousePos;
};
