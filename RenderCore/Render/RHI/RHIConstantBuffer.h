#pragma once


template<typename T>
class IRHIConstantBuffer
{
public:
	IRHIConstantBuffer() {}
	virtual ~IRHIConstantBuffer() {}

	virtual void CopyData(int ElementIndex, const T& Data) {}

	UINT GetRootParameterIndex() { return RootParameterIndex; }
	void SetRootParameterIndex(UINT InIndex) { RootParameterIndex = InIndex; }
protected:
	UINT RootParameterIndex;
};

