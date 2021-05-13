#pragma once

enum EPrimitiveTopology
{
	PrimitiveTopology_TRIANGLELIST = 0,
};

class IRHIIndexBuffer
{
public:
	IRHIIndexBuffer() {}
	virtual ~IRHIIndexBuffer() {}
};

