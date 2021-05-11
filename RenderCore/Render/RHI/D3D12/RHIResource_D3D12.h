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

		default:
			break;
		}

		return DXGI_FORMAT_UNKNOWN;
	}

	void Map(UINT SubResource, const D3D12_RANGE* ReadRange, void** Data);
	void UnMap(UINT SubResource, const D3D12_RANGE* WriteRange);

	ComPtr<ID3D12Resource> Resource;
private:
	

};

