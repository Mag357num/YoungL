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

	FParameterRange(EParameterRangeType InType, UINT InNumParameters, UINT InShaderRegister, 
		UINT InShaderRegisterSpace)
		:RangeType(InType),
		NumParameters(InNumParameters),
		ShaderRegister(InShaderRegister),
		ShaderRegisterSpace(InShaderRegisterSpace)
		{}

	EParameterRangeType RangeType;

	UINT NumParameters;
	UINT ShaderRegister;
	UINT ShaderRegisterSpace;
};

class FRHIShaderParameter
{
public:
	FRHIShaderParameter(EShaderParaType InParameterType, UINT InShaderRegister, 
	UINT InRegisterSpace, EShaderParaVisibility InShaderVisibility)
		:ShaderRegister(InShaderRegister),
		RegisterSpace(InRegisterSpace),
		ShaderVisibility(InShaderVisibility),
		ParameterType(InParameterType)
	{}
	~FRHIShaderParameter(){}
	
	void AddRangeTable(FParameterRange InRange)
	{
		RangeTables.push_back(InRange);
	}

	std::vector<FParameterRange>* GetRangeTables(){return &RangeTables;}

	EShaderParaType GetParameterType(){return ParameterType;}
	EShaderParaVisibility GetShaderVisibility() { return ShaderVisibility; }
	UINT GetShaderRegister() { return ShaderRegister; }
	UINT GetShaderRegisterSpace() { return RegisterSpace; }
	void SetNum32BitValues(UINT InNum){ Num32BitValues = InNum;}
	UINT GetNum32BitValues(){return Num32BitValues;}

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
