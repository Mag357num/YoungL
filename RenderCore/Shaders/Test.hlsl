cbuffer cbPerObject : register(b0)
{
	float4x4 gWorldViewProj;
};

struct VertexIn
{
	float3 Pos : POSITION;
	float4 Color : COLOR;
};

struct VertexOut
{
	float4 PosH : SV_POSITION;
	float4 Color : COLOR;
};

VertexOut VS(VertexIn Vin)
{
	VertexOut vout;
	
	// Transform to homogeneous clip space.
	vout.PosH = mul(float4(Vin.Pos, 1.0f), gWorldViewProj);
	
	// Just pass vertex color into the pixel shader.
    vout.Color = Vin.Color;
    
    return vout;
}

float4 PS(VertexOut Pin) : SV_Target
{
	return Pin.Color;
}