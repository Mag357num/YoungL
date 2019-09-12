#pragma once
#include "../Common/D3DApp.h"
#include "../Common/MathHelper.h"
#include "../Common/d3dUtil.h"
#include "FrameResource_LitWave.h"

using namespace DirectX;
using namespace DirectX::PackedVector;

const int gNumFrameResources_LitColumns = 3;

struct RenderItem_LitColumns
{
	RenderItem_LitColumns() = default;

	XMFLOAT4X4 World = MathHelper::Identity4x4();

	XMFLOAT4X4 TexTransform = MathHelper::Identity4x4();

	int NumFramesDirty = gNumFrameResources_LitColumns;

	UINT ObjCBIndex = -1;

	Material* Mat = nullptr;
	MeshGeometry* Geo = nullptr;

	D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	UINT IndexCount = 0;
	UINT StartIndexLocation = 0;
	UINT BaseVertexLocation = 0;

};

class LitColumnsApp : public D3DApp
{
public:
	LitColumnsApp(HINSTANCE hInstance);
	LitColumnsApp(const LitColumnsApp& rhs) = delete;
	LitColumnsApp& operator=(const LitColumnsApp& rhs) = delete;
	~LitColumnsApp();

	virtual bool Initialize()override;

private:
	
	
	
	void BuildroogSignature();
	void BuildShadersAndInputlayout();
	void BuildShapeGeometry();
	void BuildSkullGeometry();
	//void BuildPSOs();
	//void BuildFrameResrouces();
	//void BuildMaterials();
	//void BuildrenderItems();

private:
	//ComPtr<ID3D12RootSignature> mRootSignature;

	//std::vector<std::unique_ptr<FrameResource_LitWave>> mFrameResources;
	//FrameResource_LitWave* CurrentFrameResource;
	//int mCurrentFrameResourceIndex;

	//UINT mCbvSrvDescriptorSize;
	//ComPtr<ID3D12DescriptorHeap> mSrvDescriptorHeap = nullptr;

	//std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;

	//ComPtr<ID3D12PipelineState> mOpaquePSO = nullptr;

	//std::vector<std::unique_ptr<RenderItem_LitColumns>> mAllRitems;
	//std::vector<RenderItem_LitColumns> mOpaqueRitems;

	//PassConstants_LitWave mMainPassCB;

	//std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> mGeometries;
	//std::unordered_map<std::string, std::unique_ptr<Material>> mMaterials;
	//std::unordered_map<std::string, std::unique_ptr<Texture>> mTextures;
	//std::unordered_map<std::string, ComPtr<ID3DBlob>> mShaders;


	//XMVECTOR mEyePos = {0.0f, 0.0f, 0.0f};
	//XMFLOAT4X4 mView = MathHelper::Identity4x4();
	//XMFLOAT4X4 mProj = MathHelper::Identity4x4();

	//float mTheta = 1.5f*XM_PI;
	//float mPhi = 0.2f*XM_PI;
	//float mRadius = 15.0f;

	//POINT mLastMousePos;

};
