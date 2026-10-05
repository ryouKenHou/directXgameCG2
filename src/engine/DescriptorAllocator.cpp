#include "DescriptorAllocator.h"
#include <cassert>

void DescriptorAllocator::Initialize(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t capacity, bool shaderVisible) {
    type_ = type;
    capacity_ = capacity;
    isShaderVisible_ = shaderVisible;

    D3D12_DESCRIPTOR_HEAP_DESC desc{};
    desc.Type = type;
    desc.NumDescriptors = capacity;
    desc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    HRESULT hr = device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&heap_));
    assert(SUCCEEDED(hr));

    descriptorSize_ = device->GetDescriptorHandleIncrementSize(type);
}

DescriptorHandle DescriptorAllocator::Allocate() {
    uint32_t index = UINT32_MAX;

    // 1. Reuse a freed index if available
    if (!freeIndices_.empty()) {
        index = freeIndices_.back();
        freeIndices_.pop_back();
    }
    // 2. Otherwise, allocate the next contiguous slot
    else if (currentOffset_ < capacity_) {
        index = currentOffset_++;
    }
    else {
        assert(false && "Descriptor Heap ran out of memory!");
        return {};
    }

    // 3. Compute CPU & GPU handles
    DescriptorHandle handle;
    handle.index = index;

    handle.cpuHandle = heap_->GetCPUDescriptorHandleForHeapStart();
    handle.cpuHandle.ptr += static_cast<SIZE_T>(index) * descriptorSize_;

    if (isShaderVisible_) {
        handle.gpuHandle = heap_->GetGPUDescriptorHandleForHeapStart();
        handle.gpuHandle.ptr += static_cast<SIZE_T>(index) * descriptorSize_;
    }

    return handle;
}

void DescriptorAllocator::Free(DescriptorHandle& handle) {
    if (!handle.IsValid()) return;

    // Push index back to free list for future allocations
    freeIndices_.push_back(handle.index);

    // Invalidate handle
    handle = {};
}