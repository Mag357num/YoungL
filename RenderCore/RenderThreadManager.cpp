#include "RenderThreadManager.h"

FRenderThreadManager::FRenderThreadManager()
{

}

FRenderThreadManager::~FRenderThreadManager()
{

}

void FRenderThreadManager::StartRenderThread(int InWidth, int InHeight)
{
	RenderThread = new FRenderThread();
	RenderThread->StartThread(InWidth, InHeight);
}

void FRenderThreadManager::StopRenderThread()
{
	RenderThread->StopThread();
	delete RenderThread;
	RenderThread = nullptr;
}

void FRenderThreadManager::PushRenderCommand(FRenderThreadCommand InCommand)
{
	RenderThread->PushTask(InCommand);
}

void FRenderThreadManager::IncreFrameSyncFence(bool Flag)
{
	FRenderThread::IncreFrameSyncFence(Flag);
}

UINT8 FRenderThreadManager::GetFrameSyncFence()
{
	return RenderThread->GetFrameSyncFence();
}

void FRenderThreadManager::CreateRenderingItems(std::vector<std::unique_ptr<AMeshActor>>& Geometries)
{
	Renderer->CreateRenderingItem(Geometries);
}

void FRenderThreadManager::UpdateSceneConstantBuffer(FSceneConstant* SceneConstant)
{
	Renderer->UpdateSceneConstantsBuffer(SceneConstant);
}