#pragma once
#include <thread>
#include <functional>
#include <queue>
#include <windows.h>
#include <mutex>

#include "Renderer.h"

static int ClientWidth;
static int ClientHeight;
static bool RequstStopThread;
static FRenderer* Renderer;

static std::mutex FrameSyncMutex;
static UINT8 FrameSyncFence;

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

			IncreFrameSyncFence(false);
		}

	}

	void StartThread(int InWidth, int InHeight){
		RequstStopThread = false;
		ClientWidth = InWidth;
		ClientHeight = InHeight;
		FrameSyncFence = 0;

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

	static void IncreFrameSyncFence(bool Flag)
	{
		//todo add thread mutex
		std::lock_guard<std::mutex> LockGuard(FrameSyncMutex);
		if (Flag)
		{
			FrameSyncFence++;
		}
		else
		{
			FrameSyncFence--;
		}
		
	}

	UINT8 GetFrameSyncFence(){return FrameSyncFence;}

private:

	//for graphics
	std::unique_ptr<std::thread> Thread;
};