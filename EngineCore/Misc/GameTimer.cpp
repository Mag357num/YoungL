#include "pch.h"

#include "GameTimer.h"

double SystemTime::sm_CpuTickDelta = 0.0;

void SystemTime::Initialize()
{
	LARGE_INTEGER Frequency;
	ASSERT(QueryPerformanceFrequency(&Frequency), "Unable to query performance counter frequency");
	sm_CpuTickDelta = 1.0 / static_cast<double>(Frequency.QuadPart);
}

int64_t SystemTime::GetCurrentTick()
{
	LARGE_INTEGER CurrentTick;
	ASSERT(QueryPerformanceCounter(&CurrentTick), "unable to query performance counter value");
	return static_cast<int64_t>(CurrentTick.QuadPart);
}

void SystemTime::BusyLoopSleep(float SleepTime)
{
	int64_t FinalTick = (int64_t)((double)SleepTime / sm_CpuTickDelta) + GetCurrentTick();
	while (GetCurrentTick() < FinalTick);
}