#include "DepthRS.hlsli"
#include "Common.hlsli"


[RootSignature(Depth_RootSig)]
float4 main(VertexOut PIn) : SV_TARGET
{
	return float4(1.0f, 1.0f, 1.0f, 1.0f);
}