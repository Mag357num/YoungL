#pragma once


enum ESamplerFilter
{
	Filter_None= 0,
	Filter_MIN_MAG_MIP_POINT,
	Filter_MIN_MAG_POINT_MIP_LINEAR,
	Filter_MIN_POINT_MAG_LINEAR_MIP_POINT,
	Filter_MIN_POINT_MAG_MIP_LINEAR,
	Filter_MIN_LINEAR_MAG_MIP_POINT,
	Filter_MIN_LINEAR_MAG_POINT_MIP_LINEAR,
	Filter_MIN_MAG_LINEAR_MIP_POINT
	//MIN_MAG_MIP_LINEAR = 0x15,
	//ANISOTROPIC = 0x55,
	//COMPARISON_MIN_MAG_MIP_POINT = 0x80,
	//COMPARISON_MIN_MAG_POINT_MIP_LINEAR = 0x81,
	//COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT = 0x84,
	//COMPARISON_MIN_POINT_MAG_MIP_LINEAR = 0x85,
	//COMPARISON_MIN_LINEAR_MAG_MIP_POINT = 0x90,
	//COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR = 0x91,
	//COMPARISON_MIN_MAG_LINEAR_MIP_POINT = 0x94,
	//COMPARISON_MIN_MAG_MIP_LINEAR = 0x95,
	//COMPARISON_ANISOTROPIC = 0xd5,
	//MINIMUM_MIN_MAG_MIP_POINT = 0x100,
	//MINIMUM_MIN_MAG_POINT_MIP_LINEAR = 0x101,
	//MINIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT = 0x104,
	//MINIMUM_MIN_POINT_MAG_MIP_LINEAR = 0x105,
	//MINIMUM_MIN_LINEAR_MAG_MIP_POINT = 0x110,
	//MINIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR = 0x111,
	//MINIMUM_MIN_MAG_LINEAR_MIP_POINT = 0x114,
	//MINIMUM_MIN_MAG_MIP_LINEAR = 0x115,
	//MINIMUM_ANISOTROPIC = 0x155,
	//MAXIMUM_MIN_MAG_MIP_POINT = 0x180,
	//MAXIMUM_MIN_MAG_POINT_MIP_LINEAR = 0x181,
	//MAXIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT = 0x184,
	//MAXIMUM_MIN_POINT_MAG_MIP_LINEAR = 0x185,
	//MAXIMUM_MIN_LINEAR_MAG_MIP_POINT = 0x190,
	//MAXIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR = 0x191,
	//MAXIMUM_MIN_MAG_LINEAR_MIP_POINT = 0x194,
	//MAXIMUM_MIN_MAG_MIP_LINEAR = 0x195,
	//MAXIMUM_ANISOTROPIC = 0x1d5
};




enum ESampleAddress
{
	Address_None = 0,
	ADDRESS_MODE_WRAP = 1,
	ADDRESS_MODE_MIRROR = 2,
	ADDRESS_MODE_CLAMP = 3,
	ADDRESS_MODE_BORDER = 4,
	ADDRESS_MODE_MIRROR_ONCE = 5
};

class FRHISamplerState
{
public:
	FRHISamplerState(UINT InShaderRegister, UINT InRegisterSpace, ESamplerFilter InFilter,
		ESampleAddress InAddressU, ESampleAddress InAddressV, ESampleAddress InAddressW)
		:ShaderRegister(InShaderRegister),
		RegisterSpace(InRegisterSpace),
		FilterType(InFilter),
		AddressU(InAddressU),
		AddressV(InAddressV),
		AddressW(InAddressW)
	{}


	~FRHISamplerState(){}

	UINT GetShaderRegister(){return ShaderRegister;}
	UINT GetRegisterSpace() { return RegisterSpace; }
	ESamplerFilter GetSamplerFiler() { return FilterType; }
	ESampleAddress GetAddressU(){return AddressU;}
	ESampleAddress GetAddressV() { return AddressV; }
	ESampleAddress GetAddressW() { return AddressW; }

protected:
	UINT ShaderRegister;
	UINT RegisterSpace;

	ESamplerFilter FilterType;
	ESampleAddress AddressU;
	ESampleAddress  AddressV;
	ESampleAddress AddressW;

	float MipLODBias;
	UINT MaxAnisotropy;

private:

};
