#include "TextureManager.h"

DescriptorHandle TextureManager::Load(const std::string& filePath, ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, DescriptorAllocator& srvAllocator) {
	auto it = textures_.find(filePath);
	if (it != textures_.end()) {
		return it->second.srvHandle;
	}

	DirectX::ScratchImage mipImages2 = LoadTexture(filePath);
	const DirectX::TexMetadata& metadata2 = mipImages2.GetMetadata();

	textures_[filePath] = TextureData{};
	textures_[filePath].resource = CreateTextureResource(device, metadata2);
	textures_[filePath].intermediateResource = UploadTextureData(textures_[filePath].resource.Get(), mipImages2, device, cmdList);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc2{};
	srvDesc2.Format = metadata2.format;
	srvDesc2.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc2.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc2.Texture2D.MipLevels = UINT(metadata2.mipLevels);

	textures_[filePath].srvHandle = srvAllocator.Allocate();
	device->CreateShaderResourceView(textures_[filePath].resource.Get(), &srvDesc2, textures_[filePath].srvHandle.cpuHandle);
	return textures_[filePath].srvHandle;
}