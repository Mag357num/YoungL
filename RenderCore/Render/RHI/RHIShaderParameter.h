#pragma once

enum EShaderParaVisibility
{
	Visibility_None = 0,
	Visibility_VS,
	Visibility_PS,
	Visibility_All
};

enum EShaderParaType
{
	ParaType_None = 0,
	ParaType_Constant,
	ParaType_CBV,
	ParaType_SRV,
	ParaType_UAV,
	ParaType_Range
};

enum EParameterRangeType
{
	RangeType_None,
	RangeType_SRV,
	RangeType_UAV,
	RangeType_CBV,
	RangeType_Sampler
};

struct FParameterRange
{

	FParameterRange(){}

	EParameterRangeType RangeType;

	UINT NumParameters;
	UINT ShaderRegister;
	UINT ShaderRegisterSpace;
	UINT RangeTableStart;
};

class FRHIShaderParameter
{
public:
	FRHIShaderParameter(){}
	~FRHIShaderParameter(){}
	
private:

protected:

	UINT ShaderRegister;
	UINT RegisterSpace;
	
	EShaderParaVisibility ShaderVisibility;
	EShaderParaType ParameterType;

	//used for constant value(constant type)
	UINT Num32BitValues;

	//used for range table type
	std::vector<FParameterRange> RangeTables;
};
