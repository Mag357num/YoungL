#include "ShaderUtility.hlsli"
#include "PresentRS.hlsli"

Texture2D<float4> ColorTex : register(t0);

[RootSignature(Present_RootSig)]
float4 main() : SV_TARGET
{
	return float4(1.0f, 0.0f, 1.0f, 1.0f);
}