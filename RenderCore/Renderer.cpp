#include "Renderer.h"
#include "RHI/D3D12/RHIContext_D3D12.h"

#define MAX_LOADSTRING 100

using namespace Utilities;

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

	//initialize scene constant
	SceneConstant.Proj = Utilities::MatrixPerspectiveFovLH(0.25f * 3.1416f, (Viewport.Width / Viewport.Height), 1.0f, 1000.0f);

	//// Build the view matrix.
	FVector4D CamPos = FVector4D(500, 500, 100, 1.0f);
	FVector4D CamTarget = FVector4D(0, 0, 150, 0.0f);
	FVector4D CamUp = FVector4D(0.0f, 0.0f, 1.0f, 0.0f);

	SceneConstant.View = Utilities::MatrixLookAtLH(CamPos, CamTarget, CamUp);

	//
	//Create Scene Constant Buffer
	SceneConstantBuffer = RHIContext->CreateSceneConstantBuffer(SceneConstant);
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

	if (SceneConstantBuffer)
	{
	delete SceneConstantBuffer;
	SceneConstantBuffer = nullptr;
	}
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

	//buil scene constant buffer
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

	//pass sceen constant buffer
	RHIContext->SetSceneConstantBuffer(SceneConstantBuffer);
	//M_CommandList->SetGraphicsRootDescriptorTable(ConstantBuffer->GetRootParameterIndex(), ConstantBuffer->GetGpuHandle());

	//Draw Rendering items in scene
	RHIContext->DrawRenderingItems(RenderingItems);

	//change back buffer state to present
	RHIContext->TransitionBackBufferStateToPresent();

	//excute command list
	RHIContext->EndDraw();

	//present backbuffer
	RHIContext->Present();

	//flush commands
	RHIContext->FlushCommandQueue();
}

void FRenderer::UpdateConstantBuffer()
{
	for (int Index = 0 ; Index < RenderingItems.size(); ++Index)
	{
		FMatrix World = Utilities::IdentityMatrix;

		FMatrix Proj = Utilities::MatrixPerspectiveFovLH(0.25f * 3.1416f, (Viewport.Width / Viewport.Height), 1.0f, 1000.0f);

		//// Build the view matrix.
		FVector4D CamPos = FVector4D(500, 500, 100, 1.0f);
		FVector4D CamTarget = FVector4D(0, 0, 150, 0.0f);
		FVector4D CamUp = FVector4D(0.0f, 0.0f, 1.0f, 0.0f);

		FMatrix View = Utilities::MatrixLookAtLH(CamPos, CamTarget, CamUp);

		FMatrix WorldViewProj = World * View * Proj;

		// Update the constant buffer with the latest worldViewProj matrix.
		FObjectConstants ObjectConstant;

		//copy to upload buffer transposed???
		ObjectConstant.WorldViewProj = Utilities::MatrixTranspose(WorldViewProj);
		ObjectConstant.CameraLocation = FVector(CamPos.X, CamPos.Y, CamPos.Z);
		RenderingItems[Index]->GetConstantBuffer()->CopyData(0, ObjectConstant);
	}


}