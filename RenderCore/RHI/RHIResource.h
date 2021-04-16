#pragma once

enum ERHIResourceState
{
	State_None = 0,
	State_Present,
	State_RenderTarget,
	State_Srv,
	State_Uav,
	State_DepthRead,
	State_DepthWrite
};


enum EPixelBufferFormat
{
	PixelFormat_None = 0,
	//depth
	PixelFormat_D24S8
};

class IRHIResource
{
public:
	IRHIResource() {}
	virtual ~IRHIResource() {}
protected:
private:
};
