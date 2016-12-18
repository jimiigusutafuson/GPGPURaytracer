#pragma once
#include "Global.h"
#include <dinput.h>
class Input
{
private:
	static Input			*instance;
	LPDIRECTINPUT8			directInput;

	LPDIRECTINPUTDEVICE8	keyboardInput;
	LPDIRECTINPUTDEVICE8	mouseInput;

	//state
	char					keyState[256], prevKeyState[256];
	DIMOUSESTATE			mouseState, prevMouseState;

	Input();

public:
	static Input			*getInstance()
	{
		if ( !instance )
			instance = new Input();
		return instance;
	}

	bool					init( HINSTANCE *hInstance, HWND *hWnd );

	HRESULT					initKeyboard( HWND *hWnd );
	HRESULT					initMouse( HWND* hWnd );

	void					update();

	DIMOUSESTATE	*getMouseState() { return &mouseState; }
	DIMOUSESTATE	*getPrevMouseState() { return &prevMouseState; }
	char			*getKeyState() { return keyState; }
	char			*getPrevKeyState() { return prevKeyState; }

	long			getDeltaY() { return mouseState.lY; }
	long			getDeltaX() { return mouseState.lX; }
	long			getDeltaZ() { return mouseState.lZ; }

	~Input();
};