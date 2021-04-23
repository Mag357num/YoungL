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

	static void CreateRenderingItems(std::vector<std::unique_ptr<AMeshActor>>& Geometries);
	static void CreateRenderingItems(std::vector<std::unique_ptr<ASkinMeshActor>>& SkinedActors);
	static void UpdateSceneConstantBuffer(FSceneConstant* SceneConstant);

	static void UpdateSkinnedMeshBoneTransform(std::string* ActorName, FBoneTransforms* InBoneTrans);

	void PushRenderCommand(FRenderThreadCommand InCommand);
private:
	FRenderThread* RenderThread;
};

