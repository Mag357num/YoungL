#pragma once

enum ESampleFilter
{
	Filter_None= 0,

};

enum ESampleAddress
{
	Address_None = 0,

};

class FRHISamplerState
{
public:
	FRHISamplerState(){}
	~FRHISamplerState(){}



protected:
	UINT ShaderRegister;
	ESampleFilter FilterType;
	ESampleAddress AddressU;
	ESampleAddress AddressV;
	ESampleAddress AddressW;

	float MipLODBias;
	UINT MaxAnisotropy;

private:

};
