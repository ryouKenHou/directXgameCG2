#pragma once

#include "EngineCommon.h"



class Model {
private:
	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource;	
	VertexData* vertexData = nullptr;
	Material* materialData = nullptr;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource;
	ModelData modelData;
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU2;
	Microsoft::WRL::ComPtr<ID3D12Resource> textureResource2;
	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource2;

	MaterialData LoadMaterialTemplayeFile(const std::string& directoryPath, const std::string& filename);

	ModelData LoadObjFile(const std::string& directoryPath, const std::string& filename);
public:
	TransformationMatrix* wvpData = nullptr;

	void LoadModel(const std::string& directoryPath, const std::string& filename);

	void Draw();
};