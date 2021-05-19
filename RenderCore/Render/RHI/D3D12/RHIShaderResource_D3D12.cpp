#include "pch.h"
#include "RHIShaderResource_D3D12.h"


namespace ShaderMap
{
	std::map<std::wstring, D3D12_SHADER_BYTECODE> GlobalShaderMap;

	std::map<std::wstring, D3D12_SHADER_BYTECODE> GlobalCSShaderMap;
}


using namespace ShaderMap;

FRHIShaderResource_D3D12::FRHIShaderResource_D3D12()
{

	D3D12_SHADER_BYTECODE SkinnedMeshVS;
	SkinnedMeshVS.BytecodeLength = sizeof(g_SkinnedMeshVS);
	SkinnedMeshVS.pShaderBytecode = g_SkinnedMeshVS;
	GlobalShaderMap.insert(std::make_pair(L"SkinnedMeshVS", SkinnedMeshVS));

	D3D12_SHADER_BYTECODE SkinnedMeshPS;
	SkinnedMeshPS.BytecodeLength = sizeof(g_SkinnedMeshPS);
	SkinnedMeshPS.pShaderBytecode = g_SkinnedMeshPS;
	GlobalShaderMap.insert(std::make_pair(L"SkinnedMeshPS", SkinnedMeshPS));

	D3D12_SHADER_BYTECODE BasePassVS;
	BasePassVS.BytecodeLength = sizeof(g_BasePassVS);
	BasePassVS.pShaderBytecode = g_BasePassVS;
	GlobalShaderMap.insert(std::make_pair(L"BasePassVS", BasePassVS));

	D3D12_SHADER_BYTECODE BasePassPS;
	BasePassPS.BytecodeLength = sizeof(g_BasePassPS);
	BasePassPS.pShaderBytecode = g_BasePassPS;
	GlobalShaderMap.insert(std::make_pair(L"BasePassPS", BasePassPS));

	
	D3D12_SHADER_BYTECODE InstancePassVS;
	InstancePassVS.BytecodeLength = sizeof(g_InstancePassVS);
	InstancePassVS.pShaderBytecode = g_InstancePassVS;
	GlobalShaderMap.insert(std::make_pair(L"InstancePassVS", InstancePassVS));

	D3D12_SHADER_BYTECODE DepthVS;
	DepthVS.BytecodeLength = sizeof(g_DepthVS);
	DepthVS.pShaderBytecode = g_DepthVS;
	GlobalShaderMap.insert(std::make_pair(L"DepthVS", DepthVS));

	D3D12_SHADER_BYTECODE DepthPS;
	DepthPS.BytecodeLength = sizeof(g_DepthPS);
	DepthPS.pShaderBytecode = g_DepthPS;
	GlobalShaderMap.insert(std::make_pair(L"DepthPS", DepthPS));


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

	D3D12_SHADER_BYTECODE BloomDownPS;
	BloomDownPS.BytecodeLength = sizeof(g_BloomDownPS);
	BloomDownPS.pShaderBytecode = g_BloomDownPS;
	GlobalShaderMap.insert(std::make_pair(L"BloomDownPS", BloomDownPS));

	D3D12_SHADER_BYTECODE BloomUpPS;
	BloomUpPS.BytecodeLength = sizeof(g_BloomUpPS);
	BloomUpPS.pShaderBytecode = g_BloomUpPS;
	GlobalShaderMap.insert(std::make_pair(L"BloomUpPS", BloomUpPS));

	D3D12_SHADER_BYTECODE BloomSunMergePS;
	BloomSunMergePS.BytecodeLength = sizeof(g_BloomSunMergePS);
	BloomSunMergePS.pShaderBytecode = g_BloomSunMergePS;
	GlobalShaderMap.insert(std::make_pair(L"BloomSunMergePS", BloomSunMergePS));

	D3D12_SHADER_BYTECODE CombineLUTsPS;
	CombineLUTsPS.BytecodeLength = sizeof(g_CombineLUTsPS);
	CombineLUTsPS.pShaderBytecode = g_CombineLUTsPS;
	GlobalShaderMap.insert(std::make_pair(L"CombineLUTsPS", CombineLUTsPS));

	D3D12_SHADER_BYTECODE ToneMapPS;
	ToneMapPS.BytecodeLength = sizeof(g_ToneMapPS);
	ToneMapPS.pShaderBytecode = g_ToneMapPS;
	GlobalShaderMap.insert(std::make_pair(L"ToneMapPS", ToneMapPS));


	//for gpu driven
	D3D12_SHADER_BYTECODE FrustmCullCS;
	FrustmCullCS.BytecodeLength = sizeof(g_FrustmCullCS);
	FrustmCullCS.pShaderBytecode = g_FrustmCullCS;
	GlobalCSShaderMap.insert(std::make_pair(L"FrustmCullCS", FrustmCullCS));
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
