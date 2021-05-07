#pragma once

enum EShaderType
{
	ShaderType_None = 0,
	ShaderType_VS,
	ShaderType_PS
};

class IRHIShader
{
public:
	IRHIShader() {}
	virtual ~IRHIShader() {

	}

	void SetShaderPath(std::wstring InPath){ShaderPath = InPath;}
	void SetShaderType(EShaderType InType){ShaderType = InType;}

	virtual void CompileShader(){}

	std::wstring GetShaderPath(){return ShaderPath;}

protected:

	EShaderType ShaderType;
	std::wstring ShaderPath;

	std::vector<std::string> MacroDefines;
};
