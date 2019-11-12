#pragma once

#include "../Common/d3dUtil.h"
#include "../Common/GameTimer.h"

class GPUWave
{
public:
	GPUWave(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, int m, int n, float dx, float dt, float speed, float damping);
	GPUWave(const GPUWave& rhs) = delete;
	GPUWave& operator=(const GPUWave& rhs) = delete;
	~GPUWave() = default;

	UINT RowCount()const;
	UINT ColumnCount()const;

	UINT VertexCount() const;
	UINT TriangleCount() const;
	float Width()const;
	float Depth()const;
	float SpatialStep()const;

	CD3DX12_GPU_DESCRIPTOR_HANDLE DisplacementMap()const;

	UINT DescriptorCount() const;

	void BuildResources(ID3D12GraphicsCommandList* cmdList);

	void BuildDescriptorHeap(
	CD3DX12_CPU_DESCRIPTOR_HANDLE hCpuDescriptor,
		CD3DX12_GPU_DESCRIPTOR_HANDLE hGpuDescriptor,
		UINT descriptorSize
	);

	void Update(const GameTimer& gt,
		ID3D12GraphicsCommandList* cmdList,
		ID3D12RootSignature* rootSig,
		ID3D12PipelineState* pso);

	void Disturb(
		ID3D12GraphicsCommandList* cmdList,
		ID3D12RootSignature* rootSig,
		ID3D12PipelineState* pso,
		UINT i,
		UINT j,
		float Magnitude
	);

private:
	UINT mNumRows;
	UINT mNumColumns;

	UINT mVertexCount;
	UINT mTriangleCount;

	//simulation constants we can precompute
	float mK[3];

	float mTimeStep;
	float mSpatialStep;

	ID3D12Device* md3dDevice = nullptr;

	CD3DX12_GPU_DESCRIPTOR_HANDLE mPrevSolSrv;
	CD3DX12_GPU_DESCRIPTOR_HANDLE mCurrSolSrv;
	CD3DX12_GPU_DESCRIPTOR_HANDLE mNextSolSrv;

	CD3DX12_GPU_DESCRIPTOR_HANDLE mPrevSolUav;
	CD3DX12_GPU_DESCRIPTOR_HANDLE mCurrSolUav;
	CD3DX12_GPU_DESCRIPTOR_HANDLE mNextSolUav;

	Microsoft::WRL::ComPtr<ID3D12Resource> mPrevSol = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> mCurrSol = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> mNextSol = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> mPrevUploadBuffer = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> mCurrUploadBuffer = nullptr;

};
