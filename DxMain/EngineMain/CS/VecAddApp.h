#pragma once
#include "../Common/D3DApp.h"
#include "FrameResource_CS.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

const int gNumFrameResource_VecAdd = 3;

struct Data
{
	XMFLOAT3 v1;
	XMFLOAT2 v2;
};

class VecAddApp : public D3DApp
{
public:
	VecAddApp(HINSTANCE hInstance);
	VecAddApp(const VecAddApp& rhs) = delete;
	VecAddApp& operator=(const VecAddApp& rhs) = delete;
	~VecAddApp();

	virtual bool Initialize()override;

	virtual void Draw(const GameTimer& gt)override;
	virtual void Update(const GameTimer& gt)override;
	virtual void OnResize()override;

private:

	void BuildBuffers();
	void BuildRootSignature();
	void BuildDescriptorHeaps();
	void BuildShadersAndInputLayout();
	void BuildPSOs();
	void BuildFrameResources();

	void DoComputerWork();

	std::array<D3D12_STATIC_SAMPLER_DESC, 6> GetStaticSamples();

private:
	std::vector<std::unique_ptr<FrameResource_CS>> mFrameResources;
	UINT mCurrFrameResourceIndex;
	FrameResource_CS* mCurrFrameResource;

	const int NumDataElements = 32;

	ComPtr<ID3D12Resource> InputVecBufferA;
	ComPtr<ID3D12Resource> InputVecBufferUploadA;
	ComPtr<ID3D12Resource> InputVecBufferB;
	ComPtr<ID3D12Resource> InputVecBufferUploadB;
	ComPtr<ID3D12Resource> OutputBuffer;
	ComPtr<ID3D12Resource> ReadBackBuffer;

	std::unordered_map<std::string, ComPtr<ID3DBlob>> mShaders;
	std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> mPSOs;

	ComPtr<ID3D12RootSignature> mRootSignature;

};
