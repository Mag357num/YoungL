#include "pch.h"
#include "RHIShaderResource_D3D12.h"

namespace ShaderMap
{
	std::map<std::wstring, D3D12_SHADER_BYTECODE> GlobalShaderMap;
}


using namespace ShaderMap;

FRHIShaderResource_D3D12::FRHIShaderResource_D3D12()
{
	D3D12_SHADER_BYTECODE ScreenVS;
	ScreenVS.BytecodeLength = sizeof(g_ScreenVS);
	ScreenVS.pShaderBytecode = g_ScreenVS;
	GlobalShaderMap.insert(std::make_pair(L"ScreenVS", ScreenVS));

	D3D12_SHADER_BYTECODE ScreenPS;
	ScreenPS.BytecodeLength = sizeof(g_ScreenPS);
	ScreenPS.pShaderBytecode = g_ScreenPS;
	GlobalShaderMap.insert(std::make_pair(L"ScreenPS", ScreenPS));

	D3D12_SHADER_BYTECODE BloomSetUpPS;
	BloomSetUpPS.BytecodeLength = sizeof(g_BloomSetUpPS);
	BloomSetUpPS.pShaderBytecode = g_BloomSetUpPS;
	GlobalShaderMap.insert(std::make_pair(L"BloomSetUpPS", BloomSetUpPS));
}


FRHIShaderResource_D3D12::~FRHIShaderResource_D3D12() {
	//for (auto It = ShaderMap.begin(); It != ShaderMap.end(); ++It)
	//{
	//	delete It->second;
	//}

	if (!GlobalShaderMap.empty())
	{

	}

}
