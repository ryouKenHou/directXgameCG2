#include "InputSystem.h"
#include <assert.h>

InputSystem::InputSystem() {
	directInput = nullptr;
	keyboard = nullptr;
	memset(keyboardState, 0, sizeof(keyboardState));
	memset(prevKeyboardState, 0, sizeof(prevKeyboardState));
};

InputSystem::~InputSystem() {
	if (keyboard) {
		keyboard->Unacquire();
		keyboard->Release();
		keyboard = nullptr;
	}
	if (directInput) {
		directInput->Release();
		directInput = nullptr;
	}
}

void InputSystem::Initialize(HINSTANCE hInstance, HWND hwnd) {
	HRESULT hr = DirectInput8Create(hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&directInput, nullptr);
	assert(SUCCEEDED(hr));
	hr = directInput->CreateDevice(GUID_SysKeyboard, &keyboard, nullptr);
	assert(SUCCEEDED(hr));
	hr = keyboard->SetDataFormat(&c_dfDIKeyboard);
	assert(SUCCEEDED(hr));
	hr = keyboard->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	assert(SUCCEEDED(hr));

}

void InputSystem::Update() {
	memcpy(prevKeyboardState, keyboardState, sizeof(keyboardState));
	HRESULT hr = keyboard->GetDeviceState(sizeof(keyboardState), keyboardState);
	if (FAILED(hr)) {
		hr = keyboard->Acquire();
		if (SUCCEEDED(hr)) {
			keyboard->GetDeviceState(sizeof(keyboardState), keyboardState);
		}
	}
}

bool InputSystem::IsKeyPressed(int key) {
	return keyboardState[key] & 0x80 && !(prevKeyboardState[key] & 0x80);
}

bool InputSystem::IsKeyReleased(int key) {
	return !(keyboardState[key] & 0x80) && prevKeyboardState[key] & 0x80;
}

bool InputSystem::IsKeyHeld(int key) {
	return keyboardState[key] & 0x80;
}