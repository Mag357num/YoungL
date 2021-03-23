#pragma once

#include "pch.h"


namespace GameCore
{
	class IGameApp
	{
	public:
		// This function can be used to initialize application state and will run after essential
		// hardware resources are allocated.  Some state that does not depend on these resources
		// should still be initialized in the constructor such as pointers and flags.
		virtual void Startup(void) = 0;
		virtual void Cleanup(void) = 0;

		// Decide if you want the app to exit.  By default, app continues until the 'ESC' key is pressed.
		virtual bool IsDone(void);

		// The update method will be invoked once per frame.  Both state updating and scene
		// rendering should be handled by this method.
		virtual void Update(float deltaT) = 0;

		// Official rendering pass
		virtual void RenderScene(void) = 0;

		// Optional UI (overlay) rendering pass.  This is LDR.  The buffer is already cleared.
		virtual void RenderUI(class GraphicsContext&) {};
	};

	void RunApplication(IGameApp& app, const wchar_t* className, HINSTANCE hInst, int nCmdShow);
}

//#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
//#define MAIN_FUNCTION()  int wWinMain(/*int argc, wchar_t** argv*/)
//#else
//#define MAIN_FUNCTION()  [Platform::MTAThread] int main(Platform::Array<Platform::String^>^)
//#endif

#define CREATE_APPLICATION( app_class ) \
    int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE /*hPrevInstance*/, _In_ LPWSTR /*lpCmdLine*/, _In_ int nCmdShow) \
    { \
        return GameCore::RunApplication( app_class(), L#app_class, hInstance, nCmdShow ); \
    }
