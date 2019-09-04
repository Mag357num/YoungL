#include "stdafx.h"
#include "LitWavesApp.h"

LitWaveApp::LitWaveApp(HINSTANCE hInstance)
	:D3DApp(hInstance)
{

}

LitWaveApp::~LitWaveApp()
{
	if (md3dDevice != nullptr)
	{
		FlushCommandQueue();
	}
}