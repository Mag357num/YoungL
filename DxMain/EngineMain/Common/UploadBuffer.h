#pragma once
#include "d3dUtil.h"
#include "DXSampleHelper.h"

template<typename T>
class UploadBuffer
{
public:
	UploadBuffer(ID3D12Device* device, UINT elementCount, bool isContantBuffer)
		:mIsConstantBuffer(isContantBuffer);
	{
		mElementByteSize = sizeof(T);

		if (isContantBuffer)
		{
			mElementByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(T));
		}

		ThrowIfFailed(device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(mElementByteSize*elementCount),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&mUploadBuffer)
		));

		ThrowIfFailed(mUploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&mMappeddata)));
	}

	UploadBuffer(const UploadBuffer& rhs) = delete;
	UploadBuffer operator=(const UploadBuffer& rhs) = delete;
	~UploadBuffer() {
		if (mUploadBuffer!=nullptr)
		{
			mUploadBuffer->Unmap(0, nullptr);
		}
		mMappeddata = nullptr;
	}

	ID3D12Resource* Resource()const
	{
		return mUploadBuffer.Get();
	}

	void CopyData(int elementIndex, const T& data)
	{
		memcpy(&mMappeddata[elementIndex*mElementByteSize], &data, sizeof(T));
	}

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> mUploadBuffer;
	BYTE* mMappeddata = nullptr;

	UINT mElementByteSize = 0;
	bool mIsConstantBuffer = false;

};
