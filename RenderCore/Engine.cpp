#include "pch.h"
#include "Engine.h"

FEngine* FEngine::Instance = nullptr;

void FEngine::InitEngine()
{
	Instance = new FEngine();
}

void FEngine::DestroyEngine()
{
	if (Instance)
	{
		Instance->DestroyAllEngineContent();

		delete Instance;
		Instance = nullptr;
	}
}

void FEngine::InitGame(int InWidth, int InHeight)
{
	//init game core
	if (!GameCore)
	{
		GameCore = new FGameCore(InWidth, InHeight);
		GameCore->Initialize();
	}

	//init game timer
	if (!GameTimer)
	{
		GameTimer = new FGameTimer();
		//prepare timer start
		GameTimer->Reset();
		GameTimer->Start();
	}
}

static void CreateRenderingItem_RenderThread(FGameCore* InGame)
{
	FRenderThreadManager::CreateRenderingItems(InGame->GetStaticActors());
	FRenderThreadManager::CreateRenderingItems(InGame->GetSkinedActors());
}


void FEngine::InitRenderer(int InWidth, int InHeight)
{
	//try to start render thread
	if (!RenderThreadManager)
	{
		RenderThreadManager = std::make_shared<FRenderThreadManager>();
		RenderThreadManager->StartRenderThread(InWidth, InHeight);
		FRenderThreadCommand CreateRenderItemCommand;
		CreateRenderItemCommand.Wrap(CreateRenderingItem_RenderThread, GameCore);
		RenderThreadManager->PushRenderCommand(CreateRenderItemCommand);

		//pass render thread manager to game core
		GameCore->RenderThreadManager_Weak = RenderThreadManager;
	}
}

void FEngine::DestroyAllEngineContent()
{
	//destroy game core
	if (GameCore)
	{
		GameCore->ShutDown();
		delete GameCore;
		GameCore = nullptr;
	}

	//destroy game timer
	if (GameTimer)
	{
		delete GameTimer;
		GameTimer = nullptr;
	}

	//destroy render thread manager
	if (RenderThreadManager)
	{
		RenderThreadManager->StopRenderThread();
		RenderThreadManager.reset();
	}
}

void FEngine::Run()
{
	//wait for render thread processed
	RenderThreadManager->WaitForRenderThreadSingal();

	GameTimer->Tick();
	float DeltaTime = GameTimer->GetDeltaTime();
	if (DeltaTime < GameTimer->GetFrameTime())
	{
		int SleepTime = (int)round(GameTimer->GetFrameTime() - DeltaTime);
		Sleep(SleepTime);
	}
	GameCore->Tick(DeltaTime);

	//todo : notify game thread is completed
	RenderThreadManager->NotifyRenderThreadJob();
}