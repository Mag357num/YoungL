#pragma once

#include "../Common/d3dUtil.h"
#include "../Common/MathHelper.h"
#include "../Common/UploadBuffer.h"
#include "../Common/FrameResource.h"

struct ObjectConstants_CS
{
	DirectX::XMFLOAT4X4 World = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 TexTransform = MathHelper::Identity4x4();
};


struct PassConstants_CS
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

	DirectX::XMFLOAT4 AmbientLight = { 0.0f, 0.0f, 0.0f, 1.0f };

	DirectX::XMFLOAT4 FogColor = { 0.7f, 0.7f, 0.7f, 1.0f };
	float gFogStart = 5.0f;
	float gFogRange = 150.0f;
	DirectX::XMFLOAT2 cbPerObjectPad2;

	Light Lights[MaxLights];
};

struct Vertex_CS
{
	DirectX::XMFLOAT3 Pos;
	DirectX::XMFLOAT3 Normal;
	DirectX::XMFLOAT2 TexC;

};

//stores the resource need for cpu to build the command lists for a frame
struct FrameResource_CS
{
public:
	FrameResource_CS(ID3D12Device* Device, UINT passCount, UINT objectCount, UINT MaterialCount, UINT WaveVertCount);
	FrameResource_CS(const FrameResource_CS& rhs) = delete;
	FrameResource_CS& operator=(const FrameResource_CS& rhs) = delete;
	~FrameResource_CS();

	//we cannot reset the allocator until the GPU is done procesing the commands.
	// so each frame needs their own allocator.
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CmdListAlloc;

	//we cannot update a cbuffer until the GPU is done processing the commands.
	//that reference it . so each frame needs their own cbuffers.
	std::unique_ptr<UploadBuffer<PassConstants_CS>> PassCB = nullptr;
	std::unique_ptr<UploadBuffer<ObjectConstants_CS>> ObjectCB = nullptr;

	//we cannot update  a dynamic vertex buffer until the gpu is done processing
	//the commands that reference it. so each frame needs their own.
	std::unique_ptr<UploadBuffer<Vertex_CS>> WavesVB = nullptr;

	std::unique_ptr<UploadBuffer<MaterialConstants>> MaterialCB = nullptr;

	//fence value to mark commands up to the fence point. this lets up 
	//check if these frame resources are still in use by the GPU
	UINT64 Fence = 0;

};
