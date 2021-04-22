#pragma once
#include "CompiledShaders/BasePassVS.h"//g_BasePassVS
#include "CompiledShaders/BasePassPS.h"//g_BasePassPS

#include "CompiledShaders/DepthVS.h"//depth pass vs
#include "CompiledShaders/DepthPS.h"// depth pass ps

#include "CompiledShaders/SkinedMeshVS.h"// skinnedMeshVS
#include "CompiledShaders/SkinedMeshPS.h"// SkinnedMeshPs

class FRHIShaderResource_D3D12
{
public:
	FRHIShaderResource_D3D12(){}
	virtual ~FRHIShaderResource_D3D12(){}

private:

};

