#pragma once
#include "RenderThread.h"

class FRenderThreadManager
{
public:
	FRenderThreadManager();
	~FRenderThreadManager();

	void StartRenderThread(int InWidth, int InHeight);
	void StopRenderThread();

	void WaitForRenderThreadSingal();
	void NotifyRenderThreadJob();

	static void CreateRenderingItems(std::vector<std::shared_ptr<AStaticMeshActor>>& Actors);
	static void CreateRenderingItems(std::vector<std::shared_ptr<ASkeletalMeshActor>>& Actors);
	static void CreateRenderingItems(std::vector<std::shared_ptr<AInstancedStaticMeshActor>>& Actors);
	static void UpdateSceneConstantBuffer(FSceneConstant* SceneConstant);
	static void UpdateActorConstantBuffer(std::string ActorName, FObjectConstants* ObjConstants);

	static void UpdateSkinnedMeshBoneTransform(std::string* ActorName, FBoneTransforms* InBoneTrans);

	void PushRenderCommand(FRenderThreadCommand InCommand);
private:
	FRenderThread* RenderThread;
};

