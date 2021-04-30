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
	PixelFormat_R8G8B8A8_Unorm,
	PixelFormat_R10G10B10A2_UNorm
};
