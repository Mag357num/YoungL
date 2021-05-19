

RWTexture2D<float4> OutPut : register(u0);

[numthreads(8, 8, 1)]
void main(uint3 groupThreadID : SV_GroupThreadID,
		uint3 DTid : SV_DispatchThreadID )
{
	float4 TestColor = float4(1.0f, 0.6f, 0.0f, 0.0f);
	OutPut[DTid.xy] = TestColor;
}