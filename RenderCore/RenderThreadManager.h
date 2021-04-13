#pragma once
#include "RenderThread.h"

class FRenderThreadManager
{
public:
	FRenderThreadManager();
	~FRenderThreadManager();

	void StartRenderThread(int InWidth, int InHeight);
	void StopRenderThread();

	void IncreFrameSyncFence();
	UINT8 GetFrameSyncFence();

	static void CreateRenderingItems(std::vector<std::unique_ptr<AMeshActor>>& Geometries);
	static void UpdateSceneConstantBuffer(FMatrix InView, FMatrix InProj, FVector4D InCamerLoc);

	void PushRenderCommand(FRenderThreadCommand InCommand);
private:
	FRenderThread* RenderThread;
};

