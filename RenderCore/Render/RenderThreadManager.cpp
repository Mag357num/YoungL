#include "pch.h"
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

void FRenderThreadManager::WaitForRenderThreadSingal()
{
	RenderThread->WaitForRenderThreadSingal();
}

void FRenderThreadManager::NotifyRenderThreadJob()
{
	RenderThread->NotifyRenderThreadJob();
}

void FRenderThreadManager::CreateRenderingItems(std::vector<std::shared_ptr<AStaticMeshActor>>& Actors)
{
	Renderer->CreateRenderingItem(Actors);
}

void FRenderThreadManager::CreateRenderingItems(std::vector<std::shared_ptr<ASkeletalMeshActor>>& Actors)
{
	Renderer->CreateRenderingItem(Actors);
}

void FRenderThreadManager::CreateRenderingItems(std::vector<std::shared_ptr<AInstancedStaticMeshActor>>& Actors)
{
	Renderer->CreateRenderingItem(Actors);
}

void FRenderThreadManager::UpdateSceneConstantBuffer(FSceneConstant* SceneConstant)
{
	Renderer->UpdateSceneConstantsBuffer(SceneConstant);
}

void FRenderThreadManager::UpdateActorConstantBuffer(std::string ActorName, FObjectConstants* ObjConstants)
{
	Renderer->UpdateActorConstantBuffer(ActorName, ObjConstants);
}

void FRenderThreadManager::UpdateSkinnedMeshBoneTransform(std::string* ActorName, FBoneTransforms* InBoneTrans)
{
	Renderer->UpdateSkinnedMeshBoneTransform(*ActorName, InBoneTrans);
}
