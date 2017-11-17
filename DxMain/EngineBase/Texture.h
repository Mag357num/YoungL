#pragma once

#include "DXSample.h"
#include "LearnHelloWindow.h"

class TextureSample : public LearnHelloWindow
{
public:
	TextureSample(UINT width, UINT height, std::wstring name);
	~TextureSample();

	virtual void LoadAssets() override;

	virtual void PopulateCommandList() override;

protected:
private:
};
