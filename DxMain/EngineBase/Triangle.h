#pragma once

#include "DXSample.h"
#include "LearnHelloWindow.h"

class TriangleSample : public LearnHelloWindow
{
public:
	TriangleSample(UINT width, UINT height, std::wstring name);
	~TriangleSample();

	virtual void LoadAssets() override;

	virtual void PopulateCommandList() override;

protected:
private:
};
