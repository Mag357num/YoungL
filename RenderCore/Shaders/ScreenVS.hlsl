float4 main(uint VertID : SV_VertexID) : SV_POSITION
{
    // Texture coordinates range [0, 2], but only [0, 1] appears on screen.
    float2 Tex = float2(uint2(VertID, VertID << 1) & 2);
    float4 Pos = float4(lerp(float2(-1, 1), float2(1, -1), Tex), 0, 1);

	return Pos;
}
