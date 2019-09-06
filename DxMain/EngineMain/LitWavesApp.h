#pragma once

#include "Common/D3DApp.h"
#include "Common/MathHelper.h"
#include "Common/UploadBuffer.h"
#include "Common/GeometryGenerator.h"
#include "Common/FrameResource_LitWave.h"
#include "Common/FrameResource.h"
#include "Waves.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;

const int gNumFrameResources_LitWav = 3;

struct RenderItem_LitWave
{
	RenderItem_LitWave() = default;

	//world matrix of the shape that describes the object's local space
	//relative to the world space
	XMFLOAT4X4 World = MathHelper::Identity4x4();

	XMFLOAT4X4 TexTransform = MathHelper::Identity4x4();

	//dirty flag indicating that theobject data has changed and we need to update the constant buffer
	//because we have an object cbuffer for each frameresource, we have to apply the update to each frameresource
	//thus ,when we modify object data we should set numFramesDirty =gNumFrameResources_LitWav so that each frame resource gets the update.
	int NumFramesDirty = gNumFrameResources_LitWav;

	//index into gpu constant buffer corresponding to the objectCB for this render item
	UINT ObjCBIndex = -1;

	Material* Mat = nullptr;
	MeshGeometry* Geo = nullptr;

	//primitive topology
	D3D12_PRIMITIVE_TOPOLOGY Primitivetype = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	//draw indexed instanced paramater;
	UINT IndexCount = 0;
	UINT StartIndexLocation = 0;
	int BaseVertexLocation = 0;
};

enum class RenderLayer_LitWav :int
{
	Opaque = 0,
	Count

};

class LitWaveApp : public D3DApp
{
public:
	LitWaveApp(HINSTANCE hInstance);
	LitWaveApp(const LitWaveApp& rhs) = delete;
	LitWaveApp& operator=(const LitWaveApp& rhs) = delete;
	~LitWaveApp();

	virtual bool Initialize()override;
private:

	virtual void OnMouseDown(WPARAM btnState, int x, int y)override;
	virtual void OnMouseMove(WPARAM btnState, int x, int y)override;
	virtual void OnMouseUp(WPARAM btnState, int x, int y)override;

	void OnKeyboardInput(const GameTimer& gt);

	virtual void OnResize()override;
	
	virtual void Update(const GameTimer& gt)override;
	void UpdateCamera(const GameTimer& gt);
	void UpdateObjectCB(const GameTimer& gt);
	void UpdateMainPassCB(const GameTimer& gt);
	void UpdateMaterialCB(const GameTimer& gt);
	void UpdateWaves(const GameTimer& gt);

	void BuildRootSignature();
	void BuildShadersAndInputLayout();
	void BuildLandGeometry();
	void BuildWavesGeometryBuffers();
	void BuildPSOs();
	void BuildFrameResources();
	void BuildMaterials();
	void BuildRenderItems();

	float GetHillsHeight(float x, float z)const;
	XMFLOAT3 GetHillsNormal(float x, float z)const;

	virtual void Draw(const GameTimer& gt)override;
	void DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem_LitWave*>& rItems);

private:
	std::vector<std::unique_ptr<FrameResource_LitWave>> mFrameResources;
	FrameResource_LitWave* mCurrFrameResource = nullptr;
	int mCurrFrameResourceIndex = 0;

	UINT mCbvSrvDescriptorSize = 0;

	ComPtr<ID3D12RootSignature> mRootSignature = nullptr;

	std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> mGeometries;
	std::unordered_map<std::string, ComPtr<ID3DBlob>> mShaders;
	std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> mPSOs;

	std::unordered_map<std::string, std::unique_ptr<Material>> mMaterials;
	std::unordered_map<std::string, std::unique_ptr<Texture>> mTextures;

	std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;

	RenderItem_LitWave* mWavesRitem = nullptr;

	//list of all the render items.
	std::vector<std::unique_ptr<RenderItem_LitWave>> mAllRitems;

	//render items divied by pso.
	std::vector<RenderItem_LitWave*> mRitemLayer[(int)RenderLayer_LitWav::Count];

	std::unique_ptr<Waves> mWaves;

	PassConstants_LitWave mMainPassCB;

	XMFLOAT3 mEyePos = {0.0f, 0.0f, 0.0f};
	XMFLOAT4X4 mView = MathHelper::Identity4x4();
	XMFLOAT4X4 mProj = MathHelper::Identity4x4();

	float mTheta = 1.5f*XM_PI;
	float mPhi = XM_PIDIV2 - 0.1f;
	float mRadius = 50.0f;

	float mSunTheta = 1.25f*XM_PI;
	float mSunPhi = XM_PIDIV2;

	POINT mLastMousePos;

};

