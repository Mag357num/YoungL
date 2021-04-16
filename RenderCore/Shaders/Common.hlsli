
struct VertexIn
{
	float3 Pos : POSITION;
	float3 Normal : NORMAL;
	float2 Uv : TEXCOORD;
};

struct VertexOut
{
	float4 PosH : SV_POSITION;
	float4 ShadowPosH : POSITION0;
	float4 Color : COLOR;
	float3 Normal : NORMAL;
	float2 Uv : TEXCOORD;
	float3 PosW : POSITION1;
};

