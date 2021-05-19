#pragma once

#include "../RHIResource.h"
#include <d3d12.h>
#include <wrl.h>
#include "../Formats.h"


using namespace Microsoft::WRL;

class FRHIResource_D3D12 : public IRHIResource
{
public:
	FRHIResource_D3D12(){}
	virtual ~FRHIResource_D3D12(){
		if (Resource != nullptr)
		{
			Resource.Reset();
		}

		if (UploadResource != nullptr)
		{
			UploadResource.Reset();
		}
		
	}

	static DXGI_FORMAT TranslateFormat(EPixelBufferFormat InFormat)
	{
		switch (InFormat)
		{
		case PixelFormat_None:
			return DXGI_FORMAT_UNKNOWN;
			break;
		case PixelFormat_R24G8_Typeless:
			return DXGI_FORMAT_R24G8_TYPELESS;
			break;

		case PixelFormat_D24_UNORM_S8_UINT:
			return DXGI_FORMAT_D24_UNORM_S8_UINT;
			break;
		case PixelFormat_R8G8B8A8_Unorm:
			return DXGI_FORMAT_R8G8B8A8_UNORM;
			break;

		case PixelFormat_R8G8B8A8_TypeLess:
			return DXGI_FORMAT_R8G8B8A8_TYPELESS;
			break;

		case PixelFormat_R10G10B10A2_UNorm:
			return DXGI_FORMAT_R10G10B10A2_UNORM;
			break;

		case PixelFormat_R16G16B16A16_Float:
			return DXGI_FORMAT_R16G16B16A16_FLOAT;
			break;

		case PixelFormat_R11G11B10_Float:
			return DXGI_FORMAT_R11G11B10_FLOAT;
			break;

		case PixelFormat_R32G32B32_Float:
			return DXGI_FORMAT_R32G32B32_FLOAT;
			break;

		case PixelFormat_R32G32_Float:
			return DXGI_FORMAT_R32G32_FLOAT;
			break;

		case PixelFormat_R32G32B32A32_Float:
			return DXGI_FORMAT_R32G32B32A32_FLOAT;
			break;

		case PixelFormat_R32G32B32A32_UINT:
			return DXGI_FORMAT_R32G32B32A32_UINT;
			break;

		default:
			break;
		}

		return DXGI_FORMAT_UNKNOWN;
	}

	static D3D12_RESOURCE_FLAGS TranslateResourceFlag(EResourcFlag InFlag)
	{
		D3D12_RESOURCE_FLAGS Ret = D3D12_RESOURCE_FLAG_NONE;
		switch (InFlag)
		{
		case Resource_None:
			Ret = D3D12_RESOURCE_FLAG_NONE;
			break;
		case Resource_Allow_Render_Target:
			Ret = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
			break;
		case Resource_Allow_Unordered_Access:
			Ret = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
			break;
		default:
			break;
		}

		return Ret;
	}

	void Map(UINT SubResource, const D3D12_RANGE* ReadRange, void** Data);
	void UnMap(UINT SubResource, const D3D12_RANGE* WriteRange);

	ComPtr<ID3D12Resource> Resource;
	ComPtr<ID3D12Resource> UploadResource;
private:
	

};

