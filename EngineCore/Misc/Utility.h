#pragma once

#include "../pch.h"

#ifdef _M_X64
#define ENABLE_SSE_CRC32 1
#else
#define ENABLE_SSE_CRC32 0
#endif // _M_X64



namespace Utility
{
	inline void Print(const char* msg) { printf("%s", msg); }
	inline void Print(const wchar_t* msg) { wprintf(L"%ws", msg); }

	inline void Printf(const char* format, ...)
	{
		char buffer[256];
		va_list ap;
		va_start(ap, format);
		vsprintf_s(buffer, 256, format, ap);
		Print(buffer);
	}

	inline void Printf(const wchar_t* format, ...)
	{
		wchar_t buffer[256];
		va_list ap;
		va_start(ap, format);
		vswprintf(buffer, 256, format, ap);
		Print(buffer);
	}

#ifndef RELEASE
	inline void PrintSubMessage(const char* format, ...)
	{
		Print("--> ");
		char buffer[256];
		va_list ap;
		va_start(ap, format);
		vsprintf_s(buffer, 256, format, ap);
		Print(buffer);
		Print("\n");
	}

	inline void PrintSubMessage(const wchar_t* format, ...)
	{
		Print("--> ");
		wchar_t buffer[256];
		va_list ap;
		va_start(ap, format);
		vswprintf(buffer, 256, format, ap);
		Print(buffer);
		Print("\n");
	}

	inline void PrintSubMessage(void)
	{

	}
#endif // RELEASE

	inline size_t HashRange(const uint32_t* const Begin, const uint32_t* const End, size_t Hash)
	{
#if ENABLE_SSE_CRC32
		const  uint64_t* Iter64 = (const uint64_t*)Math::AlignUp(Begin, 8);
		const uint64_t* const End64 = (const uint64_t*)Math::AlignDown(End, 8);

		// if not 64 aligned start with a single u32
		if ((uint32_t*)Iter64 > Begin)
		{
			Hash = _mm_crc32_u32((uint32_t)Hash, *Begin);
		}

		//iteratro over consecutive u64 values;
		while (Iter64 < End64)
		{
			Hash = _mm_crc32_u64((uint64_t)Hash, *Iter64++);
		}

		//if there is a 32-bit remander, accumulate 
		if (Iter64 < End)
		{
			Hash = _mm_crc32_u32((uint32_t)Hash, (uint32_t*)Iter64);
		}

#else
		for (const uint32_t* Iter = Begin; Iter < End; ++Iter)
		{
			Hash = 16777619U * Hash ^ *Iter;
		}
#endif
		return Hash;
	}

	template<typename T> inline size_t HashState(const T* StateDesc, size_t Count = 1, size_t Hash = 2166136261U)
	{
		return HashRange((uint32_t*)StateDesc, (uint32_t*)(StateDesc + Count), Hash);
	}
	
}


#ifdef ERROR
	#undef ERROR
#endif // ERROR

#ifdef ASSERT
	#undef ASSERT
#endif // ASSERT

#ifdef HALT
	#undef HALT
#endif // HALT


#define HALT( ... ) ERROR( __VA_ARGS__ ) __debugbreak();

#ifdef RELEASE
	#define ASSERT( isTrue, ... ) (void)(isTrue)
	#define WARN_ONCE_IF( isTrue, ... ) (void)(isTrue)
	#define WARN_ONCE_IF_NOT( isTrue, ... ) (void)(isTrue)
	#define ERROR( msg, ... )
	#define DEBUGPRINT( msg, ... ) do {} while(0)
	#define ASSERT_SUCCEEDED( hr, ... ) (void)(hr)


#else

	#define STRINGIFY(x) #x
	#define STRINGIFY_BUILTIN(x) STRINGIFY(x)
	#define ASSERT( isFalse, ... ) \
			if (!(bool)(isFalse)) { \
				Utility::Print("\nAssertion failed in " STRINGIFY_BUILTIN(__FILE__) " @ " STRINGIFY_BUILTIN(__LINE__) "\n"); \
				Utility::PrintSubMessage("\'" #isFalse "\' is false"); \
				Utility::PrintSubMessage(__VA_ARGS__); \
				Utility::Print("\n"); \
				__debugbreak(); \
			}

	#define ASSERT_SUCCEEDED( hr, ... ) \
			if (FAILED(hr)) { \
				Utility::Print("\nHRESULT failed in " STRINGIFY_BUILTIN(__FILE__) " @ " STRINGIFY_BUILTIN(__LINE__) "\n"); \
				Utility::PrintSubMessage("hr = 0x%08X", hr); \
				Utility::PrintSubMessage(__VA_ARGS__); \
				Utility::Print("\n"); \
				__debugbreak(); \
			}


	#define WARN_ONCE_IF( isTrue, ... ) \
		{ \
			static bool s_TriggeredWarning = false; \
			if ((bool)(isTrue) && !s_TriggeredWarning) { \
				s_TriggeredWarning = true; \
				Utility::Print("\nWarning issued in " STRINGIFY_BUILTIN(__FILE__) " @ " STRINGIFY_BUILTIN(__LINE__) "\n"); \
				Utility::PrintSubMessage("\'" #isTrue "\' is true"); \
				Utility::PrintSubMessage(__VA_ARGS__); \
				Utility::Print("\n"); \
			} \
		}

	#define WARN_ONCE_IF_NOT( isTrue, ... ) WARN_ONCE_IF(!(isTrue), __VA_ARGS__)

	#define ERROR( ... ) \
			Utility::Print("\nError reported in " STRINGIFY_BUILTIN(__FILE__) " @ " STRINGIFY_BUILTIN(__LINE__) "\n"); \
			Utility::PrintSubMessage(__VA_ARGS__); \
			Utility::Print("\n");

	#define DEBUGPRINT( msg, ... ) \
		Utility::Printf( msg "\n", ##__VA_ARGS__ );
	
#endif // RELEASE

#define BreakIfFailed(hr) if(FAILED(hr)) __debugbreak()

void SIMDMemCopy(void* __restrict _Dest, const void* __restrict _Source, size_t NumQuadwords);
void SIMDMemFill(void* __restrict _Dest, __m128 FillVector, size_t NumQuadwords);

std::wstring MakeWStr(const std::string& str );

