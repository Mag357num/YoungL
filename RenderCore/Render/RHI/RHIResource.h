#pragma once

#include "Formats.h"

class IRHIResource
{
public:
	IRHIResource() { IsDepth = false;}
	virtual ~IRHIResource() {}

	bool GetIsDepth(){return IsDepth;}
	void SetIsDepth(bool InFlag){ IsDepth = InFlag;}
protected:
private:
	//true: depth; false:Color
	bool IsDepth;
};
