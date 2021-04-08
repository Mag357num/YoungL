#pragma once

#include "RHI/RHIContext.h"
class FRenderer
{
public:
	void CreateRHIContext(int InWidth, int Inheight);
	void DestroyRHIContext();

	void RenderObjects();

	void Resize(int InWidth, int InHeight);

protected:
private:

	IRHIContext* RHIContext;
};
