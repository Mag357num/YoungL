#pragma once

#include "../Common/D3DApp.h"
#include "../Common/DXSampleHelper.h"
#include "../Common/d3dUtil.h"
#include "FrameResource_CS.h"
#include "GPUWave.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

static const int gNumFrameResources_WaveCs = 3;

struct RenderItem_WaveCS
{
	RenderItem_WaveCS() = default;

	XMFLOAT4X4 World = MathHelper::Identity4x4();
	XMFLOAT4X4 TexTransform = MathHelper::Identity4x4();

	int NumFramesDirty = gNumFrameResources_WaveCs;
	UINT ObjCBIndex = -1;

	Material* Mat = nullptr;
	MeshGeometry* Geo = nullptr;

	D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	UINT IndexCount = 0;
	UINT StartIndexLocation = 0;
	int BaseVertexLocation = 0;

	//used for gpu waves render items.
	DirectX::XMFLOAT2 DispacementMapTexelSize = {1.0f, 1.0f};
	float GridSpatialStep = 1.0f;
};


enum class RenderLayer_WaveCS :int
{
	Opaque = 0,
	Transparent,
	AlphaTested,
	GPUWaves,
	Count
};

class WavesCsApp : public D3DApp
{
public:
	WavesCsApp(HINSTANCE hInstance);
	WavesCsApp(const WavesCsApp& rhs) = delete;
	WavesCsApp& operator=(const WavesCsApp&) = delete;
	~WavesCsApp();

	virtual bool Initialize()override;
	virtual void Update(const GameTimer& gt)override;
	virtual void Draw(const GameTimer& gt)override;

	virtual void OnMouseDown(WPARAM btnState, int x, int y)override;
	virtual void OnMouseUp(WPARAM btnState, int x, int y)override;
	virtual void OnMouseMove(WPARAM btnState, int x, int y)override;

private:
	void LoadTextures();
	
	void BuildRootSignature();
	void BuildWaveRootSignature();
	std::array<CD3DX12_STATIC_SAMPLER_DESC, 6> GetStaticSamples();

	void BuildDescriptorHeaps();
	void BuildShadersAndInputLayout();
	void BuildLandGeometry();
	void BuildWaveGeometry();
	void BuildBoxGeometry();
	void BuildPSOs();
	void BuildFrameResources();
	void BuildMaterials();
	void BuildRenderItems();

	void DrawRenderItems(ID3D12GraphicsCommandList* cmdList, std::vector<RenderItem_WaveCS*>& ritems);


	float GetHillsHeight(float X, float Z);
	XMFLOAT3 GetHillsNormal(float X, float Z)const;

private:
	void OnKeyboardInput(const GameTimer& gt);
	void UpdateCamer(const GameTimer& gt);

	void Animatematerials(const GameTimer& gt);
	void UpdateObjectCBs(const GameTimer& gt);
	void UpdateMateriCBs(const GameTimer& gt);
	void UpdateMainPassCBs(const GameTimer& gt);

	void UpdateWavesGPU(const GameTimer& gt);
	

private:
	std::vector<std::unique_ptr<FrameResource_CS>> mFrameResources;
	FrameResource_CS* mCurrentFrameResource;
	int currentFrameIndex;

	UINT mCbvSrvDescriptorSize;
	ComPtr<ID3D12DescriptorHeap> mSrvDescriptorHeap;

	ComPtr<ID3D12RootSignature> mRootSignature;
	ComPtr<ID3D12RootSignature> mWavesRootSignature;

	std::unordered_map<std::string, std::unique_ptr<Texture>> mTextures;
	std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> mGeometries;
	std::unordered_map<std::string, std::unique_ptr<Material>> mMaterials;
	std::unordered_map<std::string, ComPtr<ID3DBlob>> mShaders;
	std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> mPSOs;

	std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;

	std::vector <std::unique_ptr<RenderItem_WaveCS>> mAllrItems;
	std::vector<RenderItem_WaveCS*> mRitemLayers[(int)RenderLayer_WaveCS::Count];

	std::unique_ptr<GPUWave> mWaves;

	PassConstants_CS mMainPassCB;

	XMFLOAT3 mEyePos = {0.0f, 0.0f ,0.0f};
	XMFLOAT4X4 mView = MathHelper::Identity4x4();
	XMFLOAT4X4 mProj = MathHelper::Identity4x4();

	float mTheta = 1.5f * XM_PI;
	float mPhi = XM_PIDIV2 - 0.1f;
	float mRadius = 50.0f;

	POINT mLastMousePos;

};