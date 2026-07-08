#pragma once

#include <Windows.h>
#include <cstdint>

// Forward declaration of the window procedure
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

class WindowManager {
private:
    WNDCLASS windowClass_;
    HWND hwnd_;
    int32_t clientWidth_;
    int32_t clientHeight_;

public:
    // Inline getters can stay in the header for performance
    WNDCLASS getWindowClass() const { return windowClass_; }
    HWND getHwnd() const { return hwnd_; }

    // Only declarations go here
    void Initialize(int nCmdShow, int width, int height, const wchar_t* windowTitle);
    void Finalize();
};