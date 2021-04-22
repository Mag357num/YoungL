#pragma once
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
	virtual void Run();

	FGameCore* GetGameCore(){return GameCore;}
	
private:
	static FEngine* Instance;

private:

	
	void DestroyAllEngineContent();


	//game and render
	FGameCore* GameCore;
	FGameTimer* GameTimer;

	std::shared_ptr<FRenderThreadManager> RenderThreadManager;
};
