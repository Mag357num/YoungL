#pragma once
#include "../RHIShader.h"
#include <d3dcommon.h>
#include <wrl.h>

using namespace Microsoft::WRL;

class FRHIShader_D3D12 : public IRHIShader
{
public:
	FRHIShader_D3D12(){}
	~FRHIShader_D3D12(){
	
		if (CompiledShader)
		{
			CompiledShader.Reset();
		}
	}


	virtual void CompileShader()override;
private:

	ComPtr<ID3DBlob> CompileShader(
		const std::wstring& filename,
		const D3D_SHADER_MACRO* defines,
		const std::string& entrypoint,
		const std::string& target);

	ComPtr<ID3DBlob> CompiledShader;
};
