#pragma once

class SystemTime
{
public:

	static void Initialize(void);
	static int64_t GetCurrentTick();
	static void BusyLoopSleep(float SleepTime);

	static inline double TicksToSeconds(int64_t TickCount)
	{
		return TickCount * sm_CpuTickDelta;
	}

	static inline double TicksToMillisecs(int64_t TickCount)
	{
		return TickCount * sm_CpuTickDelta * 1000.0;
	}

	static inline double TimeBetweenTicks(int64_t Tick1, int64_t Tick2)
	{
		return TicksToSeconds(Tick2 - Tick1);
	}

private:

	static double sm_CpuTickDelta;
};

class GameTimer
{
public:

	GameTimer()
	{
		m_StartTick = 0ll;
		m_ElapsedTicks = 0ll;
	}

	void Start()
	{
		if (m_StartTick == 0ll)
		{
			m_StartTick = SystemTime::GetCurrentTick();
		}
	}

	void Stop()
	{
		if (m_StartTick != 0ll)
		{
			m_ElapsedTicks += SystemTime::GetCurrentTick() - m_StartTick;
			m_StartTick = 0ll;
		}
	}

	void Reset()
	{
		m_StartTick = 0ll;
		m_ElapsedTicks = 0ll;
	}

	double GetTime()const
	{
		return SystemTime::TicksToSeconds(m_ElapsedTicks);
	}
private:

	int64_t m_StartTick;
	int64_t m_ElapsedTicks;
};