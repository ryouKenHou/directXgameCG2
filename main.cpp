#include <Windows.h>
#include <cstdint>
#include "ConvertString.hpp"
#include <filesystem>
#include <fstream>
#include <chrono>

void Log(std::ostream& os, const std::string& message) {
	os << message <<std::endl;
	OutputDebugStringA(message.c_str());
}

void Log(std::ostream& os, const std::wstring& message) {
	Log(os, ConvertString(message));
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	switch (msg) {
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProcW(hwnd, msg, wparam, lparam);
}


int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {
	std::filesystem::create_directories("logs");

	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
	std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds> nowSeconds = std::chrono::time_point_cast<std::chrono::seconds>(now);
	std::chrono::zoned_time localTime{ std::chrono::current_zone(), nowSeconds };
	std::string dateString = std::format("{:%Y%m%d_%H%M%S}", localTime);
	std::string logFilePath = std::string("logs/") + dateString + ".log";
	std::ofstream logStream(logFilePath);


	//Log("Hello, DirectX!\n");
	Log(logStream, "Hello, DirectX!");

	WNDCLASS wc{};

	wc.lpfnWndProc = WndProc;
	wc.lpszClassName = L"CG2WindowClass";
	wc.hInstance = GetModuleHandleW(nullptr);
	wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);

	RegisterClass(&wc);

	const int32_t kClientWidth = 1280;
	const int32_t kClientHeight = 720;

	RECT wrc = { 0, 0, kClientWidth, kClientHeight };

	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, FALSE);

	HWND hwnd = CreateWindowW(
		wc.lpszClassName,
		L"CG2",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 
		CW_USEDEFAULT,
		wrc.right - wrc.left, 
		wrc.bottom - wrc.top,
		nullptr, 
		nullptr, 
		wc.hInstance, 
		nullptr
	);

	ShowWindow(hwnd, SW_SHOW);

	MSG msg{};

	while (msg.message != WM_QUIT) {
		if (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
		else {

		}
	}

	return 0;
}