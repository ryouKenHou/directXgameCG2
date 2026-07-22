#pragma once
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

class InputSystem {
	public:
		InputSystem();
		~InputSystem();
		void Initialize(HINSTANCE hInstance, HWND hwnd);
		void Update();
		bool IsKeyPressed(int key);
		bool IsKeyReleased(int key);
		bool IsKeyHeld(int key);

		BYTE* getKeyboardState() { return keyboardState; }
		BYTE* getPrevKeyboardState() { return prevKeyboardState; }

	private:
		IDirectInput8* directInput;
		IDirectInputDevice8* keyboard;
		BYTE keyboardState[256];
		BYTE prevKeyboardState[256];
};