#pragma once

#include "../Common/D3DApp.h"
#include "../Common/d3dUtil.h"
#include "BlendApp.h"

enum class RenderLayer_GeometryShader : int
{
	Opaque = 0,
	Transparent,
	AlphaTested,
	AlphaTestedTreeSprites,
	Count
};

class GeometryShaderApp : public D3DApp
{
public:
	GeometryShaderApp(HINSTANCE hInstance);
	GeometryShaderApp(const GeometryShaderApp& rhs) = delete;
	GeometryShaderApp& operator=(const GeometryShaderApp& rhs) = delete;
	~GeometryShaderApp();

	virtual bool Initialize()override;

	virtual void Draw(const GameTimer& gt)override;

	virtual void OnResize()override;

	virtual void Update(const GameTimer& gt)override;

	virtual void OnMouseDown(WPARAM btnState, int x, int y)override;
	virtual void OnMouseUp(WPARAM btnState, int x, int y)override;
	virtual void OnMouseMove(WPARAM btnState, int x, int y)override;
	
private:
	void LoadTextures();
	void BuildRootSignature();
	void BuildDescriptorHeaps();
	void BuildShadersInputAndLayout();
	void BuildLandGeometry();
	void BuildWaveGeometry();
	void BuildBoxGeometry();
	void BuildBillTreeGeometry();
	void BuildMaterials();
	void BuildRenderItems();
	void BuildFrameResources();
	void BuildPSOs();

	float GetHillsHeight(float x, float z)const;
	XMFLOAT3 GetHillsNormal(float x, float z)const;
	std::array<CD3DX12_STATIC_SAMPLER_DESC, 6> GetStaticSamples();

private:
	void DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem_Blend*>& rItems);

	void OnKeyboardInput(const GameTimer& gt);
	void UpdateCamera(const GameTimer& gt);
	void AnimateMaterials(const GameTimer& gt);
	void UpdateObjectCBs(const GameTimer& gt);
	void UpdatematerialCBs(const GameTimer& gt);
	void UpdateMainPassCBs(const GameTimer& gt);
	void UpdateWaves(const GameTimer& gt);

private:
	std::vector<std::unique_ptr<FrameResource_Blend>> mFrameResources;
	FrameResource_Blend* mCurrFrameResource = nullptr;
	int mCurrFrameResourceIndex = 0;

	UINT mCbvSrvDescriptorSize = 0;
	ComPtr<ID3D12RootSignature> mRootSignature = nullptr;
	ComPtr<ID3D12DescriptorHeap> mSrvDescriptorHeap = nullptr;

	std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> mGeometries;
	std::unordered_map<std::string, std::unique_ptr<Material>> mMaterials;
	std::unordered_map<std::string, std::unique_ptr<Texture>> mTextures;
	std::unordered_map<std::string, ComPtr<ID3DBlob>> mShaders;
	std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> mPSOs;

	std::vector<D3D12_INPUT_ELEMENT_DESC> mStdInputLayout;
	std::vector<D3D12_INPUT_ELEMENT_DESC> mTreeSpriteInputLayout;

	RenderItem_Blend* mWavesRitem = nullptr;

	std::vector<std::unique_ptr<RenderItem_Blend>> mAllRitems;

	std::vector<RenderItem_Blend*> mRitemLayer[(int)RenderLayer_GeometryShader::Count];

	std::unique_ptr<Waves> mWaves;
	
	PassConstants_Blend mMainPassCB;

	XMFLOAT3 mEyePos = {0.0f, 0.0f, 0.0f};
	XMFLOAT4X4 mView = MathHelper::Identity4x4();
	XMFLOAT4X4 mProj = MathHelper::Identity4x4();

	float mTheta = 1.5f*XM_PI;
	float mPhi = XM_PIDIV2 - 0.1f;
	float mRadius = 50.0f;

	POINT mLastMousePos;

};