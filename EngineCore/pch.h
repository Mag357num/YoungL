#pragma once


#pragma warning(disable:4201)// nonstandard extension used : nameless struct/union
#pragma warning(disable:4238)//nonstandard extension used:class rvalue used as lvalue
#pragma warning(disable:4324)//structure was padded due to __declaspec(align())

#ifndef WIN32_LEAN_AND_MEAN
#define  WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>
#include <d3d12.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

#define MY_IID_PPV_ARGS IID_PPV_ARGS
#define D3D12_GPU_VIRTUAL_ADDRESS_NULL	((D3D12_GPU_VIRTUAL_ADDRESS)0)
#define D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN	((D3D12_GPU_VIRTUAL_ADDRESS)-1)

#include "d3dx12.h"

#include <cstdint>
#include <cstdio>
#include <cstdarg>
#include <vector>
#include <memory>
#include <string>
#include <exception>

#include <wrl.h>
#include <ppltasks.h>

//reserved for math and utility
#include "Misc/Utility.h"
