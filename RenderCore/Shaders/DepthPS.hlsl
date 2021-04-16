#include "DepthRS.hlsli"
#include "Common.hlsli"


[RootSignature(Depth_RootSig)]
float4 main(VertexOut PIn) : SV_TARGET
{
	return PIn.Color;
}