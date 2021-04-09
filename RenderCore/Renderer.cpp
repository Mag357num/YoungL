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

	//viewport
	Viewport.X = 0;
	Viewport.Y = 0;
	Viewport.Width = (float)InWidth;
	Viewport.Height = (float)Inheight;
	Viewport.MaxDepth = 1.0f;
	Viewport.MinDepth = 0.0f;

	RHIContext->InitializeRHI(InWidth, Inheight);
	IRHIGraphicsPipelineState* BasePassPSO = RHIContext->CreateGraphicsPSO();
	GraphicsPSOs.insert(std::make_pair("BasePass", BasePassPSO));
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

	GraphicsPSOs.empty();
	RenderingItems.empty();
}

void FRenderer::Resize(int InWidth, int InHeight)
{
	if (RHIContext)
	{
		RHIContext->Resize(InWidth, InHeight);
	}
}

void FRenderer::CreateRenderingItem(std::vector<std::unique_ptr<AMeshActor>>& Geometries)
{
	for (int Index = 0; Index < Geometries.size(); ++Index)
	{
		//create an empty rendering item

		IRHIRenderingItem* Item = RHIContext->CreateEmptyRenderingItem();

		Item->BuildConstantBuffer(Geometries[Index]->GetObjectConstants(), RHIContext);
		Item->BuildVertexBuffer(Geometries[Index]->GetGeometry()->GetVertices());
		Item->BuildIndexBuffer(Geometries[Index]->GetGeometry()->GetIndices());

		RenderingItems.push_back(Item);
	}
}

void FRenderer::RenderObjects()
{
	//reset command list and command allocator here
	RHIContext->BeginDraw(GraphicsPSOs["BasePass"]);

	RHIContext->SetViewport(Viewport);
	RHIContext->SetScissor(0, 0, (long)Viewport.Width, (long)Viewport.Height);

	//change back buffer state to rendertarget
	RHIContext->TransitionBackBufferStateToRT();

	//set backbuffer as rendertarget
	RHIContext->SetBackBufferAsRt();

	//prepare shader parameters
	RHIContext->PrepareShaderParameter();

	//Draw Rendering items in scene
	//RHIContext->DrawRenderingItems(RenderingItems);

	//change back buffer state to present
	RHIContext->TransitionBackBufferStateToPresent();

	//excute command list
	RHIContext->EndDraw();

	//present backbuffer
	RHIContext->Present();

	//flush commands
	RHIContext->FlushCommandQueue();
}