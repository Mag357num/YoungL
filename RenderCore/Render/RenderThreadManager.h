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
	static void UpdateSceneConstantBuffer(FSceneConstant* SceneConstant);

	void PushRenderCommand(FRenderThreadCommand InCommand);
private:
	FRenderThread* RenderThread;
};

