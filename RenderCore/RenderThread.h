#pragma once
#include<thread>
#include <functional>
#include <queue>

static int ClientWidth;
static int ClientHeight;
static bool RequstStopThread;
static FRenderer* Renderer;
static std::weak_ptr<FGameCore> WeakGameCore;


namespace RenderFrameSync
{
	static UINT8 FrameSyncFence = 0;
	static std::queue<std::function<void(FRenderer*, FGameCore*)>> TaskQueue;
}


class FRenderThreadTask
{
public:
protected:
private:
};

class FRenderThread
{
public:
	FRenderThread(){}
	~FRenderThread(){}

	using RehderThreadTask = std::function<void(FRenderThread*, FGameCore*)>;

	static void Run()
	{
		//FRenderer* Renderer;
		//init render
		if (!Renderer)
		{
			Renderer = new FRenderer();
			Renderer->CreateRHIContext(ClientWidth, ClientHeight);
		}

		while (!RequstStopThread)
		{
			while (RenderFrameSync::FrameSyncFence < 1)
			{
				Sleep(10);
			} 

			while (RenderFrameSync::TaskQueue.size() > 0)
			{
				std::function<void(FRenderer*, FGameCore*)> Func = RenderFrameSync::TaskQueue.front();
				if (!WeakGameCore.expired())
				{
					std::shared_ptr<FGameCore> TempGameCore = WeakGameCore.lock();
					Func(Renderer, TempGameCore.get());
				}
				
				RenderFrameSync::TaskQueue.pop();
			}

			Renderer->RenderObjects();
			Renderer->UpdateConstantBuffer();

			RenderFrameSync::FrameSyncFence--;

		}

		////
		if (Renderer)
		{
			Renderer->DestroyRHIContext();
			delete Renderer;
			Renderer = nullptr;
		}

	}

	void StartThread(int InWidth, int InHeight, std::weak_ptr<FGameCore> InGameCore){
		RequstStopThread = false;
		ClientWidth = InWidth;
		ClientHeight = InHeight;

		WeakGameCore = InGameCore;

		Thread = std::make_unique<std::thread>(Run);
		Thread->detach();
	}

	void StopThread()
	{
		RequstStopThread = true;
		Thread.release();

	}

	//temparory for main thread excute
	void CreateRenderingItem(std::vector<std::unique_ptr<AMeshActor>>& Geometries)
	{
		if (Renderer)
		{
			Renderer->CreateRenderingItem(Geometries);
		}
	}

	void PushTask(const std::function<void(FRenderer*, FGameCore*)>& InTask)
	{
		RenderFrameSync::TaskQueue.push(InTask);
	}

private:
	
	//for graphics
	std::unique_ptr<std::thread> Thread;
};