#pragma once
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_win32.h"
#include "externals/imgui/imgui_impl_dx12.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

#include <Windows.h>
#include <string>

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam)) {
		return true;
	}

	switch (msg) {
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProcW(hwnd, msg, wparam, lparam);
}

class WindowManager {
	WNDCLASS windowClass_;
	HWND hwnd_;

	int32_t clientWidth_;
	int32_t clientHeight_;
public:

	WNDCLASS getWindowClass() const {
		return windowClass_;
	}

	HWND getHwnd() const {
		return hwnd_;
	}

	void Initialize(int nCmdShow, int width, int height, const wchar_t* windowTitle) {

		windowClass_ = {};

		windowClass_.lpfnWndProc = WndProc;
		windowClass_.lpszClassName = L"CG2WindowClass";
		windowClass_.hInstance = GetModuleHandleW(nullptr);
		windowClass_.hCursor = LoadCursorW(nullptr, IDC_ARROW);

		if (!RegisterClass(&windowClass_)) {
			OutputDebugStringW(L"RegisterClass failed!\n");
			return;
		}

		clientWidth_ = width;
		clientHeight_ = height;

		RECT wrc = { 0, 0, clientWidth_, clientHeight_ };

		AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, FALSE);

		hwnd_ = CreateWindowW(
			windowClass_.lpszClassName,
			windowTitle,
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			wrc.right - wrc.left,
			wrc.bottom - wrc.top,
			nullptr,
			nullptr,
			windowClass_.hInstance,
			nullptr
		);

		if (hwnd_ == nullptr) {
			DWORD error = GetLastError();
			wchar_t buffer[256];
			swprintf_s(buffer, L"CreateWindowW failed! Error: %d\n", error);
			OutputDebugStringW(buffer);
			return;
		}

		ShowWindow(hwnd_, nCmdShow);		
	}

	void Finalize() {
		CloseWindow(hwnd_);
		CoUninitialize();
	}

};