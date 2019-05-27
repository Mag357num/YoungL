#pragma once

#include <windows.h>
#include <wrl.h>
#include <dxgi1_4.h>
#include <d3d12.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>
#include <DirectXCollision.h>
#include <string>
#include <memory>
#include <algorithm>
#include <vector>
#include <array>
#include <unordered_map>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <cassert>
#include "d3dx12.h"
//#include "DDSTextureLoader.h"
//#include "MathHelper.h"

extern const int gNumFrameResources;

inline void d3dSetDebugName(IDXGIObject* obj, const char* name)
{
	if (obj)
	{
		obj->SetPrivateData(WKPDID_D3DDebugObjectName, lstrlenA(name), name);
	}
}

inline void d3dSetDebugName(ID3D12Device* device, const char* name)
{
	if (device)
	{
		device->SetPrivateData(WKPDID_D3DDebugObjectName, lstrlenA(name), name);
	}
}

inline void d3dSetDebugName(ID3D12DeviceChild* obj, const char* name)
{
	if (obj)
	{
		obj->SetPrivateData(WKPDID_D3DDebugObjectName, lstrlenA(name), name);
	}
}

inline std::wstring AnsiToWstring(const std::string& Str)
{
	WCHAR buffer[512];
	MultiByteToWideChar(CP_ACP, 0, Str.c_str(), -1, buffer, 512);

	return std::wstring(buffer);
}

#if defined(_DEBUG)
	#ifndef Assert
	#define Assert(x, description)\
	{\
		static bool ignoreAssert = false;\
		if (!ignoreAssert && !(x))\
		{\
			Debug::AsertResult result = Debug::ShowAssertDialog((L#x), description, AnsiToWstring(__FILE__), __LINE__);\
			if(result = Debug::AssertIgnore)\
			{\
				ignoreAssert = true;\
			}\
			else if (result = Debug::AssertBreak)\
			{\
				__debugbreak();\
			}\
		}\
	}
	#endif
#else
	#ifndef Assert
	#define Assert(x, description)
	#endif // !Assert

#endif

class d3dUtil
{
public:
	static bool IskeyDown(int vKeyCode);
	static std::string  ToString(HRESULT hr);

	static UINT CalcConstantBufferByteSize(UINT byteSize)
	{
		// Constant buffers must be a multiple of the minimum hardware
		// allocation size (usually 256 bytes).  So round up to nearest
		// multiple of 256.  We do this by adding 255 and then masking off
		// the lower 2 bytes which store all bits < 256.
		// Example: Suppose byteSize = 300.
		// (300 + 255) & ~255
		// 555 & ~255
		// 0x022B & ~0x00ff
		// 0x022B & 0xff00
		// 0x0200
		// 512

		return (byteSize + 255) & ~255;
	}

	static Microsoft::WRL::ComPtr<ID3DBlob> LoadBinary(const std::wstring& filename);

	static Microsoft::WRL::ComPtr<ID3D12Resource> CreateaDefaultBuffer(
		ID3D12Device* device,
		ID3D12GraphicsCommandList* cmdList,
		const void* initdata,
		UINT64 byteSize,
		Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer
	);

	static Microsoft::WRL::ComPtr<ID3DBlob> CompileShader(
		const std::wstring& filename,
		const D3D_SHADER_MACRO* defines,
		const std::string& entrypoint,
		const std::string& target
	);
};

class DxException
{
public:
	DxException() = default;
	DxException(HRESULT hr, const std::wstring& functionname, const std::wstring& filename, int lineNumer);

	std::wstring ToString() const;

	HRESULT ErrorCode = S_OK;
	std::wstring FunctionName;
	std::wstring Filename;
	int LineNumer = -1;
};

// Defines a subrange of geometry in a MeshGeometry.  This is for when multiple
// geometries are stored in one vertex and index buffer.  It provides the offsets
// and data needed to draw a subset of geometry stores in the vertex and index 
// buffers so that we can implement the technique described by Figure 6.3.

struct SubmeshGeometry
{
	UINT IndexCount = 0;
	UINT StartIndexLocation = 0;
	INT BaseVertexLocation = 0;

	//Bounding box of the geometry defined by this submesh
	DirectX::BoundingBox Bounds;
};

struct MeshGeometry
{
	std::string Name;

	Microsoft::WRL::ComPtr<ID3DBlob> VertexBufferCPU = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> IndexBufferCPU = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> VertexBufferGPU = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> IndexBufferGPU = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> VertexBufferUpload = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> IndexBufferUpload = nullptr;

	UINT VertextByteStride = 0;
	UINT VertextBufferByteSize = 0;
	DXGI_FORMAT IndexFormat = DXGI_FORMAT_R16_UINT;
	UINT IndexBufferByteSize = 0;

	std::unordered_map<std::string, SubmeshGeometry> DrawArgs;

	D3D12_VERTEX_BUFFER_VIEW VertexBufferView() const
	{
		D3D12_VERTEX_BUFFER_VIEW vbv;
		vbv.BufferLocation = VertexBufferGPU->GetGPUVirtualAddress();
		vbv.StrideInBytes = VertextByteStride;
		vbv.SizeInBytes = VertextBufferByteSize;

		return vbv;
	}

	D3D12_INDEX_BUFFER_VIEW IndexBufferView() const
	{
		D3D12_INDEX_BUFFER_VIEW ibv;
		ibv.BufferLocation = IndexBufferGPU->GetGPUVirtualAddress();
		ibv.Format = IndexFormat;
		ibv.SizeInBytes = IndexBufferByteSize;

		return ibv;
	}

	void DisposeUploaders()
	{
		VertexBufferUpload = nullptr;
		IndexBufferUpload = nullptr;
	}
};

#ifndef ReleaseCom
#define ReleaseCom(x){if(x){x->Release(); x=0;}}
#endif // !ReleaseCom