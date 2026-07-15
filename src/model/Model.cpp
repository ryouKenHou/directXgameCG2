#include "Model.h"

void Model::LoadModel(const std::string& directoryPath, const std::string& filename) {
	modelData = LoadObjFile(directoryPath, filename);

	// == vertex resource for model ==
	vertexResource = engineCommon_->CreateBufferResource(engineCommon_->device.Get(), sizeof(VertexData) * modelData.vertices.size());

	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	vertexBufferView.SizeInBytes = UINT(sizeof(VertexData) * modelData.vertices.size());
	vertexBufferView.StrideInBytes = sizeof(VertexData);

	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	std::memcpy(vertexData, modelData.vertices.data(), sizeof(*vertexData) * modelData.vertices.size());

	// material resource
	materialResource = engineCommon_->CreateBufferResource(engineCommon_->device.Get(), sizeof(Material));

	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	*materialData = { {1.0f, 1.0f, 1.0f, 1.0f}, 1 };
	materialData->uvTransform = Matrix4x4::Identity();

	// WVP resource
	wvpResource = engineCommon_->CreateBufferResource(engineCommon_->device.Get(), sizeof(TransformationMatrix));

	wvpResource->Map(0, nullptr, reinterpret_cast<void**>(&wvpData));
	*wvpData = { Matrix4x4::Identity(), Matrix4x4::Identity() };

	//load sencond texture for sprite
	DirectX::ScratchImage mipImages2 = engineCommon_->LoadTexture(modelData.material.textureFilePath);
	const DirectX::TexMetadata& metadata2 = mipImages2.GetMetadata();
	textureResource2 = engineCommon_->CreateTextureResource(engineCommon_->device.Get(), metadata2);
	intermediateResource2 = engineCommon_->UploadTextureData(textureResource2.Get(), mipImages2, engineCommon_->device.Get(), engineCommon_->commandList.Get());

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc2{};
	srvDesc2.Format = metadata2.format;
	srvDesc2.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc2.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc2.Texture2D.MipLevels = UINT(metadata2.mipLevels);

	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU2 = engineCommon_->GetCPUDescriptorHandle(engineCommon_->srvDescriptorHeap.Get(), engineCommon_->descriptorSizeSRV, 2);
	textureSrvHandleGPU2 = engineCommon_->GetGPUDescriptorHandle(engineCommon_->srvDescriptorHeap.Get(), engineCommon_->descriptorSizeSRV, 2);
	engineCommon_->device->CreateShaderResourceView(textureResource2.Get(), &srvDesc2, textureSrvHandleCPU2);

}

void Model::Draw() {
	engineCommon_->commandList->SetGraphicsRootSignature(engineCommon_->rootSignature.Get());
	engineCommon_->commandList->SetPipelineState(engineCommon_->pipelineState.Get());
	engineCommon_->commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
	engineCommon_->commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	engineCommon_->commandList->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
	engineCommon_->commandList->SetGraphicsRootConstantBufferView(1, wvpResource->GetGPUVirtualAddress());
	engineCommon_->commandList->SetGraphicsRootConstantBufferView(3, engineCommon_->directionalLightResource->GetGPUVirtualAddress());
	engineCommon_->commandList->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU2);
	engineCommon_->commandList->DrawInstanced(UINT(modelData.vertices.size()), 1, 0, 0);
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

