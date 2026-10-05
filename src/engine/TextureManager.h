#pragma once
#include <string>
#include <unordered_map>
#include <d3d12.h>
#include <wrl.h>
#include "DescriptorAllocator.h" // Assume we have this from earlier
#include "../../externals/DirectXTex/DirectXTex.h"
#include "../../externals/DirectXTex/d3dx12.h"
#include "ConvertString.hpp"
#include "EngineHelper.h"


struct TextureData {
    Microsoft::WRL::ComPtr<ID3D12Resource> resource; 
	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource;
    DescriptorHandle srvHandle; // Holds the CPU/GPU handles and index
};

class TextureManager {
public:
    static TextureManager& GetInstance() {
        static TextureManager instance;
        return instance;
    }

    // Loads a texture, or returns the existing handle if already loaded
    DescriptorHandle Load(const std::string& filePath, ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, DescriptorAllocator& srvAllocator);

	DirectX::ScratchImage LoadTexture(const std::string& filePath) {
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

	Microsoft::WRL::ComPtr<ID3D12Resource> CreateTextureResource(ID3D12Device* device, const DirectX::TexMetadata& metadata) {
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
	Microsoft::WRL::ComPtr<ID3D12Resource> UploadTextureData(ID3D12Resource* texture, const DirectX::ScratchImage& mipImages,
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

private:
    // Caches loaded textures using their file path as the key
    std::unordered_map<std::string, TextureData> textures_;
};