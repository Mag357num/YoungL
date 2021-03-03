#include "../pch.h"
#include "../GameCore.h"
#include "GameInput.h"

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
	#define USE_XINPUT
	#include <Xinput.h>
	#pragma comment(lib, "xinput9_1_0.lib")

	#define USE_KEYBOARD_MOUSE
	#define DIRECTINPUT_VERSION 0x0800
	#include <dinput.h>
	#pragma comment(lib, "dinput8.lib")
	#pragma comment(lib, "dxguid.lib")

	namespace GameCore
	{
		extern HWND g_hWnd;
	}

#else

	using namespace Windows::Gaming::Input;
	using namespace Windows::Foundation::Collections;

	#define USE_KEYBOARD_MOUSE

	struct DIMOUSESTATE2
	{
		LONG lX, lY, lZ;
		BYTE rgbButtons[8];
	};

#endif

namespace
{
	bool S_Buttons[2][GameInput::kNumDigitalInputs];
	float S_HoldDuration[GameInput::kNumDigitalInputs];
	float S_Analogs[GameInput::kNumAnalogInputs];
	float S_AnalogsTC[GameInput::kNumAnalogInputs];

#ifdef USE_KEYBOARD_MOUSE

	#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
		IDirectInput8A* S_DI;
		IDirectInputDevice8A* S_Keyboard;
		IDirectInputDevice8A* S_Mouse;
	#endif

	DIMOUSESTATE2 S_MouseState;
	unsigned char S_KeyBuffer[256];
	unsigned char S_DXKeyMapping[GameInput::kNumKeys];

#endif // USE_KEYBOARD_MOUSE

#ifdef USE_XINPUT
	float FilterAnalogInput(int Val, int DeadZone)
	{
		if (Val < 0)
		{
			if (Val > -DeadZone)
			{
				return 0.0f;
			}
			else
			{
				return (Val + DeadZone) / (32768.0f - DeadZone);
			}
		}
		else
		{
			if (Val < DeadZone)
			{
				return 0.0f;
			}
			else
			{
				return (Val - DeadZone) / (32767.0f - DeadZone);
			}
		}
	}

#else

	float FilterAnalogInput(int Val, int DeadZone)
	{
		if (Val < -DeadZone)
		{
			return (Val + DeadZone) / (1.0f - DeadZone);
		}
		else if (Val > DeadZone)
		{
			return (Val - DeadZone) / (1.0f - DeadZone);
		}
		else
			return 0.0f;
	}


#endif // USE_XINPUT

#ifdef USE_KEYBOARD_MOUSE
	void KbmBuildKeyMapping()
	{
		#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
			S_DXKeyMapping[GameInput::kKey_escape] = 1;
			S_DXKeyMapping[GameInput::kKey_1] = 2;
			S_DXKeyMapping[GameInput::kKey_2] = 3;
			S_DXKeyMapping[GameInput::kKey_3] = 4;
			S_DXKeyMapping[GameInput::kKey_4] = 5;
			S_DXKeyMapping[GameInput::kKey_5] = 6;
			S_DXKeyMapping[GameInput::kKey_6] = 7;
			S_DXKeyMapping[GameInput::kKey_7] = 8;
			S_DXKeyMapping[GameInput::kKey_8] = 9;
			S_DXKeyMapping[GameInput::kKey_9] = 10;
			S_DXKeyMapping[GameInput::kKey_0] = 11;
			S_DXKeyMapping[GameInput::kKey_minus] = 12;
			S_DXKeyMapping[GameInput::kKey_equals] = 13;
			S_DXKeyMapping[GameInput::kKey_back] = 14;
			S_DXKeyMapping[GameInput::kKey_tab] = 15;
			S_DXKeyMapping[GameInput::kKey_q] = 16;
			S_DXKeyMapping[GameInput::kKey_w] = 17;
			S_DXKeyMapping[GameInput::kKey_e] = 18;
			S_DXKeyMapping[GameInput::kKey_r] = 19;
			S_DXKeyMapping[GameInput::kKey_t] = 20;
			S_DXKeyMapping[GameInput::kKey_y] = 21;
			S_DXKeyMapping[GameInput::kKey_u] = 22;
			S_DXKeyMapping[GameInput::kKey_i] = 23;
			S_DXKeyMapping[GameInput::kKey_o] = 24;
			S_DXKeyMapping[GameInput::kKey_p] = 25;
			S_DXKeyMapping[GameInput::kKey_lbracket] = 26;
			S_DXKeyMapping[GameInput::kKey_rbracket] = 27;
			S_DXKeyMapping[GameInput::kKey_return] = 28;
			S_DXKeyMapping[GameInput::kKey_lcontrol] = 29;
			S_DXKeyMapping[GameInput::kKey_a] = 30;
			S_DXKeyMapping[GameInput::kKey_s] = 31;
			S_DXKeyMapping[GameInput::kKey_d] = 32;
			S_DXKeyMapping[GameInput::kKey_f] = 33;
			S_DXKeyMapping[GameInput::kKey_g] = 34;
			S_DXKeyMapping[GameInput::kKey_h] = 35;
			S_DXKeyMapping[GameInput::kKey_j] = 36;
			S_DXKeyMapping[GameInput::kKey_k] = 37;
			S_DXKeyMapping[GameInput::kKey_l] = 38;
			S_DXKeyMapping[GameInput::kKey_semicolon] = 39;
			S_DXKeyMapping[GameInput::kKey_apostrophe] = 40;
			S_DXKeyMapping[GameInput::kKey_grave] = 41;
			S_DXKeyMapping[GameInput::kKey_lshift] = 42;
			S_DXKeyMapping[GameInput::kKey_backslash] = 43;
			S_DXKeyMapping[GameInput::kKey_z] = 44;
			S_DXKeyMapping[GameInput::kKey_x] = 45;
			S_DXKeyMapping[GameInput::kKey_c] = 46;
			S_DXKeyMapping[GameInput::kKey_v] = 47;
			S_DXKeyMapping[GameInput::kKey_b] = 48;
			S_DXKeyMapping[GameInput::kKey_n] = 49;
			S_DXKeyMapping[GameInput::kKey_m] = 50;
			S_DXKeyMapping[GameInput::kKey_comma] = 51;
			S_DXKeyMapping[GameInput::kKey_period] = 52;
			S_DXKeyMapping[GameInput::kKey_slash] = 53;
			S_DXKeyMapping[GameInput::kKey_rshift] = 54;
			S_DXKeyMapping[GameInput::kKey_multiply] = 55;
			S_DXKeyMapping[GameInput::kKey_lalt] = 56;
			S_DXKeyMapping[GameInput::kKey_space] = 57;
			S_DXKeyMapping[GameInput::kKey_capital] = 58;
			S_DXKeyMapping[GameInput::kKey_f1] = 59;
			S_DXKeyMapping[GameInput::kKey_f2] = 60;
			S_DXKeyMapping[GameInput::kKey_f3] = 61;
			S_DXKeyMapping[GameInput::kKey_f4] = 62;
			S_DXKeyMapping[GameInput::kKey_f5] = 63;
			S_DXKeyMapping[GameInput::kKey_f6] = 64;
			S_DXKeyMapping[GameInput::kKey_f7] = 65;
			S_DXKeyMapping[GameInput::kKey_f8] = 66;
			S_DXKeyMapping[GameInput::kKey_f9] = 67;
			S_DXKeyMapping[GameInput::kKey_f10] = 68;
			S_DXKeyMapping[GameInput::kKey_numlock] = 69;
			S_DXKeyMapping[GameInput::kKey_scroll] = 70;
			S_DXKeyMapping[GameInput::kKey_numpad7] = 71;
			S_DXKeyMapping[GameInput::kKey_numpad8] = 72;
			S_DXKeyMapping[GameInput::kKey_numpad9] = 73;
			S_DXKeyMapping[GameInput::kKey_subtract] = 74;
			S_DXKeyMapping[GameInput::kKey_numpad4] = 75;
			S_DXKeyMapping[GameInput::kKey_numpad5] = 76;
			S_DXKeyMapping[GameInput::kKey_numpad6] = 77;
			S_DXKeyMapping[GameInput::kKey_add] = 78;
			S_DXKeyMapping[GameInput::kKey_numpad1] = 79;
			S_DXKeyMapping[GameInput::kKey_numpad2] = 80;
			S_DXKeyMapping[GameInput::kKey_numpad3] = 81;
			S_DXKeyMapping[GameInput::kKey_numpad0] = 82;
			S_DXKeyMapping[GameInput::kKey_decimal] = 83;
			S_DXKeyMapping[GameInput::kKey_f11] = 87;
			S_DXKeyMapping[GameInput::kKey_f12] = 88;
			S_DXKeyMapping[GameInput::kKey_numpadenter] = 156;
			S_DXKeyMapping[GameInput::kKey_rcontrol] = 157;
			S_DXKeyMapping[GameInput::kKey_divide] = 181;
			S_DXKeyMapping[GameInput::kKey_sysrq] = 183;
			S_DXKeyMapping[GameInput::kKey_ralt] = 184;
			S_DXKeyMapping[GameInput::kKey_pause] = 197;
			S_DXKeyMapping[GameInput::kKey_home] = 199;
			S_DXKeyMapping[GameInput::kKey_up] = 200;
			S_DXKeyMapping[GameInput::kKey_pgup] = 201;
			S_DXKeyMapping[GameInput::kKey_left] = 203;
			S_DXKeyMapping[GameInput::kKey_right] = 205;
			S_DXKeyMapping[GameInput::kKey_end] = 207;
			S_DXKeyMapping[GameInput::kKey_down] = 208;
			S_DXKeyMapping[GameInput::kKey_pgdn] = 209;
			S_DXKeyMapping[GameInput::kKey_insert] = 210;
			S_DXKeyMapping[GameInput::kKey_delete] = 211;
			S_DXKeyMapping[GameInput::kKey_lwin] = 219;
			S_DXKeyMapping[GameInput::kKey_rwin] = 220;
			S_DXKeyMapping[GameInput::kKey_apps] = 221;
		#else
			#define WinRTKey(name) (unsigned char)Windows::System::VirtualKey::name
			S_DXKeyMapping[GameInput::kKey_escape] = WinRTKey(Escape);
			S_DXKeyMapping[GameInput::kKey_1] = WinRTKey(Number1);
			S_DXKeyMapping[GameInput::kKey_2] = WinRTKey(Number2);
			S_DXKeyMapping[GameInput::kKey_3] = WinRTKey(Number3);
			S_DXKeyMapping[GameInput::kKey_4] = WinRTKey(Number4);
			S_DXKeyMapping[GameInput::kKey_5] = WinRTKey(Number5);
			S_DXKeyMapping[GameInput::kKey_6] = WinRTKey(Number6);
			S_DXKeyMapping[GameInput::kKey_7] = WinRTKey(Number7);
			S_DXKeyMapping[GameInput::kKey_8] = WinRTKey(Number8);
			S_DXKeyMapping[GameInput::kKey_9] = WinRTKey(Number9);
			S_DXKeyMapping[GameInput::kKey_0] = WinRTKey(Number0);
			S_DXKeyMapping[GameInput::kKey_minus] = WinRTKey(Subtract);
			S_DXKeyMapping[GameInput::kKey_equals] = WinRTKey(Add);
			S_DXKeyMapping[GameInput::kKey_back] = WinRTKey(Back);
			S_DXKeyMapping[GameInput::kKey_tab] = WinRTKey(Tab);
			S_DXKeyMapping[GameInput::kKey_q] = WinRTKey(Q);
			S_DXKeyMapping[GameInput::kKey_w] = WinRTKey(W);
			S_DXKeyMapping[GameInput::kKey_e] = WinRTKey(E);
			S_DXKeyMapping[GameInput::kKey_r] = WinRTKey(R);
			S_DXKeyMapping[GameInput::kKey_t] = WinRTKey(T);
			S_DXKeyMapping[GameInput::kKey_y] = WinRTKey(Y);
			S_DXKeyMapping[GameInput::kKey_u] = WinRTKey(U);
			S_DXKeyMapping[GameInput::kKey_i] = WinRTKey(I);
			S_DXKeyMapping[GameInput::kKey_o] = WinRTKey(O);
			S_DXKeyMapping[GameInput::kKey_p] = WinRTKey(P);
			S_DXKeyMapping[GameInput::kKey_lbracket] = 219;
			S_DXKeyMapping[GameInput::kKey_rbracket] = 221;
			S_DXKeyMapping[GameInput::kKey_return] = WinRTKey(Enter);
			S_DXKeyMapping[GameInput::kKey_lcontrol] = WinRTKey(Control);  // No L/R
			S_DXKeyMapping[GameInput::kKey_a] = WinRTKey(A);
			S_DXKeyMapping[GameInput::kKey_s] = WinRTKey(S);
			S_DXKeyMapping[GameInput::kKey_d] = WinRTKey(D);
			S_DXKeyMapping[GameInput::kKey_f] = WinRTKey(F);
			S_DXKeyMapping[GameInput::kKey_g] = WinRTKey(G);
			S_DXKeyMapping[GameInput::kKey_h] = WinRTKey(H);
			S_DXKeyMapping[GameInput::kKey_j] = WinRTKey(J);
			S_DXKeyMapping[GameInput::kKey_k] = WinRTKey(K);
			S_DXKeyMapping[GameInput::kKey_l] = WinRTKey(L);
			S_DXKeyMapping[GameInput::kKey_semicolon] = 186;
			S_DXKeyMapping[GameInput::kKey_apostrophe] = 222;
			S_DXKeyMapping[GameInput::kKey_grave] = 192; // ` or ~
			S_DXKeyMapping[GameInput::kKey_lshift] = WinRTKey(LeftShift);
			S_DXKeyMapping[GameInput::kKey_backslash] = 220;
			S_DXKeyMapping[GameInput::kKey_z] = WinRTKey(Z);
			S_DXKeyMapping[GameInput::kKey_x] = WinRTKey(X);
			S_DXKeyMapping[GameInput::kKey_c] = WinRTKey(C);
			S_DXKeyMapping[GameInput::kKey_v] = WinRTKey(V);
			S_DXKeyMapping[GameInput::kKey_b] = WinRTKey(B);
			S_DXKeyMapping[GameInput::kKey_n] = WinRTKey(N);
			S_DXKeyMapping[GameInput::kKey_m] = WinRTKey(M);
			S_DXKeyMapping[GameInput::kKey_comma] = 188;
			S_DXKeyMapping[GameInput::kKey_period] = 190;
			S_DXKeyMapping[GameInput::kKey_slash] = 191;
			S_DXKeyMapping[GameInput::kKey_rshift] = WinRTKey(RightShift);
			S_DXKeyMapping[GameInput::kKey_multiply] = WinRTKey(Multiply);
			S_DXKeyMapping[GameInput::kKey_lalt] = 255; // Only a modifier
			S_DXKeyMapping[GameInput::kKey_space] = WinRTKey(Space);
			S_DXKeyMapping[GameInput::kKey_capital] = WinRTKey(CapitalLock);
			S_DXKeyMapping[GameInput::kKey_f1] = WinRTKey(F1);
			S_DXKeyMapping[GameInput::kKey_f2] = WinRTKey(F2);
			S_DXKeyMapping[GameInput::kKey_f3] = WinRTKey(F3);
			S_DXKeyMapping[GameInput::kKey_f4] = WinRTKey(F4);
			S_DXKeyMapping[GameInput::kKey_f5] = WinRTKey(F5);
			S_DXKeyMapping[GameInput::kKey_f6] = WinRTKey(F6);
			S_DXKeyMapping[GameInput::kKey_f7] = WinRTKey(F7);
			S_DXKeyMapping[GameInput::kKey_f8] = WinRTKey(F8);
			S_DXKeyMapping[GameInput::kKey_f9] = WinRTKey(F9);
			S_DXKeyMapping[GameInput::kKey_f10] = WinRTKey(F10);
			S_DXKeyMapping[GameInput::kKey_numlock] = WinRTKey(NumberKeyLock);
			S_DXKeyMapping[GameInput::kKey_scroll] = WinRTKey(Scroll);
			S_DXKeyMapping[GameInput::kKey_numpad7] = WinRTKey(NumberPad7);
			S_DXKeyMapping[GameInput::kKey_numpad8] = WinRTKey(NumberPad8);
			S_DXKeyMapping[GameInput::kKey_numpad9] = WinRTKey(NumberPad9);
			S_DXKeyMapping[GameInput::kKey_subtract] = WinRTKey(Subtract);
			S_DXKeyMapping[GameInput::kKey_numpad4] = WinRTKey(NumberPad4);
			S_DXKeyMapping[GameInput::kKey_numpad5] = WinRTKey(NumberPad5);
			S_DXKeyMapping[GameInput::kKey_numpad6] = WinRTKey(NumberPad6);
			S_DXKeyMapping[GameInput::kKey_add] = WinRTKey(Add);
			S_DXKeyMapping[GameInput::kKey_numpad1] = WinRTKey(NumberPad1);
			S_DXKeyMapping[GameInput::kKey_numpad2] = WinRTKey(NumberPad2);
			S_DXKeyMapping[GameInput::kKey_numpad3] = WinRTKey(NumberPad3);
			S_DXKeyMapping[GameInput::kKey_numpad0] = WinRTKey(NumberPad0);
			S_DXKeyMapping[GameInput::kKey_decimal] = WinRTKey(Decimal);
			S_DXKeyMapping[GameInput::kKey_f11] = WinRTKey(F11);
			S_DXKeyMapping[GameInput::kKey_f12] = WinRTKey(F12);
			S_DXKeyMapping[GameInput::kKey_numpadenter] = WinRTKey(Enter); // No distinction
			S_DXKeyMapping[GameInput::kKey_rcontrol] = WinRTKey(Control);  // No L/R
			S_DXKeyMapping[GameInput::kKey_divide] = WinRTKey(Divide);
			S_DXKeyMapping[GameInput::kKey_sysrq] = 255; // Ignored
			S_DXKeyMapping[GameInput::kKey_ralt] = 255; // Only a modifier
			S_DXKeyMapping[GameInput::kKey_pause] = WinRTKey(Pause);
			S_DXKeyMapping[GameInput::kKey_home] = WinRTKey(Home);
			S_DXKeyMapping[GameInput::kKey_up] = WinRTKey(Up);
			S_DXKeyMapping[GameInput::kKey_pgup] = WinRTKey(PageUp);
			S_DXKeyMapping[GameInput::kKey_left] = WinRTKey(Left);
			S_DXKeyMapping[GameInput::kKey_right] = WinRTKey(Right);
			S_DXKeyMapping[GameInput::kKey_end] = WinRTKey(End);
			S_DXKeyMapping[GameInput::kKey_down] = WinRTKey(Down);
			S_DXKeyMapping[GameInput::kKey_pgdn] = WinRTKey(PageDown);
			S_DXKeyMapping[GameInput::kKey_insert] = WinRTKey(Insert);
			S_DXKeyMapping[GameInput::kKey_delete] = WinRTKey(Delete);
			S_DXKeyMapping[GameInput::kKey_lwin] = WinRTKey(LeftWindows);
			S_DXKeyMapping[GameInput::kKey_rwin] = WinRTKey(RightWindows);
			S_DXKeyMapping[GameInput::kKey_apps] = WinRTKey(Application);
		#endif
	}

	void KbmZeroInputs()
	{
		memset(&S_MouseState, 0, sizeof(DIMOUSESTATE2));
		memset(S_KeyBuffer, 0, sizeof(S_KeyBuffer));
	}

	void KbmInitialize()
	{
		KbmBuildKeyMapping();

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
		if (FAILED(DirectInput8Create(GetModuleHandle(nullptr), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&S_DI, nullptr)))
		{
			ASSERT(false, "DirectInput8 initialization failed!");
		}

		if (FAILED(S_DI->CreateDevice(GUID_SysKeyboard, &S_Keyboard, nullptr)))
		{
			ASSERT(false, "Keyboard create device failed");
		}

		if (FAILED(S_Keyboard->SetCooperativeLevel(GameCore::g_hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE)))
		{
			ASSERT(false, "Keyboard setcooperativelevle failed");
		}

		DIPROPDWORD dipdw;
		dipdw.diph.dwSize = sizeof(DIPROPDWORD);
		dipdw.diph.dwHeaderSize = sizeof(DIPROPDWORD);
		dipdw.diph.dwObj = 0;
		dipdw.diph.dwHow = DIPH_DEVICE;
		dipdw.dwData = 10;

		if (FAILED(S_Keyboard->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph)))
		{
			ASSERT(false, "keyboard set buffer size failed");
		}

		if (FAILED(S_DI->CreateDevice(GUID_SysMouse, &S_Mouse, nullptr)))
		{
			ASSERT(false, "Mouse device create failed");
		}
		if (FAILED(S_Mouse->SetDataFormat(&c_dfDIMouse2)))
		{
			ASSERT(false, "mouse set data format failed");
		}
		if (FAILED(S_Mouse->SetCooperativeLevel(GameCore::g_hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE)))
		{
			ASSERT(false, "Mouse Set cooperative level failed");
		}

#endif

		KbmZeroInputs();
	}

	void KbmShutdown()
	{
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
		if (S_Keyboard)
		{
			S_Keyboard->Unacquire();
			S_Keyboard->Release();
			S_Keyboard = nullptr;
		}

		if (S_Mouse)
		{
			S_Mouse->Unacquire();
			S_Mouse->Release();
			S_Mouse = nullptr;
		}

		if (S_DI)
		{
			S_DI->Release();
			S_DI = nullptr;
		}
#endif
	}

	void KbmUpdate()
	{
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
		HWND foreground = GetForegroundWindow();
		bool visible = IsWindowVisible(foreground);

		if (foreground != GameCore::g_hWnd || !visible)
		{
			KbmZeroInputs();
		}
		else
		{
			S_Mouse->Acquire();
			S_Mouse->GetDeviceState(sizeof(DIMOUSESTATE2), &S_MouseState);

			S_Keyboard->Acquire();
			S_Keyboard->GetDeviceState(sizeof(S_KeyBuffer), S_KeyBuffer);
		}
#endif
	}

#endif

}

#if !WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP | WINAPI_PARTITION_TV_TITLE)
void GameInput::SetKeyState(Windows::System::VirtualKey key, bool IsDown)
{
	S_KeyBuffer[(unsigned char)key] = IsDown ? 0x80 : 0x00;
}
#endif

void GameInput::Initialize()
{
	ZeroMemory(S_Buttons, sizeof(S_Buttons));
	ZeroMemory(S_Analogs, sizeof(S_Analogs));

#ifdef USE_KEYBOARD_MOUSE
	KbmInitialize();
#endif
}

void GameInput::ShutDown()
{
#ifdef USE_KEYBOARD_MOUSE
	KbmShutdown();
#endif // USE_KEYBOARD_MOUSE

}


void GameInput::Update(float frameDelta)
{
	memcpy(S_Buttons[1], S_Buttons[0], sizeof(S_Buttons[0]));
	memset(S_Buttons[0], 0, sizeof(S_Buttons[0]));
	memset(S_Analogs, 0, sizeof(S_Analogs));

#ifdef USE_XINPUT
	XINPUT_STATE newInputState;
	if (ERROR_SUCCESS == XInputGetState(0, &newInputState))
	{
		if (newInputState.Gamepad.wButtons & (1 << 0)) S_Buttons[0][kDPadUp] = true;
		if (newInputState.Gamepad.wButtons & (1 << 1)) S_Buttons[0][kDPadDown] = true;
		if (newInputState.Gamepad.wButtons & (1 << 2)) S_Buttons[0][kDPadLeft] = true;
		if (newInputState.Gamepad.wButtons & (1 << 3)) S_Buttons[0][kDPadRight] = true;
		if (newInputState.Gamepad.wButtons & (1 << 4)) S_Buttons[0][kStartButton] = true;
		if (newInputState.Gamepad.wButtons & (1 << 5)) S_Buttons[0][kBackButton] = true;
		if (newInputState.Gamepad.wButtons & (1 << 6)) S_Buttons[0][kLThumbClick] = true;
		if (newInputState.Gamepad.wButtons & (1 << 7)) S_Buttons[0][kRThumbClick] = true;
		if (newInputState.Gamepad.wButtons & (1 << 8)) S_Buttons[0][kLShoulder] = true;
		if (newInputState.Gamepad.wButtons & (1 << 9)) S_Buttons[0][kRShoulder] = true;
		if (newInputState.Gamepad.wButtons & (1 << 12)) S_Buttons[0][kAButton] = true;
		if (newInputState.Gamepad.wButtons & (1 << 13)) S_Buttons[0][kBButton] = true;
		if (newInputState.Gamepad.wButtons & (1 << 14)) S_Buttons[0][kXButton] = true;
		if (newInputState.Gamepad.wButtons & (1 << 15)) S_Buttons[0][kYButton] = true;

		S_Analogs[kAnalogLeftTrigger] = newInputState.Gamepad.bLeftTrigger / 255.0f;
		S_Analogs[kAnalogRightTrigger] = newInputState.Gamepad.bRightTrigger / 255.0f;
		S_Analogs[kAnalogLeftStickX] = FilterAnalogInput(newInputState.Gamepad.sThumbLX, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
		S_Analogs[kAnalogLeftStickY] = FilterAnalogInput(newInputState.Gamepad.sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
		S_Analogs[kAnalogRightStickX] = FilterAnalogInput(newInputState.Gamepad.sThumbRX, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
		S_Analogs[kAnalogRightStickY] = FilterAnalogInput(newInputState.Gamepad.sThumbRY, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
	}

#else
	IVectorView<Gamepad^>^ gamepads = Gamepad::Gamepads;
	if (gamepads->Size != 0)
	{
		IGamepad^ gamepad = gamepads->GetAt(0);
		GamepadReading reading = gamepad->GetCurrentReading();
		uint32_t Buttons = (uint32_t)reading.Buttons;
		if (Buttons & (uint32_t)GamepadButtons::DPadUp) S_Buttons[0][kDPadUp] = true;
		if (Buttons & (uint32_t)GamepadButtons::DPadDown) S_Buttons[0][kDPadDown] = true;
		if (Buttons & (uint32_t)GamepadButtons::DPadLeft) S_Buttons[0][kDPadLeft] = true;
		if (Buttons & (uint32_t)GamepadButtons::DPadRight) S_Buttons[0][kDPadRight] = true;
		if (Buttons & (uint32_t)GamepadButtons::Menu) S_Buttons[0][kStartButton] = true;
		if (Buttons & (uint32_t)GamepadButtons::View) S_Buttons[0][kBackButton] = true;
		if (Buttons & (uint32_t)GamepadButtons::LeftThumbstick) S_Buttons[0][kLThumbClick] = true;
		if (Buttons & (uint32_t)GamepadButtons::RightThumbstick) S_Buttons[0][kRThumbClick] = true;
		if (Buttons & (uint32_t)GamepadButtons::LeftShoulder) S_Buttons[0][kLShoulder] = true;
		if (Buttons & (uint32_t)GamepadButtons::RightShoulder) S_Buttons[0][kRShoulder] = true;
		if (Buttons & (uint32_t)GamepadButtons::A) S_Buttons[0][kAButton] = true;
		if (Buttons & (uint32_t)GamepadButtons::B) S_Buttons[0][kBButton] = true;
		if (Buttons & (uint32_t)GamepadButtons::X) S_Buttons[0][kXButton] = true;
		if (Buttons & (uint32_t)GamepadButtons::Y) S_Buttons[0][kYButton] = true;

		static const float kAnalogStickDeadZone = 0.18f;

		S_Analogs[kAnalogLeftTrigger] = (float)reading.LeftTrigger;
		S_Analogs[kAnalogRightTrigger] = (float)reading.RightTrigger;
		S_Analogs[kAnalogLeftStickX] = FilterAnalogInput((float)reading.LeftThumbstickX, kAnalogStickDeadZone);
		S_Analogs[kAnalogLeftStickY] = FilterAnalogInput((float)reading.LeftThumbstickY, kAnalogStickDeadZone);
		S_Analogs[kAnalogRightStickX] = FilterAnalogInput((float)reading.RightThumbstickX, kAnalogStickDeadZone);
		S_Analogs[kAnalogRightStickY] = FilterAnalogInput((float)reading.RightThumbstickY, kAnalogStickDeadZone);
	}
#endif // USE_XINPUT

#ifdef USE_KEYBOARD_MOUSE
	KbmUpdate();

	for (uint32_t i = 0; i < kNumKeys; ++i)
	{
		S_Buttons[0][i] = (S_KeyBuffer[S_DXKeyMapping[i]] & 0x80) != 0;
	}

	for (uint32_t i = 0; i < 8; ++i)
	{
		if (S_MouseState.rgbButtons[i] > 0) S_Buttons[0][kMouse0 + i] = true;
	}

	S_Analogs[kAnalogMouseX] = (float)S_MouseState.lX * .0018f;
	S_Analogs[kAnalogMouseY] = (float)S_MouseState.lY * -.0018f;

	if (S_MouseState.lZ > 0)
		S_Analogs[kAnalogMouseScroll] = 1.0f;
	else if (S_MouseState.lZ < 0)
		S_Analogs[kAnalogMouseScroll] = -1.0f;
#endif

	for (uint32_t i=0; i< kNumDigitalInputs; ++i)
	{
		if (S_Buttons[0][i])
		{
			if (!S_Buttons[1][i])
				S_HoldDuration[i] = 0.0f;
			else
				S_HoldDuration[i] += frameDelta;
		}
	}

	for (uint32_t i = 0; i < kNumAnalogInputs; ++i)
	{
		S_AnalogsTC[i] = S_AnalogsTC[i] * frameDelta;
	}

}


bool GameInput::IsAnyPressed(void)
{
	return S_Buttons[0] != 0;
}

bool GameInput::IsPressed(EDigitalInput di)
{
	return S_Buttons[0][di];
}

bool GameInput::IsFirstPressed(EDigitalInput di)
{
	return S_Buttons[0][di] && !S_Buttons[1][di];
}

bool GameInput::IsReleased(EDigitalInput di)
{
	return !S_Buttons[0][di];
}

bool GameInput::IsFirstReleased(EDigitalInput di)
{
	return !S_Buttons[0][di] && S_Buttons[1][di];
}

float GameInput::GetDurationPressed(EDigitalInput di)
{
	return S_HoldDuration[di];
}

float GameInput::GetAnalogInput(EAnalogInput ai)
{
	return S_Analogs[ai];
}

float GameInput::GetTimeCorrectedAnalogInput(EAnalogInput ai)
{
	return S_AnalogsTC[ai];
}