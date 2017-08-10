#pragma once

#include "stdsfx.h"

// Note that while ComPtr is used to manage the lifetime of resources on the CPU,
// it has no understanding of the lifetime of resources on the GPU. Apps must account
// for the GPU lifetime of resources to avoid destroying objects that may still be
// referenced by the GPU.
// An example of this can be found in the class method: OnDestroy().
using Microsoft::WRL::ComPtr;

class Sample
{
public:
	Sample();
	~Sample();

	void LoadPipeline();
	void LoadAsset();
private:

};


