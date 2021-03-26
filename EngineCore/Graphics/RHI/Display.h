#pragma once

#include <cstdint>

namespace Display
{
	void Initialize(void);
	void ShutDown(void);
	void Resize(uint32_t Width, uint32_t Height);
	void Present(void);
}

namespace Graphics
{
	extern uint32_t g_DisplayWidth;
	extern uint32_t g_DisplayHeight;
	extern bool g_EnableHDROutput;

	uint64_t GetFrameCount(void);

	float GetFrameTime(void);

	float GetFrameRate(void);

}
