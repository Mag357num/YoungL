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
static bool GameThreadReady = false;//initialized for game thread do first
static bool RenderThreadProcessed = true;//initialized for game thread do first

static std::condition_variable ThreadSingnal;

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
			//WaitforGameThreadSingal();
			std::unique_lock<std::mutex> RenderLock(FrameSyncMutex);
			//wait for game thread is ready
			WaitforGameThreadSingal(RenderLock);

			//Utilities::Print("Render Thread Tick.....\n");

			while (RenderThreadTask::CommandQueue.size() > 0)
			{
				FRenderThreadCommand* Command = &RenderThreadTask::CommandQueue.front();
				Command->Excute();
				RenderThreadTask::CommandQueue.pop();
			}

			if (!RenderThreadTask::CommandQueue.empty())
			{
				Utilities::Print("Empty Error");
			}

			Renderer->RenderScene();
			//Renderer->UpdateConstantBuffer();

			//notify reander thread is completed
			NotifyGameThreadjob(RenderLock);
		}

	}

	void StartThread(int InWidth, int InHeight){
		RequstStopThread = false;
		ClientWidth = InWidth;
		ClientHeight = InHeight;

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

	static void WaitforGameThreadSingal(std::unique_lock<std::mutex>& InLock)
	{
		ThreadSingnal.wait(InLock, []{return GameThreadReady;});
	}

	static void NotifyGameThreadjob(std::unique_lock<std::mutex>& InLock)
	{
		RenderThreadProcessed = true;
		GameThreadReady = false;
		InLock.unlock();
		ThreadSingnal.notify_one();
	}

	static void WaitForRenderThreadSingal()
	{
		std::unique_lock<std::mutex> RenderLock(FrameSyncMutex);
		ThreadSingnal.wait(RenderLock, [] {return RenderThreadProcessed; });
	}

	static void NotifyRenderThreadJob()
	{
		GameThreadReady = true;
		RenderThreadProcessed = false;
		ThreadSingnal.notify_one();
	}


private:

	//for graphics
	std::unique_ptr<std::thread> Thread;
};