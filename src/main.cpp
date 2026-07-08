
#include "EngineCommon.h"
#include "Model.h"

const int32_t kClientWidth = 1280;
const int32_t kClientHeight = 720;

// ========================= Entry point =======================
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {
	EngineCommon::Initialize(kClientWidth, kClientHeight);
	//EngineCommon::TempMainFunction();

	Transform transform{ {1.0f,1.0f,1.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f} };
	Transform cameraTransform{ {1.0f,1.0f,1.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,-5.0f} };
	DebugCamera debugCamera;
	debugCamera.Initialize(kClientWidth, kClientHeight, cameraTransform);
	Transform TransformSprite{ {1.0f,1.0f,1.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f} };
	Transform uvTransformSprite{ {1.0f,1.0f,1.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f} };

	int frameCount = 0;
	bool useMonsterBall = 0;

	Model a;
	a.LoadModel("resources/05_02", "axis.obj");

	while (true) {
		MSG msg{};
		if (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) {
				break;
			}
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
		else {

			EngineCommon::keyboard->Acquire();
			BYTE key[256] = {};
			EngineCommon::keyboard->GetDeviceState(sizeof(key), key);

			if (key[DIK_0]) {
				OutputDebugStringA("Hit 0.\n");
			}

			debugCamera.Update(key);

			frameCount++;

			if (frameCount == 210) {
				EngineCommon::SoundPlayWave(EngineCommon::xAudio2.Get(), EngineCommon::soundData1);
			}

			Matrix4x4 worldMatrix = Matrix4x4::MakeAffineMatrix(transform.scale, transform.rotation, transform.translation);

			Matrix4x4 wvpMatrix = worldMatrix * debugCamera.getViewProjectionMatrix();
			*a.wvpData = { wvpMatrix, worldMatrix };


			EngineCommon::PreDraw();

#ifdef _DEBUG
			// ImGui demo window
			ImGui::Begin("window");
			// camera control

			ImGui::DragFloat3("sprite transform", &TransformSprite.translation.x, 0.1f);
			ImGui::DragFloat2("sprite uv transform", &uvTransformSprite.translation.x, 0.01f);
			ImGui::DragFloat2("sprite uv scale", &uvTransformSprite.scale.x, 0.01f);
			ImGui::DragFloat("sprite uv rotation", &uvTransformSprite.rotation.z, 0.01f);

			ImGui::Checkbox("use monster ball texture", &useMonsterBall);
			ImGui::DragFloat3("Trasform", &transform.translation.x, 0.1f);
			ImGui::DragFloat3("Rotation", &transform.rotation.x, 0.1f);
			ImGui::DragFloat3("Scale", &transform.scale.x, 0.1f);

			ImGui::End();

			// ImGui render
			ImGui::Render();
#endif
			a.Draw();

			EngineCommon::PostDraw();

		}
	}
	EngineCommon::Finalize();

	return 0;
}