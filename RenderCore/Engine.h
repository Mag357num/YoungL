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

	UGameCore* GetGameCore(){return GameCore;}
	FRenderThreadManager* GetRenderThreadManager(){return RenderThreadManager.get();}
	
	double GetFPS(){
		
		if (GameTimer)
		{
			return GameTimer->GetFPS();
		}
		return 0.0;
	}

private:
	static FEngine* Instance;

private:

	void DestroyAllEngineContent();


	//game and render
	UGameCore* GameCore;
	UGameTimer* GameTimer;

	std::shared_ptr<FRenderThreadManager> RenderThreadManager;
};
