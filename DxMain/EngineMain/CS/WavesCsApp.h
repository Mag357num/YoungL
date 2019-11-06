#pragma once

#include "../Common/D3DApp.h"
#include "../Common/DXSampleHelper.h"
#include "../Common/d3dUtil.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

class WavesCsApp : public D3DApp
{
public:
	WavesCsApp(HINSTANCE hInstance);
	WavesCsApp(const WavesCsApp& rhs) = delete;
	WavesCsApp& operator=(const WavesCsApp&) = delete;
	~WavesCsApp();

	virtual bool Initialize()override;
	virtual void Update(const GameTimer& gt)override;
	virtual void Draw(const GameTimer& gt)override;

private:
	void LoadTextures();
	
	void BuildRootSignature();
	std::array<CD3DX12_STATIC_SAMPLER_DESC, 6> GetStaticSamples();

	std::unordered_map<std::string, std::unique_ptr<Texture>> mTextures;

};