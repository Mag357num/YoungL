#pragma once
#include "Game/GameCore.h"
#include "Game/GameTimer.h"

class FEngine
{
public:
	FEngine(){}
	~FEngine(){}

	
	static FEngine* GetEngine(){return Instance;}
	static void InitEngine();
	static void DestroyEngine();

	void InitGame(int InWidth, int InHeight);
	void InitRenderer(int InWidth, int InHeight);
	virtual void Tick();

	FGameCore* GetGameCore(){return GameCore;}
	FRenderThreadManager* GetRenderThreadManager(){return RenderThreadManager.get();}
	
private:
	static FEngine* Instance;

private:

	void DestroyAllEngineContent();


	//game and render
	FGameCore* GameCore;
	FGameTimer* GameTimer;

	std::shared_ptr<FRenderThreadManager> RenderThreadManager;
};
