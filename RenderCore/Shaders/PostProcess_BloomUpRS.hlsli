#define PostProcess_BloomUpRootSig \
	"DescriptorTable(SRV(t0, numDescriptors = 1))," \
	"DescriptorTable(SRV(t1, numDescriptors = 1))," \
	"RootConstants(b0, num32BitConstants = 4), " \
	"StaticSampler(s0," \
	"addressU = TEXTURE_ADDRESS_CLAMP," \
	"addressV = TEXTURE_ADDRESS_CLAMP," \
	"addressW = TEXTURE_ADDRESS_CLAMP," \
	"filter = FILTER_MIN_MAG_LINEAR_MIP_POINT),"