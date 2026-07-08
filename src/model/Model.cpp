#include "Model.h"

void Model::LoadModel() {
	modelData = LoadObjFile("resources/05_02", "axis.obj");

	// == vertex resource for model ==
	vertexResource = EngineCommon::CreateBufferResource(EngineCommon::device.Get(), sizeof(VertexData) * modelData.vertices.size());

	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	vertexBufferView.SizeInBytes = UINT(sizeof(VertexData) * modelData.vertices.size());
	vertexBufferView.StrideInBytes = sizeof(VertexData);

	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	std::memcpy(vertexData, modelData.vertices.data(), sizeof(*vertexData) * modelData.vertices.size());

	// material resource
	materialResource = EngineCommon::CreateBufferResource(EngineCommon::device.Get(), sizeof(Material));

	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	*materialData = { {1.0f, 1.0f, 1.0f, 1.0f}, 1 };
	materialData->uvTransform = Matrix4x4::Identity();

	// WVP resource
	wvpResource = EngineCommon::CreateBufferResource(EngineCommon::device.Get(), sizeof(TransformationMatrix));

	wvpResource->Map(0, nullptr, reinterpret_cast<void**>(&wvpData));
	*wvpData = { Matrix4x4::Identity(), Matrix4x4::Identity() };

	//load sencond texture for sprite
	DirectX::ScratchImage mipImages2 = EngineCommon::LoadTexture(modelData.material.textureFilePath);
	const DirectX::TexMetadata& metadata2 = mipImages2.GetMetadata();
	textureResource2 = EngineCommon::CreateTextureResource(EngineCommon::device.Get(), metadata2);
	intermediateResource2 = EngineCommon::UploadTextureData(textureResource2.Get(), mipImages2, EngineCommon::device.Get(), EngineCommon::commandList.Get());

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc2{};
	srvDesc2.Format = metadata2.format;
	srvDesc2.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc2.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc2.Texture2D.MipLevels = UINT(metadata2.mipLevels);

	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU2 = EngineCommon::GetCPUDescriptorHandle(EngineCommon::srvDescriptorHeap.Get(), EngineCommon::descriptorSizeSRV, 2);
	textureSrvHandleGPU2 = EngineCommon::GetGPUDescriptorHandle(EngineCommon::srvDescriptorHeap.Get(), EngineCommon::descriptorSizeSRV, 2);
	EngineCommon::device->CreateShaderResourceView(textureResource2.Get(), &srvDesc2, textureSrvHandleCPU2);

}

void Model::Draw() {
	EngineCommon::commandList->SetGraphicsRootSignature(EngineCommon::rootSignature.Get());
	EngineCommon::commandList->SetPipelineState(EngineCommon::pipelineState.Get());
	EngineCommon::commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
	EngineCommon::commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	EngineCommon::commandList->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
	EngineCommon::commandList->SetGraphicsRootConstantBufferView(1, wvpResource->GetGPUVirtualAddress());
	EngineCommon::commandList->SetGraphicsRootConstantBufferView(3, EngineCommon::directionalLightResource->GetGPUVirtualAddress());
	EngineCommon::commandList->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU2);
	EngineCommon::commandList->DrawInstanced(UINT(modelData.vertices.size()), 1, 0, 0);
}

MaterialData Model::LoadMaterialTemplayeFile(const std::string& directoryPath, const std::string& filename) {
	MaterialData materialData;
	std::string line;
	std::ifstream file(directoryPath + "/" + filename);
	assert(file.is_open());

	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;

		if (identifier == "map_Kd") {
			std::string textureFilename;
			s >> textureFilename;

			materialData.textureFilePath = directoryPath + "/" + textureFilename;
		}

	}

	return materialData;
}

ModelData  Model::LoadObjFile(const std::string& directoryPath, const std::string& filename) {
		ModelData modelData;
		std::vector<Vector4> positions;
		std::vector<Vector3> normals;
		std::vector<Vector2> texcoords;
		std::string line;

		std::ifstream file(directoryPath + "/" + filename);
		assert(file.is_open());

		while (std::getline(file, line)) {
			std::string identifier;
			std::istringstream s(line);
			s >> identifier;

			if (identifier == "v") {
				Vector4 position;
				s >> position.x >> position.y >> position.z;
				position.w = 1.0f;
				positions.push_back(position);
			}
			else if (identifier == "vt") {
				Vector2 texcoord;
				s >> texcoord.x >> texcoord.y;

				texcoord.y = 1.0f - texcoord.y; // Invert the y-coordinate of the texture coordinate

				texcoords.push_back(texcoord);
			}
			else if (identifier == "vn") {
				Vector3 normal;
				s >> normal.x >> normal.y >> normal.z;
				normals.push_back(normal);
			}
			else if (identifier == "f") {
				VertexData triangle[3];

				for (int32_t faceVertex = 0; faceVertex < 3; ++faceVertex) {
					std::string vertexDefinition;
					s >> vertexDefinition;

					std::istringstream v(vertexDefinition);
					uint32_t elementIndices[3];
					for (int32_t element = 0; element < 3; ++element) {
						std::string index;
						std::getline(v, index, '/');
						elementIndices[element] = std::stoi(index);
					}

					Vector4 position = positions[elementIndices[0] - 1];
					Vector2 texcoord = texcoords[elementIndices[1] - 1];
					Vector3 normal = normals[elementIndices[2] - 1];

					position.x *= -1.0f;
					normal.x *= -1.0f;

					triangle[faceVertex] = { position, texcoord, normal };
				}



				modelData.vertices.push_back(triangle[2]);
				modelData.vertices.push_back(triangle[1]);
				modelData.vertices.push_back(triangle[0]);

			}
			else if (identifier == "mtllib") {
				std::string materialFilename;
				s >> materialFilename;
				modelData.material = LoadMaterialTemplayeFile(directoryPath, materialFilename);
			}
		}
		return modelData;
}

