#include "Model.h"

void Model::LoadModel(const std::string& directoryPath, const std::string& filename) {
	modelData = LoadObjFile(directoryPath, filename);

	// == vertex resource for model ==
	vertexResource = CreateBufferResource(engineCommon_->GetDevice(), sizeof(VertexData) * modelData.vertices.size());

	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	vertexBufferView.SizeInBytes = UINT(sizeof(VertexData) * modelData.vertices.size());
	vertexBufferView.StrideInBytes = sizeof(VertexData);

	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	std::memcpy(vertexData, modelData.vertices.data(), sizeof(*vertexData) * modelData.vertices.size());

	// material resource
	materialResource = CreateBufferResource(engineCommon_->GetDevice(), sizeof(Material));

	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	*materialData = { {1.0f, 1.0f, 1.0f, 1.0f}, 1 };
	materialData->uvTransform = Matrix4x4::Identity();

	// WVP resource
	wvpResource = CreateBufferResource(engineCommon_->GetDevice(), sizeof(TransformationMatrix));

	wvpResource->Map(0, nullptr, reinterpret_cast<void**>(&wvpData));
	*wvpData = { Matrix4x4::Identity(), Matrix4x4::Identity() };

	//load sencond texture for sprite
	DescriptorHandle texHandle = TextureManager::GetInstance().Load(
		modelData.material.textureFilePath,
		engineCommon_->GetDevice(),
		engineCommon_->GetCommandList(),
		engineCommon_->GetSRVAllocator() // You will need to add this getter to EngineCommon
	);

	// Store ONLY the GPU handle needed for drawing
	textureSrvGPUHandle = texHandle.gpuHandle;

}

void Model::UsingTemplateModel(int type) {
	switch (type) {
	case 0: {
		vertexResource = CreateBufferResource(engineCommon_->GetDevice(), sizeof(VertexData) * 4);
		vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
		vertexBufferView.SizeInBytes = sizeof(VertexData) * 4;
		vertexBufferView.StrideInBytes = sizeof(VertexData);

		vertexData = nullptr;
		vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));

		vertexData[0].position = { 0.0f, 360.f, 0.0f, 1.0f };
		vertexData[0].texcoord = { 0.0f, 1.0f };
		vertexData[0].normal = { 0.0f, 0.0f, -1.0f };

		vertexData[1].position = { 0.0f, 0.0f, 0.0f, 1.0f };
		vertexData[1].texcoord = { 0.0f, 0.0f };
		vertexData[1].normal = { 0.0f, 0.0f, -1.0f };

		vertexData[2].position = { 640.f, 360.f, 0.0f, 1.0f };
		vertexData[2].texcoord = { 1.0f, 1.0f };
		vertexData[2].normal = { 0.0f, 0.0f, -1.0f };

		vertexData[3].position = { 640.f, 0.0f, 0.0f, 1.0f };
		vertexData[3].texcoord = { 1.0f, 0.0f };
		vertexData[3].normal = { 0.0f, 0.0f, -1.0f };

		indexResource = CreateBufferResource(engineCommon_->GetDevice(), sizeof(uint32_t) * 6);
		indexBufferView.BufferLocation = indexResource->GetGPUVirtualAddress();
		indexBufferView.SizeInBytes = sizeof(uint32_t) * 6;
		indexBufferView.Format = DXGI_FORMAT_R32_UINT;

		uint32_t* indexData = nullptr;
		indexResource->Map(0, nullptr, reinterpret_cast<void**>(&indexData));
		indexData[0] = 0; indexData[1] = 1; indexData[2] = 2;
		indexData[3] = 1; indexData[4] = 3; indexData[5] = 2;

		wvpResource = CreateBufferResource(engineCommon_->GetDevice(), sizeof(TransformationMatrix));
		wvpData = nullptr;
		wvpResource->Map(0, nullptr, reinterpret_cast<void**>(&wvpData));
		*wvpData = { Matrix4x4::Identity(), Matrix4x4::Identity() };

		materialResource = CreateBufferResource(engineCommon_->GetDevice(), sizeof(Material));
		materialData = nullptr;
		materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
		materialData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
		materialData->enableLighting = false;
		materialData->uvTransform = Matrix4x4::Identity();

		indexCount = 6;
		break;
	}
	case 1: {
		// == vertex resource for sphere ==
		const uint32_t kSubdivisoin = 20;
		const uint32_t kVertexCount = kSubdivisoin * kSubdivisoin * 4;
		const uint32_t kIndexCount = kSubdivisoin * kSubdivisoin * 6;

		vertexResource = CreateBufferResource(engineCommon_->GetDevice(), sizeof(VertexData) * kVertexCount);

		// vertex buffer view
		vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
		vertexBufferView.SizeInBytes = sizeof(VertexData) * kVertexCount;
		vertexBufferView.StrideInBytes = sizeof(VertexData);

		// index buffer view
		indexResource = CreateBufferResource(engineCommon_->GetDevice(), sizeof(uint32_t) * kIndexCount);
		indexBufferView.BufferLocation = indexResource->GetGPUVirtualAddress();
		indexBufferView.SizeInBytes = sizeof(uint32_t) * kIndexCount;
		indexBufferView.Format = DXGI_FORMAT_R32_UINT;

		uint32_t* indexData = nullptr;
		indexResource->Map(0, nullptr, reinterpret_cast<void**>(&indexData));

		// copy vertex data
		vertexData = nullptr;
		vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));

		const float pi = 3.14159265358979323846f;
		const float kLonEvery = (pi * 2 / kSubdivisoin);
		const float kLatEvery = (pi / kSubdivisoin);

		for (uint32_t latIndex = 0; latIndex < kSubdivisoin; ++latIndex) {
			float lat = -pi / 2.0f + latIndex * kLatEvery;
			for (uint32_t lonIndex = 0; lonIndex < kSubdivisoin; ++lonIndex) {
				float lon = lonIndex * kLonEvery;
				uint32_t start = (latIndex * kSubdivisoin + lonIndex) * 4;
				uint32_t indexStart = (latIndex * kSubdivisoin + lonIndex) * 6;

				vertexData[start + 0].position = { cosf(lat) * cosf(lon), sinf(lat), cosf(lat) * sinf(lon), 1.0f };
				vertexData[start + 0].texcoord = { lon / (2 * pi), 1.0f - (lat + pi / 2) / pi };
				vertexData[start + 0].normal = { vertexData[start + 0].position.x, vertexData[start + 0].position.y, vertexData[start + 0].position.z };

				vertexData[start + 1].position = { cosf(lat + kLatEvery) * cosf(lon), sinf(lat + kLatEvery), cosf(lat + kLatEvery) * sinf(lon), 1.0f };
				vertexData[start + 1].texcoord = { lon / (2 * pi), 1.0f - (lat + kLatEvery + pi / 2) / pi };
				vertexData[start + 1].normal = { vertexData[start + 1].position.x, vertexData[start + 1].position.y, vertexData[start + 1].position.z };

				vertexData[start + 2].position = { cosf(lat) * cosf(lon + kLonEvery), sinf(lat), cosf(lat) * sinf(lon + kLonEvery), 1.0f };
				vertexData[start + 2].texcoord = { (lon + kLonEvery) / (2 * pi), 1.0f - (lat + pi / 2) / pi };
				vertexData[start + 2].normal = { vertexData[start + 2].position.x, vertexData[start + 2].position.y, vertexData[start + 2].position.z };

				vertexData[start + 3].position = { cosf(lat + kLatEvery) * cosf(lon + kLonEvery), sinf(lat + kLatEvery), cosf(lat + kLatEvery) * sinf(lon + kLonEvery), 1.0f };
				vertexData[start + 3].texcoord = { (lon + kLonEvery) / (2 * pi), 1.0f - (lat + kLatEvery + pi / 2) / pi };
				vertexData[start + 3].normal = { vertexData[start + 3].position.x, vertexData[start + 3].position.y, vertexData[start + 3].position.z };

				indexData[indexStart + 0] = start + 0; indexData[indexStart + 1] = start + 1; indexData[indexStart + 2] = start + 2;
				indexData[indexStart + 3] = start + 1; indexData[indexStart + 4] = start + 3; indexData[indexStart + 5] = start + 2;
			}
		}

		// material resource
		materialResource = CreateBufferResource(engineCommon_->GetDevice(), sizeof(Material));
		materialData = nullptr;
		materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
		*materialData = { {1.0f, 1.0f, 1.0f, 1.0f}, 1 };
		materialData->uvTransform = Matrix4x4::Identity();

		// WVP resource
		wvpResource = CreateBufferResource(engineCommon_->GetDevice(), sizeof(TransformationMatrix));
		wvpData = nullptr;
		wvpResource->Map(0, nullptr, reinterpret_cast<void**>(&wvpData));
		*wvpData = { Matrix4x4::Identity(), Matrix4x4::Identity() };

		indexCount = kIndexCount;
		break;
	}
	default:
		assert(false && "Invalid model type");
		break;
	}
	//load sencond texture for sprite
	DescriptorHandle texHandle = TextureManager::GetInstance().Load(
		"resources/05_02/uvChecker.png",
		engineCommon_->GetDevice(),
		engineCommon_->GetCommandList(),
		engineCommon_->GetSRVAllocator() // You will need to add this getter to EngineCommon
	);

	// Store ONLY the GPU handle needed for drawing
	textureSrvGPUHandle = texHandle.gpuHandle;
}

void Model::Draw() {
	engineCommon_->GetCommandList()->SetGraphicsRootSignature(engineCommon_->GetRootSignature());
	engineCommon_->GetCommandList()->SetPipelineState(engineCommon_->GetPSOManager().GetPSO(PSOType::Opaque3D));
	if (indexResource) {
		engineCommon_->GetCommandList()->IASetIndexBuffer(&indexBufferView);
	}
	engineCommon_->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView);
	engineCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	engineCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
	engineCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(1, wvpResource->GetGPUVirtualAddress());
	engineCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(3, engineCommon_->GetDirectionalLightResource()->GetGPUVirtualAddress());
	engineCommon_->GetCommandList()->SetGraphicsRootDescriptorTable(2, textureSrvGPUHandle);
	if (indexResource) {
		engineCommon_->GetCommandList()->DrawIndexedInstanced(indexCount, 1, 0, 0, 0);
	}
	else {
		engineCommon_->GetCommandList()->DrawInstanced(UINT(modelData.vertices.size()), 1, 0, 0);
	}
	
}

MaterialData Model::LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename) {
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
					if (!std::getline(v, index, '/') || index.empty()) {
						elementIndices[element] = 0; // 0 means "not present"; adjust downstream usage accordingly
						continue;
					}
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
			modelData.material = LoadMaterialTemplateFile(directoryPath, materialFilename);
		}
	}
	return modelData;
}

