#include "Renderer.h"
#include "RHI/D3D12/RHIContext_D3D12.h"

#define MAX_LOADSTRING 100



void FRenderer::CreateRHIContext(int InWidth, int Inheight)
{
#ifdef _WIN32
	RHIContext = new FRHIContext_D3D12();
#elif __APPLE__

#elif __ANDROID__

#else

#endif

	RHIContext->InitializeRHI(InWidth, Inheight);

}

void FRenderer::DestroyRHIContext()
{
#ifdef _WIN32
	delete RHIContext;
	RHIContext = nullptr;
#elif __APPLE__

#elif __ANDROID__

#else

#endif
}

void FRenderer::Resize(int InWidth, int InHeight)
{
	if (RHIContext)
	{
		RHIContext->Resize(InWidth, InHeight);
	}
}

void FRenderer::RenderObjects()
{

}