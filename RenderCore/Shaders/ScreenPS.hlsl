#include "ScreenRS.hlsli"

Texture2D gShadowMap : register(t0);

[RootSignature(PostProcess_RootSig)]
float4 main() : SV_TARGET
{
	return float4(1.0f, 1.0f, 1.0f, 1.0f);
}