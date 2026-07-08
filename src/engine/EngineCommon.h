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
#include <xaudio2.h>

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

#include "ConvertString.hpp"
#include "Matrix4x4.hpp"
#include "DebugCamera.h"
#include "WindowManager.h"
#include "EngineHelper.h"
#include "../../externals/imgui/imgui.h"
#include "../../externals/imgui/imgui_impl_win32.h"
#include "../../externals/imgui/imgui_impl_dx12.h"
#include "../../externals/DirectXTex/DirectXTex.h"
#include "../../externals/DirectXTex/d3dx12.h"

namespace EngineCommon {

	// ========================= Structs ========================
	struct ChunkHeader {
		char id[4];
		int32_t size;
	};

	struct RiffHeader {
		ChunkHeader chunk;
		char type[4];
	};

	struct FormatChunk {
		ChunkHeader chunk;
		WAVEFORMATEX fmt;
	};

	struct SoundData {
		WAVEFORMATEX wfex;
		BYTE* pBuffer;
		unsigned int bufferSize;
	};

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
	extern D3DresourceLeakChecker leakChecker;
	extern Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory;
	extern Microsoft::WRL::ComPtr<IDXGIAdapter4> useAdapter;
	extern Microsoft::WRL::ComPtr<ID3D12Device> device;
	extern Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue;
	extern Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator;
	extern Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList;
	extern Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain;
	extern DXGI_SWAP_CHAIN_DESC1 swapChainDesc;
	extern WindowManager windowManager;
	extern Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvHeap;
	extern Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvDescriptorHeap;

	extern Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState;
	extern D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;

	extern uint32_t descriptorSizeSRV;
	extern uint32_t descriptorSizeRTV;
	extern uint32_t descriptorSizeDSV;

	extern D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2];
	extern Microsoft::WRL::ComPtr<ID3D12Resource> swapChainTargets[2];

	extern Microsoft::WRL::ComPtr<ID3D12Fence> fence;
	extern uint64_t fenceValue;
	extern HANDLE fenceEvent;

	extern IDxcBlob* vertexShaderBlob;
	extern IDxcBlob* pixelShaderBlob;

	extern Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
	extern Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvHeap;
	extern Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilTexture;

	extern ID3DBlob* signatureBlob;
	extern ID3DBlob* errorBlob;

	extern D3D12_VIEWPORT viewport;
	extern D3D12_RECT scissorRect;

	extern Microsoft::WRL::ComPtr<IXAudio2> xAudio2;
	extern IDirectInput8* directInput;
	extern IDirectInputDevice8* keyboard;

	extern SoundData soundData1;

	extern uint32_t clientWidth;
	extern uint32_t clientHeight;

	extern D3D12_RESOURCE_BARRIER barrier;
	extern Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource;

	// ================= Function Declarations =================
	SoundData SoundLoadWave(const char* filename);
	void SoundUnload(SoundData* soundData);
	void SoundPlayWave(IXAudio2* xAudio2, const SoundData& soundData);

	void CreateDefaultPSO();
	void Initialize(int32_t Width, int32_t Height);

	void PreDraw();
	void PostDraw();
	void Finalize();

	void Log(std::ostream& os, const std::string& message);

	void Log(std::ostream& os, const std::wstring& message);

	static LONG WINAPI ExportDump(EXCEPTION_POINTERS* exception);

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
}