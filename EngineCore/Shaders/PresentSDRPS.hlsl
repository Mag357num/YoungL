#include "ShaderUtility.hlsli"
#include "PresentRS.hlsli"

Texture2D<float3> ColorTex : register(t0);

[RootSignature(Present_RootSig)]
float4 main() : SV_TARGET
{
	return float4(0.0f, 0.0f, 1.0f, 1.0f);
}