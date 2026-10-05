#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <vector>
#include <cstdint>

#include <cstdint>

struct DescriptorHandle {
    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle{ 0 };
    D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle{ 0 };
    uint32_t index = UINT32_MAX;

    bool IsValid() const { return cpuHandle.ptr != 0; }
};

class DescriptorAllocator {
public:
    DescriptorAllocator() = default;
    ~DescriptorAllocator() = default;

    void Initialize(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t capacity, bool shaderVisible);

    DescriptorHandle Allocate();
    void Free(DescriptorHandle& handle);

    ID3D12DescriptorHeap* GetHeap() const { return heap_.Get(); }

private:
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> heap_;
    D3D12_DESCRIPTOR_HEAP_TYPE type_ = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

    uint32_t descriptorSize_ = 0;
    uint32_t capacity_ = 0;
    uint32_t currentOffset_ = 0;
    bool isShaderVisible_ = false;

    // Stores index slots that were freed and are ready to be reused
    std::vector<uint32_t> freeIndices_;
};