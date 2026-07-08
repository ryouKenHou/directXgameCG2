#include "WindowManager.h"
#include <cstdio>

#include "../../externals/imgui/imgui.h"
#include "../../externals/imgui/imgui_impl_win32.h"
#include "../../externals/imgui/imgui_impl_dx12.h"

// ImGui handler declaration
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Definition of the global WndProc
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

// Definition of WindowManager methods
void WindowManager::Initialize(int nCmdShow, int width, int height, const wchar_t* windowTitle) {
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

void WindowManager::Finalize() {
    // Note: If you call DestroyWindow(hwnd_), Windows will automatically 
    // send a WM_DESTROY message to your WndProc handler.
    if (hwnd_) {
        DestroyWindow(hwnd_);
    }
    CoUninitialize();
}