#include "ScreenRS.hlsli"

Texture2D gSceneColor : register(t0);

[RootSignature(PostProcess_RootSig)]
float4 main(float4 position : SV_Position) : SV_TARGET
{
	int X = floor(position.x);
	int Y = floor(position.y);

	float4 Color = gSceneColor.Load(int3(X, Y, 0));
	return Color;
}