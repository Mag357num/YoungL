#pragma once

class UGameTimer
{
public:
	UGameTimer();
	~UGameTimer();

	float GetTotalTime()const;
	float GetDeltaTime()const;

	void Reset();
	void Start();
	void Stop();

	void Tick();

	int GetFrameRate(){return FrameRate;}
	float GetFrameTime(){return 1.0f / FrameRate;}

	__int64 GetCurrentTimeCount();
	double GetDeltaTimeBetweenTwoCount(__int64 Start, __int64 End);

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

