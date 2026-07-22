#pragma once

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <format>
#include <vector>
#include <string>
#include <sstream>
#include <wrl.h>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <cassert>
#include <dbghelp.h>
#include <strsafe.h>
#include <dxgidebug.h>
#include <dxcapi.h>

#include "ConvertString.hpp"
#include "Matrix4x4.hpp"
#include "DebugCamera.h"
#include "WindowManager.h"
#include "EngineHelper.h"
#include "Audio.h"
#include "InputSystem.h"
#include "../../externals/imgui/imgui.h"
#include "../../externals/imgui/imgui_impl_win32.h"
#include "../../externals/imgui/imgui_impl_dx12.h"
#include "../../externals/DirectXTex/DirectXTex.h"
#include "../../externals/DirectXTex/d3dx12.h"

static LONG WINAPI ExportDump(EXCEPTION_POINTERS* exception);

class EngineCommon {
public:
	static EngineCommon& GetInstance() {
		static EngineCommon instance;
		return instance;
	}

	std::ofstream logStream;
	// ========================= Structs ========================
	struct D3DresourceLeakChecker {
		~D3DresourceLeakChecker() {
			Microsoft::WRL::ComPtr<IDXGIDebug1> debug;
			if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debug)))) {
				debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
				debug->ReportLiveObjects(DXGI_DEBUG_APP, DXGI_DEBUG_RLO_ALL);
				debug->ReportLiveObjects(DXGI_DEBUG_D3D12, DXGI_DEBUG_RLO_ALL);
			}
		}
	};

	// ================= Global Variable Declarations =================
	//extern std::ofstream logStream;
	

	// ================= Function Declarations =================
	void CreateDefaultPSO();
	void Initialize(int32_t Width, int32_t Height);

	void PreDraw();
	void PostDraw();
	void Finalize();

	void Log(std::ostream& os, const std::string& message);
	void Log(std::ostream& os, const std::wstring& message);

	IDxcBlob* CompileShader(const std::wstring& filePath,const wchar_t* profile,IDxcUtils* dxcUtils,IDxcCompiler3* dxcCompiler,IDxcIncludeHandler* includeHandler);

	Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(ID3D12Device* device, size_t sizeInBytes);

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type, UINT numDescriptors, bool shaderVisible);

	DirectX::ScratchImage LoadTexture(const std::string& filePath);

	Microsoft::WRL::ComPtr<ID3D12Resource> CreateTextureResource(ID3D12Device* device, const DirectX::TexMetadata& metadata);

	[[nodiscard]]
	Microsoft::WRL::ComPtr<ID3D12Resource> UploadTextureData(ID3D12Resource* texture, const DirectX::ScratchImage& mipImages,
		ID3D12Device* device, ID3D12GraphicsCommandList* commandList);

	Microsoft::WRL::ComPtr<ID3D12Resource> CreateDepthStencilTextureResource(ID3D12Device* device, int32_t width, int32_t height);

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(ID3D12DescriptorHeap* descriptorHeap, uint32_t descriptorSize, uint32_t index);

	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(ID3D12DescriptorHeap* descriptorHeap, uint32_t descriptorSize, uint32_t index);

	//void TempMainFunction();

	// ================= Audio Function Declarations =================
	bool LoadAudio(const std::string& soundName, const std::string& filePath) { return audioSystem_.LoadAudio(soundName, filePath); }
	void SoundUnload(const std::string& soundName) { audioSystem_.UnloadSound(soundName); }
	void PlayAudio(const std::string& soundName) { audioSystem_.PlayAudio(soundName); }

	// ================== getters ==================
	InputSystem& GetInputSystem() { return inputSystem_; }
	AudioSystem& GetAudioSystem() { return audioSystem_; }

	ID3D12Device* GetDevice() { return device.Get(); }
	ID3D12GraphicsCommandList* GetCommandList() { return commandList.Get(); }
	ID3D12DescriptorHeap* GetSRVDescriptorHeap() { return srvDescriptorHeap.Get(); }
	uint32_t GetDescriptorSizeSRV() { return descriptorSizeSRV; }
	ID3D12RootSignature* GetRootSignature() { return rootSignature.Get(); }
	ID3D12Resource* GetDirectionalLightResource() { return directionalLightResource.Get(); }
	ID3D12PipelineState* GetPipelineState() { return pipelineState.Get(); }

	private:
	InputSystem inputSystem_;
	AudioSystem audioSystem_;

	D3DresourceLeakChecker leakChecker;
	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory;
	Microsoft::WRL::ComPtr<IDXGIAdapter4> useAdapter;
	Microsoft::WRL::ComPtr<ID3D12Device> device;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList;
	Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain;
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc;
	WindowManager windowManager;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvDescriptorHeap;

	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState;
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;

	uint32_t descriptorSizeSRV;
	uint32_t descriptorSizeRTV;
	uint32_t descriptorSizeDSV;

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2];
	Microsoft::WRL::ComPtr<ID3D12Resource> swapChainTargets[2];

	Microsoft::WRL::ComPtr<ID3D12Fence> fence;
	uint64_t fenceValue;
	HANDLE fenceEvent;

	IDxcBlob* vertexShaderBlob;
	IDxcBlob* pixelShaderBlob;

	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvHeap;
	Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilTexture;

	ID3DBlob* signatureBlob;
	ID3DBlob* errorBlob;

	D3D12_VIEWPORT viewport;
	D3D12_RECT scissorRect;

	uint32_t clientWidth;
	uint32_t clientHeight;

	D3D12_RESOURCE_BARRIER barrier;
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource;

};

class Input {
public:
	static bool IsKeyPressed(int key) {
		return EngineCommon::GetInstance().GetInputSystem().IsKeyPressed(key);
	}

	static bool IsKeyHeld(int key) {
		return EngineCommon::GetInstance().GetInputSystem().IsKeyHeld(key);
	}

	static bool IsKeyReleased(int key) {
		return EngineCommon::GetInstance().GetInputSystem().IsKeyReleased(key);
	}
	
	static void Update() {
		EngineCommon::GetInstance().GetInputSystem().Update();
	}
};