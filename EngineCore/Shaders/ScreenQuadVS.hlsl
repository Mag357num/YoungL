#include "PresentRS.hlsli"

[RootSignature(Present_RootSig)]
void main(
	in uint VertId : SV_VertexID,
	out float4 Pos : SV_Position,
	out float2 Tex : TexCoord0)
{
	Tex = float2(uint2(VertId, VertId << 1) & 2);
	Pos = float4(lerp(float2(-1, 1), float2(1, -1), Tex), 0, 1);
}