#include "pch.h"
#include "GameTimer.h"
#include "../Utilities.h"

#include <windows.h>

FGameTimer::FGameTimer()
{
	__int64 CountPerSec;
	QueryPerformanceFrequency((LARGE_INTEGER*)&CountPerSec);
	SecondsPerCount = 1.0f / (double) CountPerSec;

	FrameRate = 60;
}

FGameTimer::~FGameTimer()
{

}

float FGameTimer::GetTotalTime()const
{
	// If we are stopped, do not count the time that has passed since we stopped.
// Moreover, if we previously already had a pause, the distance 
// mStopTime - mBaseTime includes paused time, which we do not want to count.
// To correct this, we can subtract the paused time from mStopTime:  
//
//                     |<--paused time-->|
// ----*---------------*-----------------*------------*------------*------> time
//  mBaseTime       mStopTime        startTime     mStopTime    mCurrTime

	if (bStoped)
	{
		return (float)(((StopTime - PausedTime) - BaseTime) * SecondsPerCount);
	}

	// The distance mCurrTime - mBaseTime includes paused time,
	// which we do not want to count.  To correct this, we can subtract 
	// the paused time from mCurrTime:  
	//
	//  (mCurrTime - mPausedTime) - mBaseTime 
	//
	//                     |<--paused time-->|
	// ----*---------------*-----------------*------------*------> time
	//  mBaseTime       mStopTime        startTime     mCurrTime

	else
	{
		return (float)(((CurrTime - PausedTime) - BaseTime) * SecondsPerCount);
	}
}

float FGameTimer::GetDeltaTime()const
{
	return (float)DelteTime;
}

void FGameTimer::Reset()
{
	__int64 TemCurrTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&TemCurrTime);

	BaseTime = TemCurrTime;
	PrevTime = TemCurrTime;
	StopTime = 0;
	bStoped = true;
}

void FGameTimer::Start()
{
	if (bStoped)
	{
		__int64 TemCurrTime;
		QueryPerformanceCounter((LARGE_INTEGER*)&TemCurrTime);

		PausedTime += (TemCurrTime - StopTime);
		PrevTime = TemCurrTime;
		StopTime = 0;
		bStoped = false;
	}
}

void FGameTimer::Stop()
{
	if (!bStoped)
	{
		__int64 TemCurrTime;
		QueryPerformanceCounter((LARGE_INTEGER*)&TemCurrTime);

		StopTime = TemCurrTime;
		bStoped = true;
	}
}

void FGameTimer::Tick()
{
	if (bStoped)
	{
		DelteTime = 0.0;
		return;
	}

	__int64 TemCurrTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&TemCurrTime);

	CurrTime = TemCurrTime;
	DelteTime = (CurrTime - PrevTime)* SecondsPerCount;
	PrevTime = TemCurrTime;

	if (DelteTime < 0.0)
	{
		DelteTime = 0.0;
	}
}

double FGameTimer::GetFPS()
{
	double ActoruallyFrame = DelteTime;
	double FreeStateFrameTime = 1.0f / FrameRate;
	if (ActoruallyFrame < FreeStateFrameTime)
	{
		ActoruallyFrame = FreeStateFrameTime;		
	}
	else
	{
		Utilities::Print("ActoruallyFrame > Frame Rate Time\n");
	}

	return 1.0f / ActoruallyFrame;
}

__int64 FGameTimer::GetCurrentTimeCount()
{
	__int64 TemCurrTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&TemCurrTime);

	return TemCurrTime;
}

double FGameTimer::GetDeltaTimeBetweenTwoCount(__int64 Start, __int64 End)
{
	double RetDelta;
	RetDelta = (CurrTime - PrevTime) * SecondsPerCount;

	return RetDelta;
}