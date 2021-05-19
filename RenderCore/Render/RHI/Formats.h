#pragma once

enum ERHIResourceState
{
	State_None = 0,
	State_Present,
	State_RenderTarget,
	State_GenerateRead,
	State_Uav,
	State_DepthRead,
	State_DepthWrite
};


enum EPixelBufferFormat
{
	PixelFormat_None = 0,
	//depth
	PixelFormat_R24G8_Typeless,
	PixelFormat_D24_UNORM_S8_UINT,
	PixelFormat_R8G8B8A8_Unorm,
	PixelFormat_R8G8B8A8_TypeLess,
	PixelFormat_R10G10B10A2_UNorm,
	PixelFormat_R16G16B16A16_Float,
	PixelFormat_R11G11B10_Float,
	PixelFormat_R32G32B32_Float,
	PixelFormat_R32G32_Float,
	PixelFormat_R32G32B32A32_Float,
	PixelFormat_R32G32B32A32_UINT
};

enum EResourcFlag
{
	Resource_None = 0,
	Resource_Allow_Render_Target,
	Resource_Allow_Depth_Stencil,
	Resource_Allow_Unordered_Access,

};

struct FColorResourceDesc
{
	int Width;
	int Height;

	ERHIResourceState ResourceState;
	EResourcFlag ResourceFlag;

	EPixelBufferFormat Format;
};