#include "pch.h"
#include "FRHIShader_D3D12.h"

#include <D3Dcompiler.h>
#pragma comment(lib, "D3DCompiler.lib")


ComPtr<ID3DBlob> FRHIShader_D3D12::CompileShader(
	const std::wstring& filename,
	const D3D_SHADER_MACRO* defines,
	const std::string& entrypoint,
	const std::string& target)
{
	UINT compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)  
	compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	HRESULT hr = S_OK;

	ComPtr<ID3DBlob> byteCode = nullptr;
	ComPtr<ID3DBlob> errors;
	hr = D3DCompileFromFile(filename.c_str(), defines, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		entrypoint.c_str(), target.c_str(), compileFlags, 0, &byteCode, &errors);

	if (errors != nullptr)
		OutputDebugStringA((char*)errors->GetBufferPointer());

	//ThrowIfFailed(hr);

	return byteCode;
}

void FRHIShader_D3D12::CompileShader()
{
	//const D3D_SHADER_MACRO alphaTestDefines[] =
	//{
	//	"ALPHA_TEST", "1",
	//		NULL, NULL
	//};
	

	//shader path example
	//L"Shaders\\Default.hlsl"
	if (ShaderType == ShaderType_VS)
	{
		CompiledShader = CompileShader(ShaderPath, nullptr, "main", "vs_5_1");
	}
	else if (ShaderType == ShaderType_PS)
	{
		CompiledShader = CompileShader(ShaderPath, nullptr, "main", "ps_5_1");
	}
	else
	{

	}

}