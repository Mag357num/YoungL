#pragma once

#include "CompiledShaders/BasePassVS.h"//g_BasePassVS
#include "CompiledShaders/BasePassPS.h"//g_BasePassPS

#include "CompiledShaders/DepthVS.h"//depth pass vs
#include "CompiledShaders/DepthPS.h"// depth pass ps

#include "CompiledShaders/SkinnedMeshVS.h"// skinnedMeshVS
#include "CompiledShaders/SkinnedMeshPS.h"// SkinnedMeshPs

#include "CompiledShaders/ScreenVS.h"
#include "CompiledShaders/ScreenPS.h"
#include "CompiledShaders/BloomSetUpPS.h"
#include "CompiledShaders/BloomDownPS.h"
#include "CompiledShaders/BloomUpPS.h"
#include "CompiledShaders/CombineLUTsPS.h"
#include "CompiledShaders/ToneMapPS.h"
#include "CompiledShaders/BloomSunMergePS.h"

#include <d3d12.h>
#include <map>



class FRHIShaderResource_D3D12
{
public:
	FRHIShaderResource_D3D12();


	virtual ~FRHIShaderResource_D3D12();


private:
};

