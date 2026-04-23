#include <Windows.h>
#include <cstdint>
#include "ConvertString.hpp"
#include <filesystem>
#include <fstream>
#include <chrono>
#include <format>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <cassert>
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

#include <dbghelp.h>
#include <strsafe.h>
#pragma comment(lib, "dbghelp.lib")

void Log(std::ostream& os, const std::string& message) {
	os << message <<std::endl;
	OutputDebugStringA((message + "\n").c_str());
}

void Log(std::ostream& os, const std::wstring& message) {
	Log(os, ConvertString(message));
}

static LONG WINAPI ExportDump(EXCEPTION_POINTERS* exception) {
	SYSTEMTIME time;
	GetLocalTime(&time);
	wchar_t filePath[MAX_PATH] = { 0 };
	CreateDirectory(L"./dump", nullptr);
	StringCchPrintfW(filePath, MAX_PATH, L"./dump/%04d-%02d%02d-%02d%02d%02d.dmp",
		time.wYear, time.wMonth, time.wDay,
		time.wHour, time.wMinute);
	HANDLE dumpFileHandle = CreateFile(filePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ, 0, CREATE_ALWAYS, 0,0);
	DWORD processId = GetCurrentProcessId();
	DWORD threadId = GetCurrentThreadId();

	MINIDUMP_EXCEPTION_INFORMATION minidumpInformation{ 0 };
	minidumpInformation.ThreadId = threadId;
	minidumpInformation.ExceptionPointers = exception;
	minidumpInformation.ClientPointers = TRUE;

	MiniDumpWriteDump(GetCurrentProcess(), processId, dumpFileHandle, MiniDumpNormal, &minidumpInformation, nullptr, nullptr);

	return EXCEPTION_EXECUTE_HANDLER;
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
	SetUnhandledExceptionFilter(ExportDump);

	// =========================Create logs====================
	std::filesystem::create_directories("logs");

	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
	std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds> nowSeconds = std::chrono::time_point_cast<std::chrono::seconds>(now);
	std::chrono::zoned_time localTime{ std::chrono::current_zone(), nowSeconds };
	std::string dateString = std::format("{:%Y%m%d_%H%M%S}", localTime);
	std::string logFilePath = std::string("logs/") + dateString + ".log";
	std::ofstream logStream(logFilePath);


	//Log("Hello, DirectX!\n");
	Log(logStream, "Hello, DirectX!");

	// ======================= DXGI Factory ======================
	IDXGIFactory7* dxgiFactory = nullptr;
	HREFTYPE hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));
	assert(SUCCEEDED(hr));

	// =========================Select adapter====================
	IDXGIAdapter4* useAdapter = nullptr;
	for (UINT i = 0; dxgiFactory->EnumAdapterByGpuPreference(i, 
		DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, 
		IID_PPV_ARGS(&useAdapter)) != DXGI_ERROR_NOT_FOUND; ++i
		) {

		DXGI_ADAPTER_DESC3 adapterDesc{};
		hr = useAdapter->GetDesc3(&adapterDesc);
		assert(SUCCEEDED(hr));

		if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)) {
			Log(logStream, std::format("Use adapter: {}\n", ConvertString(static_cast<std::wstring> (adapterDesc.Description))));
			break;
		}
		useAdapter = nullptr;
	}

	assert(useAdapter != nullptr);

	// =========================Create device====================
	ID3D12Device* device = nullptr;

	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_12_2,
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
	};

	const char *featureLevelStrings[] = {
		"12.2",
		"12.1",
		"12.0",
	};

	for (size_t i = 0; i < std::size(featureLevels); ++i) {
		hr = D3D12CreateDevice(
			useAdapter,
			featureLevels[i],
			IID_PPV_ARGS(&device)
		);
		if (SUCCEEDED(hr)) {
			Log(logStream, std::format("feature level: {}.\n", featureLevelStrings[i]));
			break;
		}
	}

	assert(device != nullptr);
	Log(logStream, "Complete create D3D12 device!\n");

	// ===================make it crash for testing dump=================
	//uint32_t* p = nullptr;
	//*p = 100;

	// =========================Create window====================
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