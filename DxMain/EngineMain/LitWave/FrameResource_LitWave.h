#pragma once

#include "../Common/d3dUtil.h"
#include "../Common/MathHelper.h"
#include "../Common/UploadBuffer.h"
#include "../Common/FrameResource.h"

struct PassConstants_LitWave
{
	DirectX::XMFLOAT4X4 View = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 InvView = MathHelper::Identity4x4();

	DirectX::XMFLOAT4X4 Proj = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 InvProj = MathHelper::Identity4x4();

	DirectX::XMFLOAT4X4 ViewProj = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 InvViewProj = MathHelper::Identity4x4();

	DirectX::XMFLOAT3 EyePosw = { 0.0f, 0.0f, 0.0f };

	float cbPerObjectPad1 = 0.0f;

	DirectX::XMFLOAT2 RenderTargetSize = { 0.0f, 0.0f };
	DirectX::XMFLOAT2 InvRenderTargetSize = { 0.0f, 0.0f };

	float NearZ = 0.0f;
	float FarZ = 0.0f;
	float TotalTime = 0.0f;
	float DeltaTime = 0.0f;

	DirectX::XMFLOAT4 AmbientLight = {0.0f, 0.0f, 0.0f, 1.0f};
	Light Lights[MaxLights];
};

struct Vertex_LitWave
{
	DirectX::XMFLOAT3 Pos;
	DirectX::XMFLOAT3 Normal;
};

//stores the resource need for cpu to build the command lists for a frame
struct FrameResource_LitWave
{
public:
	FrameResource_LitWave(ID3D12Device* Device, UINT passCount, UINT objectCount, UINT MaterialCount, UINT WaveVertCount);
	FrameResource_LitWave(const FrameResource_LitWave& rhs) = delete;
	FrameResource_LitWave& operator=(const FrameResource_LitWave& rhs) = delete;
	~FrameResource_LitWave();

	//we cannot reset the allocator until the GPU is done procesing the commands.
	// so each frame needs their own allocator.
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CmdListAlloc;

	//we cannot update a cbuffer until the GPU is done processing the commands.
	//that reference it . so each frame needs their own cbuffers.
	std::unique_ptr<UploadBuffer<PassConstants_LitWave>> PassCB = nullptr;
	std::unique_ptr<UploadBuffer<ObjectConstants>> ObjectCB = nullptr;

	//we cannot update  a dynamic vertex buffer until the gpu is done processing
	//the commands that reference it. so each frame needs their own.
	std::unique_ptr<UploadBuffer<Vertex_LitWave>> WavesVB = nullptr;

	std::unique_ptr<UploadBuffer<MaterialConstants>> MaterialCB = nullptr;

	//fence value to mark commands up to the fence point. this lets up 
	//check if these frame resources are still in use by the GPU
	UINT64 Fence = 0;

};
