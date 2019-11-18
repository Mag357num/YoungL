#pragma once

#include "../Common/UploadBuffer.h"
#include "../Common/d3dUtil.h"
#include "../Common/MathHelper.h"

struct ObjectConstans_Tessellation
{
	DirectX::XMFLOAT4X4 World = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 TexTransform = MathHelper::Identity4x4();
};

struct PassConstants_Tessellation
{
	DirectX::XMFLOAT4X4 View = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 InvView = MathHelper::Identity4x4();

	DirectX::XMFLOAT4X4 Proj = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 InvProj = MathHelper::Identity4x4();

	DirectX::XMFLOAT4X4 ViewProj = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 InvViewProj = MathHelper::Identity4x4();

	DirectX::XMFLOAT3 EyePosW = {0.0f, 0.0f, 0.0f};
	float cbPerObjectPad = 0.0f;
	DirectX::XMFLOAT2 RenderTargetSize = {0.0f, 0.0f};
	DirectX::XMFLOAT2 InvRenderTargetSize = {0.0f, 0.0f};
	float NearZ = 0.0f;
	float FarZ = 0.0f;
	float TotalTime = 0.0f;
	float DeltaTime = 0.0f;

	DirectX::XMFLOAT4 AmbientLight = {0.0f, 0.0f, 0.0f, 1.0f};

	DirectX::XMFLOAT4 FogColor = {0.7f, 0.7f, 0.7f, 1.0f};
	float gFogStart = 5.0f;
	float gFogEnd = 150.0f;
	DirectX::XMFLOAT2 cbPerObjectPad2;

	Light Lights[MaxLights];
};

struct Vertex_Tessellation
{
	Vertex_Tessellation() = default;
	Vertex_Tessellation(float x, float y, float z, float nx, float ny, float nz, float u, float v) :
		Pos(x, y, z),
		Normal(nx, ny, nz),
		TexC(u, v) {}

	DirectX::XMFLOAT3 Pos;
	DirectX::XMFLOAT3 Normal;
	DirectX::XMFLOAT2 TexC;

};

struct FrameResource_Tessellation
{
	FrameResource_Tessellation(ID3D12Device* mDevice, UINT passCount, UINT objCount, UINT materialCount);
	FrameResource_Tessellation(const FrameResource_Tessellation& rhs) = delete;
	FrameResource_Tessellation& operator=(const FrameResource_Tessellation& rhs) = delete;
	~FrameResource_Tessellation();

	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> cmdListAlloc;

	std::unique_ptr<UploadBuffer<PassConstants_Tessellation>> FrameCB = nullptr;
	std::unique_ptr<UploadBuffer<ObjectConstans_Tessellation>> ObjectCB = nullptr;
	std::unique_ptr<UploadBuffer<MaterialConstants>> MaterialCB = nullptr;

	UINT64 Fence = 0;
};