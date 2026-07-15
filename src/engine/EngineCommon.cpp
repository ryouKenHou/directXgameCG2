#include "EngineCommon.h"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dbghelp.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "dxcompiler.lib")
#pragma comment(lib, "xaudio2.lib")
#pragma comment(lib, "dinput8.lib")

 


    // ================= Function Definitions =================

    SoundData EngineCommon::SoundLoadWave(const char* filename) {
        std::ifstream file;
        file.open(filename, std::ios_base::binary);
        assert(file.is_open());

        RiffHeader riff;
        file.read((char*)&riff, sizeof(riff));
        if (strncmp(riff.chunk.id, "RIFF", 4) != 0) { assert(0); }
        if (strncmp(riff.type, "WAVE", 4) != 0) { assert(0); }

        FormatChunk format = {};
        file.read((char*)&format, sizeof(ChunkHeader));
        while (strncmp(format.chunk.id, "fmt ", 4) != 0) {
            file.seekg(format.chunk.size, std::ios_base::cur);
            file.read((char*)&format, sizeof(ChunkHeader));
            assert(!file.eof());
        }

        assert(format.chunk.size <= sizeof(format.fmt));
        file.read((char*)&format.fmt, format.chunk.size);

        ChunkHeader data;
        file.read((char*)&data, sizeof(data));
        if (strncmp(data.id, "JUNK", 4) == 0) {
            file.seekg(data.size, std::ios_base::cur);
            file.read((char*)&data, sizeof(data));
        }

        if (strncmp(data.id, "data", 4) != 0) { assert(0); }

        char* pBuffer = new char[data.size];
        file.read(pBuffer, data.size);
        file.close();

        SoundData soundData = {};
        soundData.wfex = format.fmt;
        soundData.pBuffer = reinterpret_cast<BYTE*>(pBuffer);
        soundData.bufferSize = data.size;

        return soundData;
    }

    void EngineCommon::SoundUnload(SoundData* soundData) {
        delete[] soundData->pBuffer;
        soundData->pBuffer = 0;
        soundData->bufferSize = 0;
        soundData->wfex = {};
    }

    void EngineCommon::SoundPlayWave(IXAudio2* xAudio2, const SoundData& soundData) {
        HRESULT result;
        IXAudio2SourceVoice* pSourceVoice = nullptr;
        result = xAudio2->CreateSourceVoice(&pSourceVoice, &soundData.wfex);
        assert(SUCCEEDED(result));

        XAUDIO2_BUFFER buf{};
        buf.pAudioData = soundData.pBuffer;
        buf.AudioBytes = soundData.bufferSize;
        buf.Flags = XAUDIO2_END_OF_STREAM;

        result = pSourceVoice->SubmitSourceBuffer(&buf);
        result = pSourceVoice->Start();
    }

    void  EngineCommon::CreateDefaultPSO() {
        IDxcUtils* dxcUtils = nullptr;
        IDxcCompiler3* dxcCompiler = nullptr;
        HREFTYPE hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils));
        assert(SUCCEEDED(hr));
        hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler));
        assert(SUCCEEDED(hr));

        IDxcIncludeHandler* includeHandler = nullptr;
        hr = dxcUtils->CreateDefaultIncludeHandler(&includeHandler);
        assert(SUCCEEDED(hr));

        D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
        descriptorRange[0].BaseShaderRegister = 0;
        descriptorRange[0].NumDescriptors = 1;
        descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
        descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

        D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
        descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

        D3D12_ROOT_PARAMETER rootParameters[4] = {};
        rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
        rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
        rootParameters[0].Descriptor.ShaderRegister = 0;
        rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
        rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
        rootParameters[1].Descriptor.ShaderRegister = 0;
        rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
        rootParameters[2].DescriptorTable.pDescriptorRanges = descriptorRange;
        rootParameters[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);
        rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
        rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
        rootParameters[3].Descriptor.ShaderRegister = 1;
        descriptionRootSignature.pParameters = rootParameters;
        descriptionRootSignature.NumParameters = _countof(rootParameters);

        D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
        staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
        staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
        staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;
        staticSamplers[0].ShaderRegister = 0;
        staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
        descriptionRootSignature.pStaticSamplers = staticSamplers;
        descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);

        hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
        if (FAILED(hr)) {
            Log(logStream, reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
            assert(false);
        }

        hr = device->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
        assert(SUCCEEDED(hr));

        D3D12_INPUT_ELEMENT_DESC inputElementDescs[3] = {};
        inputElementDescs[0].SemanticName = "POSITION";
        inputElementDescs[0].SemanticIndex = 0;
        inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
        inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
        inputElementDescs[1].SemanticName = "TEXCOORD";
        inputElementDescs[1].SemanticIndex = 0;
        inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
        inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
        inputElementDescs[2].SemanticName = "NORMAL";
        inputElementDescs[2].SemanticIndex = 0;
        inputElementDescs[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
        inputElementDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
        D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
        inputLayoutDesc.pInputElementDescs = inputElementDescs;
        inputLayoutDesc.NumElements = _countof(inputElementDescs);

        D3D12_BLEND_DESC blendDesc{};
        blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

        D3D12_RASTERIZER_DESC rasterizerDesc{};
        rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
        rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

        vertexShaderBlob = CompileShader(L"shader/Object3d.VS.hlsl", L"vs_6_0", dxcUtils, dxcCompiler, includeHandler);
        assert(vertexShaderBlob != nullptr);

        pixelShaderBlob = CompileShader(L"shader/Object3d.PS.hlsl", L"ps_6_0", dxcUtils, dxcCompiler, includeHandler);
        assert(pixelShaderBlob != nullptr);

        depthStencilTexture = CreateDepthStencilTextureResource(device.Get(), clientWidth, clientHeight);
        dsvHeap = CreateDescriptorHeap(device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);

        D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
        dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
        device->CreateDepthStencilView(depthStencilTexture.Get(), &dsvDesc, dsvHeap->GetCPUDescriptorHandleForHeapStart());

        D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
        depthStencilDesc.DepthEnable = TRUE;
        depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
        depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

        D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
        graphicsPipelineStateDesc.pRootSignature = rootSignature.Get();
        graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;
        graphicsPipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize() };
        graphicsPipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize() };
        graphicsPipelineStateDesc.BlendState = blendDesc;
        graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;
        graphicsPipelineStateDesc.NumRenderTargets = 1;
        graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
        graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        graphicsPipelineStateDesc.SampleDesc.Count = 1;
        graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
        graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc;
        graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

        hr = device->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&pipelineState));
        assert(SUCCEEDED(hr));
    }

    void EngineCommon::Initialize(int32_t Width, int32_t Height) {
        clientHeight = Height;
        clientWidth = Width;

        SetUnhandledExceptionFilter(ExportDump);
        CoInitializeEx(0, COINIT_MULTITHREADED);

        std::filesystem::create_directories("logs");

        std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
        std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds> nowSeconds = std::chrono::time_point_cast<std::chrono::seconds>(now);
        std::chrono::zoned_time localTime{ std::chrono::current_zone(), nowSeconds };
        std::string dateString = std::format("{:%Y%m%d_%H%M%S}", localTime);
        std::string logFilePath = std::string("logs/") + dateString + ".log";
        logStream.open(logFilePath);

        Log(logStream, "Hello, DirectX!");

        HREFTYPE hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));
        assert(SUCCEEDED(hr));

#ifdef _DEBUG
        Microsoft::WRL::ComPtr<ID3D12Debug1> debugController;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
            debugController->EnableDebugLayer();
            debugController->SetEnableGPUBasedValidation(TRUE);
        }
#endif

        for (UINT i = 0; dxgiFactory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&useAdapter)) != DXGI_ERROR_NOT_FOUND; ++i) {
            DXGI_ADAPTER_DESC3 adapterDesc{};
            hr = useAdapter->GetDesc3(&adapterDesc);
            assert(SUCCEEDED(hr));

            if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)) {
                Log(logStream, std::format("Use adapter: {}\n", ConvertString(std::wstring(adapterDesc.Description))));
                break;
            }
            useAdapter = nullptr;
        }
        assert(useAdapter != nullptr);

        D3D_FEATURE_LEVEL featureLevels[] = {
            D3D_FEATURE_LEVEL_12_2,
            D3D_FEATURE_LEVEL_12_1,
            D3D_FEATURE_LEVEL_12_0,
        };

        const char* featureLevelStrings[] = { "12.2", "12.1", "12.0" };

        for (size_t i = 0; i < std::size(featureLevels); ++i) {
            hr = D3D12CreateDevice(useAdapter.Get(), featureLevels[i], IID_PPV_ARGS(&device));
            if (SUCCEEDED(hr)) {
                Log(logStream, std::format("feature level: {}.\n", featureLevelStrings[i]));
                break;
            }
        }
        assert(device != nullptr);
        Log(logStream, "Complete create D3D12 device!\n");

#ifdef _DEBUG
        ID3D12InfoQueue* infoQueue = nullptr;
        if (SUCCEEDED(device->QueryInterface(IID_PPV_ARGS(&infoQueue)))) {
            infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
            infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
            infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);

            D3D12_MESSAGE_ID deniedIds[] = { D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE };
            D3D12_MESSAGE_SEVERITY severities[] = { D3D12_MESSAGE_SEVERITY_INFO };
            D3D12_INFO_QUEUE_FILTER filter{};
            filter.DenyList.NumIDs = _countof(deniedIds);
            filter.DenyList.pIDList = deniedIds;
            filter.DenyList.NumSeverities = _countof(severities);
            filter.DenyList.pSeverityList = severities;
            infoQueue->PushStorageFilter(&filter);
            infoQueue->Release();
        }
#endif

        windowManager.Initialize(SW_SHOW, clientWidth, clientHeight, L"CG2");

        D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
        hr = device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue));
        assert(SUCCEEDED(hr));

        hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
        assert(SUCCEEDED(hr));

        hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList));
        assert(SUCCEEDED(hr));

        assert(windowManager.getHwnd() != nullptr);
        assert(IsWindow(windowManager.getHwnd()));

        swapChainDesc.Width = clientWidth;
        swapChainDesc.Height = clientHeight;
        swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.BufferCount = 2;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        hr = dxgiFactory->CreateSwapChainForHwnd(
            commandQueue.Get(), windowManager.getHwnd(), &swapChainDesc, nullptr, nullptr, reinterpret_cast<IDXGISwapChain1**>(swapChain.GetAddressOf())
        );
        assert(SUCCEEDED(hr));

        rtvHeap = CreateDescriptorHeap(device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 2, false);
        srvDescriptorHeap = CreateDescriptorHeap(device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 128, true);

        descriptorSizeSRV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        descriptorSizeRTV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        descriptorSizeDSV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

        hr = swapChain->GetBuffer(0, IID_PPV_ARGS(&swapChainTargets[0]));
        assert(SUCCEEDED(hr));
        hr = swapChain->GetBuffer(1, IID_PPV_ARGS(&swapChainTargets[1]));
        assert(SUCCEEDED(hr));

        rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
        rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = GetCPUDescriptorHandle(rtvHeap.Get(), descriptorSizeRTV, 0);
        rtvHandles[0] = rtvHandle;
        device->CreateRenderTargetView(swapChainTargets[0].Get(), &rtvDesc, rtvHandles[0]);
        rtvHandles[1] = GetCPUDescriptorHandle(rtvHeap.Get(), descriptorSizeRTV, 1);
        device->CreateRenderTargetView(swapChainTargets[1].Get(), &rtvDesc, rtvHandles[1]);

        hr = device->CreateFence(fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
        assert(SUCCEEDED(hr));

        fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        assert(fenceEvent != nullptr);

        CreateDefaultPSO();

        IXAudio2MasteringVoice* masterVoice;
        hr = XAudio2Create(&xAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);
        assert(SUCCEEDED(hr));
        hr = xAudio2->CreateMasteringVoice(&masterVoice);
        assert(SUCCEEDED(hr));

        soundData1 = SoundLoadWave("resources/Alarm01.wav");

        hr = DirectInput8Create(windowManager.getWindowClass().hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&directInput, nullptr);
        assert(SUCCEEDED(hr));

        hr = directInput->CreateDevice(GUID_SysKeyboard, &keyboard, NULL);
        assert(SUCCEEDED(hr));

        hr = keyboard->SetDataFormat(&c_dfDIKeyboard);
        assert(SUCCEEDED(hr));

        hr = keyboard->SetCooperativeLevel(windowManager.getHwnd(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
        assert(SUCCEEDED(hr));

        viewport.Width = static_cast<float>(clientWidth);
        viewport.Height = static_cast<float>(clientHeight);
        viewport.TopLeftX = 0.0f;
        viewport.TopLeftY = 0.0f;
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;

        scissorRect.left = 0;
        scissorRect.top = 0;
        scissorRect.right = static_cast<LONG>(clientWidth);
        scissorRect.bottom = static_cast<LONG>(clientHeight);

#ifdef _DEBUG
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::StyleColorsDark();
        ImGui_ImplWin32_Init(windowManager.getHwnd());
        ImGui_ImplDX12_Init(device.Get(), swapChainDesc.BufferCount, rtvDesc.Format, srvDescriptorHeap.Get(), srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
        ImGuiIO& io = ImGui::GetIO();
        io.Fonts->Build();
#endif

        directionalLightResource = CreateBufferResource(device.Get(), sizeof(DirectionalLight));
        DirectionalLight* directionalLightData = nullptr;
        directionalLightResource->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData));
        directionalLightData->color = { 1.0f, 1.0f, 1.0f ,1.0f };
        directionalLightData->direction = { 0.0f, -1.0f, 0.0f };
        directionalLightData->intensity = 1.0f;
    }

    void EngineCommon::PreDraw() {
#ifdef _DEBUG
        ImGui_ImplDX12_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
#endif
        UINT backBufferIndex = swapChain->GetCurrentBackBufferIndex();

        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource = swapChainTargets[backBufferIndex].Get();
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
        commandList->ResourceBarrier(1, &barrier);

        commandList->OMSetRenderTargets(1, &rtvHandles[backBufferIndex], FALSE, nullptr);
        float clearColor[] = { 0.1f, 0.25f, 0.5f, 1.0f };
        commandList->ClearRenderTargetView(rtvHandles[backBufferIndex], clearColor, 0, nullptr);

        ID3D12DescriptorHeap* descriptorHeaps[] = { srvDescriptorHeap.Get() };
        commandList->SetDescriptorHeaps(1, descriptorHeaps);

        D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dsvHeap->GetCPUDescriptorHandleForHeapStart();
        commandList->OMSetRenderTargets(1, &rtvHandles[backBufferIndex], FALSE, &dsvHandle);
        commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

        commandList->RSSetViewports(1, &viewport);
        commandList->RSSetScissorRects(1, &scissorRect);
    }

    void EngineCommon::PostDraw() {
#ifdef _DEBUG
        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList.Get());
#endif
        UINT backBufferIndex = swapChain->GetCurrentBackBufferIndex();
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource = swapChainTargets[backBufferIndex].Get();
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
        commandList->ResourceBarrier(1, &barrier);

        HRESULT hr = commandList->Close();
        assert(SUCCEEDED(hr));

        ID3D12CommandList* commandLists[] = { commandList.Get() };
        commandQueue->ExecuteCommandLists(1, commandLists);

        swapChain->Present(1, 0);
        fenceValue++;
        commandQueue->Signal(fence.Get(), fenceValue);
        if (fence->GetCompletedValue() < fenceValue) {
            fence->SetEventOnCompletion(fenceValue, fenceEvent);
            WaitForSingleObject(fenceEvent, INFINITE);
        }

        hr = commandAllocator->Reset();
        assert(SUCCEEDED(hr));
        hr = commandList->Reset(commandAllocator.Get(), nullptr);
        assert(SUCCEEDED(hr));
    }

    void EngineCommon::Finalize() {
        CloseHandle(fenceEvent);
        signatureBlob->Release();
        if (errorBlob != nullptr) {
            errorBlob->Release();
        }
        pixelShaderBlob->Release();
        vertexShaderBlob->Release();

#ifdef _DEBUG
        ImGui_ImplDX12_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
#endif

        windowManager.Finalize();
        device.Reset();
    }

    void EngineCommon::Log(std::ostream& os, const std::string& message) {
        os << message << std::endl;
        OutputDebugStringA((message + "\n").c_str());
    }

    void EngineCommon::Log(std::ostream& os, const std::wstring& message) {
        Log(os, ConvertString(message));
    }

    static LONG WINAPI ExportDump(EXCEPTION_POINTERS* exception) {
        SYSTEMTIME time;
        GetLocalTime(&time);
        wchar_t filePath[MAX_PATH] = { 0 };
        CreateDirectory(L"./dump", nullptr);
        StringCchPrintfW(filePath, MAX_PATH, L"./dump/%04d-%02d%02d-%02d%02d%02d.dmp",
            time.wYear, time.wMonth, time.wDay,
            time.wHour, time.wMinute, time.wSecond);
        HANDLE dumpFileHandle = CreateFile(filePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ, 0, CREATE_ALWAYS, 0, 0);
        DWORD processId = GetCurrentProcessId();
        DWORD threadId = GetCurrentThreadId();

        MINIDUMP_EXCEPTION_INFORMATION minidumpInformation{ 0 };
        minidumpInformation.ThreadId = threadId;
        minidumpInformation.ExceptionPointers = exception;
        minidumpInformation.ClientPointers = TRUE;

        MiniDumpWriteDump(GetCurrentProcess(), processId, dumpFileHandle, MiniDumpNormal, &minidumpInformation, nullptr, nullptr);

        return EXCEPTION_EXECUTE_HANDLER;
    }

    IDxcBlob* EngineCommon::CompileShader(
        const std::wstring& filePath,
        const wchar_t* profile,
        IDxcUtils* dxcUtils,
        IDxcCompiler3* dxcCompiler,
        IDxcIncludeHandler* includeHandler
    ) {
        // read file
        Log(logStream, ConvertString(std::format(L"Begin CompileShader: path:{}, profile:{}.\n", filePath, profile)));
        IDxcBlobEncoding* shaderSource = nullptr;
        HRESULT hr = dxcUtils->LoadFile(filePath.c_str(), nullptr, &shaderSource);
        assert(SUCCEEDED(hr));

        DxcBuffer shaderSourceBuffer;
        shaderSourceBuffer.Ptr = shaderSource->GetBufferPointer();
        shaderSourceBuffer.Size = shaderSource->GetBufferSize();
        shaderSourceBuffer.Encoding = DXC_CP_UTF8;

        // compile
        LPCWSTR arguments[] = {
            filePath.c_str(),
            L"-E", L"main",
            L"-T", profile,
            L"-Zi", L"-Qembed_debug",
            L"-Od",
            L"-Zpr",
        };

        IDxcResult* shaderResult = nullptr;
        hr = dxcCompiler->Compile(
            &shaderSourceBuffer,
            arguments,
            _countof(arguments),
            includeHandler,
            IID_PPV_ARGS(&shaderResult)
        );
        assert(SUCCEEDED(hr));

        // check error
        IDxcBlobUtf8* shaderError = nullptr;
        shaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&shaderError), nullptr);
        if (shaderError != nullptr && shaderError->GetStringLength() > 0) {
            Log(logStream, shaderError->GetStringPointer());
            assert(false);
        }

        // get result
        IDxcBlob* shaderBlob = nullptr;
        hr = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr);
        assert(SUCCEEDED(hr));

        Log(logStream, ConvertString(std::format(L"Compile Succeeded: path:{}, profile:{}.\n", filePath, profile)));
        shaderSource->Release();
        shaderResult->Release();
        return shaderBlob;
    }

    Microsoft::WRL::ComPtr<ID3D12Resource> EngineCommon::CreateBufferResource(ID3D12Device* device, size_t sizeInBytes) {
        D3D12_HEAP_PROPERTIES heapProperties{};
        heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
        D3D12_RESOURCE_DESC resourceDesc{};
        resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        resourceDesc.Width = sizeInBytes;
        resourceDesc.Height = 1;
        resourceDesc.DepthOrArraySize = 1;
        resourceDesc.MipLevels = 1;
        resourceDesc.SampleDesc.Count = 1;
        resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

        Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
        HRESULT hr = device->CreateCommittedResource(
            &heapProperties,
            D3D12_HEAP_FLAG_NONE,
            &resourceDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&resource)
        );
        assert(SUCCEEDED(hr));
        return resource;
    }

    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> EngineCommon::CreateDescriptorHeap(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type, UINT numDescriptors, bool shaderVisible) {
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap;
        D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
        descriptorHeapDesc.Type = type;
        descriptorHeapDesc.NumDescriptors = numDescriptors;
        descriptorHeapDesc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        HRESULT hr = device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&descriptorHeap));
        assert(SUCCEEDED(hr));
        return descriptorHeap;
    }

    DirectX::ScratchImage EngineCommon::LoadTexture(const std::string& filePath) {
        // texture loading
        DirectX::ScratchImage image{};
        std::wstring filePathW = ConvertString(filePath);
        HRESULT hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
        assert(SUCCEEDED(hr));

        // mipmap generation
        DirectX::ScratchImage mipImage{};
        hr = DirectX::GenerateMipMaps(
            image.GetImages(), image.GetImageCount(), image.GetMetadata(),
            DirectX::TEX_FILTER_SRGB, 0, mipImage
        );
        assert(SUCCEEDED(hr));

        return mipImage;

    }

    Microsoft::WRL::ComPtr<ID3D12Resource> EngineCommon::CreateTextureResource(ID3D12Device* device, const DirectX::TexMetadata& metadata) {
        // 1. set resource base on metadata
        D3D12_RESOURCE_DESC resourceDesc{};
        resourceDesc.Width = UINT(metadata.width);
        resourceDesc.Height = UINT(metadata.height);
        resourceDesc.MipLevels = UINT16(metadata.mipLevels);
        resourceDesc.DepthOrArraySize = UINT16(metadata.arraySize);
        resourceDesc.Format = metadata.format;
        resourceDesc.SampleDesc.Count = 1;
        resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION(metadata.dimension);

        // 2. set heap properties
        D3D12_HEAP_PROPERTIES heapProperties{};
        heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
        //heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
        //heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;

        // 3. create resource
        Microsoft::WRL::ComPtr<ID3D12Resource> resource;
        HRESULT hr = device->CreateCommittedResource(
            &heapProperties,
            D3D12_HEAP_FLAG_NONE,
            &resourceDesc,
            D3D12_RESOURCE_STATE_COPY_DEST,
            nullptr,
            IID_PPV_ARGS(&resource)
        );
        assert(SUCCEEDED(hr));
        return resource;
    }

    [[nodiscard]]
    Microsoft::WRL::ComPtr<ID3D12Resource> EngineCommon::UploadTextureData(ID3D12Resource* texture, const DirectX::ScratchImage& mipImages,
        ID3D12Device* device, ID3D12GraphicsCommandList* commandList) {
        std::vector<D3D12_SUBRESOURCE_DATA> subResources;
        DirectX::PrepareUpload(device, mipImages.GetImages(),
            mipImages.GetImageCount(), mipImages.GetMetadata(), subResources);
        uint64_t intermediateSize = GetRequiredIntermediateSize(texture, 0, UINT(subResources.size()));
        Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource = CreateBufferResource(device, intermediateSize);
        UpdateSubresources(commandList, texture, intermediateResource.Get(), 0, 0, UINT(subResources.size()), subResources.data());
        D3D12_RESOURCE_BARRIER barrier{};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource = texture;
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;
        commandList->ResourceBarrier(1, &barrier);
        return intermediateResource;

    }

    Microsoft::WRL::ComPtr<ID3D12Resource> EngineCommon::CreateDepthStencilTextureResource(ID3D12Device* device, int32_t width, int32_t height) {
        D3D12_RESOURCE_DESC resourceDesc{};
        resourceDesc.Width = width;
        resourceDesc.Height = height;
        resourceDesc.MipLevels = 1;
        resourceDesc.DepthOrArraySize = 1;
        resourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        resourceDesc.SampleDesc.Count = 1;
        resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

        D3D12_HEAP_PROPERTIES heapProperties{};
        heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

        D3D12_CLEAR_VALUE depthClearValue{};
        depthClearValue.DepthStencil.Depth = 1.0f;
        depthClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

        Microsoft::WRL::ComPtr<ID3D12Resource> resource;
        HRESULT hr = device->CreateCommittedResource(
            &heapProperties,
            D3D12_HEAP_FLAG_NONE,
            &resourceDesc,
            D3D12_RESOURCE_STATE_DEPTH_WRITE,
            &depthClearValue,
            IID_PPV_ARGS(&resource)
        );
        assert(SUCCEEDED(hr));
        return resource;
    }

    D3D12_CPU_DESCRIPTOR_HANDLE EngineCommon::GetCPUDescriptorHandle(ID3D12DescriptorHeap* descriptorHeap, uint32_t descriptorSize, uint32_t index) {
        D3D12_CPU_DESCRIPTOR_HANDLE handle = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
        handle.ptr += index * descriptorSize;
        return handle;
    }

    D3D12_GPU_DESCRIPTOR_HANDLE EngineCommon::GetGPUDescriptorHandle(ID3D12DescriptorHeap* descriptorHeap, uint32_t descriptorSize, uint32_t index) {
        D3D12_GPU_DESCRIPTOR_HANDLE handle = descriptorHeap->GetGPUDescriptorHandleForHeapStart();
        handle.ptr += index * descriptorSize;
        return handle;
    }

    //void TempMainFunction() {
    //    Microsoft::WRL::ComPtr<ID3D12Resource> vertexResourceSprite = CreateBufferResource(device.Get(), sizeof(VertexData) * 4);
    //    D3D12_VERTEX_BUFFER_VIEW vertexBufferViewSprite{};
    //    vertexBufferViewSprite.BufferLocation = vertexResourceSprite->GetGPUVirtualAddress();
    //    vertexBufferViewSprite.SizeInBytes = sizeof(VertexData) * 4;
    //    vertexBufferViewSprite.StrideInBytes = sizeof(VertexData);

    //    VertexData* vertexDataSprite = nullptr;
    //    vertexResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataSprite));

    //    vertexDataSprite[0].position = { 0.0f, 360.f, 0.0f, 1.0f };
    //    vertexDataSprite[0].texcoord = { 0.0f, 1.0f };
    //    vertexDataSprite[0].normal = { 0.0f, 0.0f, -1.0f };

    //    vertexDataSprite[1].position = { 0.0f, 0.0f, 0.0f, 1.0f };
    //    vertexDataSprite[1].texcoord = { 0.0f, 0.0f };
    //    vertexDataSprite[1].normal = { 0.0f, 0.0f, -1.0f };

    //    vertexDataSprite[2].position = { 640.f, 360.f, 0.0f, 1.0f };
    //    vertexDataSprite[2].texcoord = { 1.0f, 1.0f };
    //    vertexDataSprite[2].normal = { 0.0f, 0.0f, -1.0f };

    //    vertexDataSprite[3].position = { 640.f, 0.0f, 0.0f, 1.0f };
    //    vertexDataSprite[3].texcoord = { 1.0f, 0.0f };
    //    vertexDataSprite[3].normal = { 0.0f, 0.0f, -1.0f };

    //    Microsoft::WRL::ComPtr<ID3D12Resource> indexResourceSprite = CreateBufferResource(device.Get(), sizeof(uint32_t) * 6);
    //    D3D12_INDEX_BUFFER_VIEW indexBufferViewSprite{};
    //    indexBufferViewSprite.BufferLocation = indexResourceSprite->GetGPUVirtualAddress();
    //    indexBufferViewSprite.SizeInBytes = sizeof(uint32_t) * 6;
    //    indexBufferViewSprite.Format = DXGI_FORMAT_R32_UINT;

    //    uint32_t* indexDataSprite = nullptr;
    //    indexResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&indexDataSprite));
    //    indexDataSprite[0] = 0; indexDataSprite[1] = 1; indexDataSprite[2] = 2;
    //    indexDataSprite[3] = 1; indexDataSprite[4] = 3; indexDataSprite[5] = 2;

    //    Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResourceSprite = CreateBufferResource(device.Get(), sizeof(TransformationMatrix));
    //    TransformationMatrix* transformationMatrixDataSprite = nullptr;
    //    transformationMatrixResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixDataSprite));
    //    *transformationMatrixDataSprite = { Matrix4x4::Identity(), Matrix4x4::Identity() };

    //    Microsoft::WRL::ComPtr<ID3D12Resource> materialResourceSprite = CreateBufferResource(device.Get(), sizeof(Material));
    //    Material* materialDataSprite = nullptr;
    //    materialResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&materialDataSprite));
    //    materialDataSprite->color = { 1.0f, 1.0f, 1.0f, 1.0f };
    //    materialDataSprite->enableLighting = false;
    //    materialDataSprite->uvTransform = Matrix4x4::Identity();

    //    DirectX::ScratchImage mipImages = LoadTexture("resources/uvChecker.png");
    //    const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
    //    Microsoft::WRL::ComPtr<ID3D12Resource> textureResource = CreateTextureResource(device.Get(), metadata);
    //    Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource = UploadTextureData(textureResource.Get(), mipImages, device.Get(), commandList.Get());

    //    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    //    srvDesc.Format = metadata.format;
    //    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    //    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    //    srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);

    //    D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU = GetCPUDescriptorHandle(srvDescriptorHeap.Get(), descriptorSizeSRV, 1);
    //    D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU = GetGPUDescriptorHandle(srvDescriptorHeap.Get(), descriptorSizeSRV, 1);
    //    device->CreateShaderResourceView(textureResource.Get(), &srvDesc, textureSrvHandleCPU);

    //    MSG msg{};
    //    while (msg.message != WM_QUIT) {
    //        if (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
    //            TranslateMessage(&msg);
    //            DispatchMessageW(&msg);
    //        }
    //        else {
    //            commandList->IASetIndexBuffer(&indexBufferViewSprite);
    //            commandList->IASetVertexBuffers(0, 1, &vertexBufferViewSprite);
    //            commandList->SetGraphicsRootConstantBufferView(0, materialResourceSprite->GetGPUVirtualAddress());
    //            commandList->SetGraphicsRootConstantBufferView(1, transformationMatrixResourceSprite->GetGPUVirtualAddress());
    //            commandList->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU);
    //            commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);
    //        }
    //    }
    //}