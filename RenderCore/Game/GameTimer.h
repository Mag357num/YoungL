#pragma once

class FGameTimer
{
public:
	FGameTimer();
	~FGameTimer();

	float GetTotalTime()const;
	float GetDeltaTime()const;

	void Reset();
	void Start();
	void Stop();

	void Tick();

	int GetFrameRate(){return FrameRate;}
	float GetFrameTime(){return 1000.0f / FrameRate;}

	double GetFPS();
private:

	double SecondsPerCount;
	double DelteTime;

	__int64 BaseTime;
	__int64 PausedTime;
	__int64 StopTime;
	__int64 PrevTime;
	__int64 CurrTime;

	bool bStoped;
	int FrameRate;

};

