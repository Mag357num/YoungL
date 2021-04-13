#pragma once
#include <thread>
#include <functional>
#include <queue>
#include <windows.h>

#include "Renderer.h"

static int ClientWidth;
static int ClientHeight;
static bool RequstStopThread;
static FRenderer* Renderer;
static UINT8 FrameSyncFence;

//namespace RenderFrameSync
//{
//	static UINT8 FrameSyncFence = 0;
//}

struct FRenderThreadCommand
{

public:

	template<class F, class... Args, class=typename std::enable_if<!std::is_member_function_pointer<F>::value>::type>
	void Wrap(F&& f, Args&& ... args)
	{
		Fun = [&]{f(args...);};
	}

	void Excute()
	{
		Fun();
	}

private:
	std::function<void()> Fun;
};

namespace RenderThreadTask
{
	static std::queue<FRenderThreadCommand> CommandQueue;
}


class FRenderThread
{
public:
	FRenderThread(){}
	~FRenderThread(){}

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
			while (FrameSyncFence < 1)
			{
				Sleep(10);
			} 

			while (RenderThreadTask::CommandQueue.size() > 0)
			{
				FRenderThreadCommand Command = RenderThreadTask::CommandQueue.front();
				Command.Excute();
				RenderThreadTask::CommandQueue.pop();
			}

			Renderer->RenderObjects();
			Renderer->UpdateConstantBuffer();

			FrameSyncFence--;

		}

	}

	void StartThread(int InWidth, int InHeight){
		RequstStopThread = false;
		ClientWidth = InWidth;
		ClientHeight = InHeight;
		FrameSyncFence = 0;
		//WeakGameCore = InGameCore;

		Thread = std::make_unique<std::thread>(Run);
		Thread->detach();
	}

	void StopThread()
	{
		RequstStopThread = true;
		Thread.release();

		////
		if (Renderer)
		{
			Renderer->DestroyRHIContext();
			delete Renderer;
			Renderer = nullptr;
		}

	}

	void PushTask(FRenderThreadCommand InCommand)
	{
		RenderThreadTask::CommandQueue.push(InCommand);
	}

	void IncreFrameSyncFence()
	{
		FrameSyncFence++;
	}

	UINT8 GetFrameSyncFence(){return FrameSyncFence;}

private:

	//for graphics
	std::unique_ptr<std::thread> Thread;
};