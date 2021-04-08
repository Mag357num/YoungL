#pragma once

#include <intsafe.h>

class FGameCore
{
public:
	FGameCore(){}
	~FGameCore(){}

	virtual void Initialize(){}
	virtual void ShutDown(){}

	virtual void Tick(){}

	virtual void OnKeyDown(UINT8 Key);
	virtual void OnKeyUp(UINT8 Key);
private:
};

