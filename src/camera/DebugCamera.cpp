#include "DebugCamera.h"

//DebugCamera::DebugCamera() {
//	rotation_ = { 0, 0, 0 };
//	translation_ = { 0, 0, 0 };
//	viewMatrix_ = Matrix4x4::Identity();
//	projectionMatrix_ = Matrix4x4::Identity();
//}
//
//DebugCamera::~DebugCamera() {
//	// Destructor implementation (if needed)
//}

void DebugCamera::Initialize(int kClientWidth, int kClientHeight, Transform cameraTransform) {
	translation_ = cameraTransform.translation;

	Matrix4x4 rotationXMatrix = Matrix4x4::MakeRotationXMatrix(cameraTransform.rotation.x);
	Matrix4x4 rotationYMatrix = Matrix4x4::MakeRotationYMatrix(cameraTransform.rotation.y);
	Matrix4x4 rotationZMatrix = Matrix4x4::MakeRotationZMatrix(cameraTransform.rotation.z);
	
	matRot_ = rotationXMatrix * rotationYMatrix * rotationZMatrix;

	Matrix4x4 translateMatrix = Matrix4x4::MakeTranslateMatrix(translation_);
	Matrix4x4 scaleMatrix = Matrix4x4::MakeScaleMatrix({1.f,1.f,1.f});

	Matrix4x4 cameraMatrix = scaleMatrix * matRot_ * translateMatrix;
	viewMatrix_ = Matrix4x4::Inverse(cameraMatrix);
	projectionMatrix_ = Matrix4x4::MakePerspectiveFovMatrix(0.45f, static_cast<float>(kClientWidth) / static_cast<float>(kClientHeight), 0.1f, 100.0f); 
}

void DebugCamera::Update(BYTE key[256]) {
	const float rotationSpeed = 0.02f;

	Matrix4x4 matRotDelta = Matrix4x4::Identity();

	if (key[DIK_LEFT]) {
		matRotDelta = matRotDelta * Matrix4x4::MakeRotationYMatrix(-rotationSpeed);
	}
	if(key[DIK_RIGHT]) {
		matRotDelta = matRotDelta * Matrix4x4::MakeRotationYMatrix(rotationSpeed);
	}
	if (key[DIK_UP]) {
		matRotDelta = matRotDelta * Matrix4x4::MakeRotationXMatrix(-rotationSpeed);
	}
	if (key[DIK_DOWN]) {
		matRotDelta = matRotDelta * Matrix4x4::MakeRotationXMatrix(rotationSpeed);
	}

	matRot_ = matRotDelta * matRot_;

	const float moveSpeed = 0.1f;

	// Calculate forward, right, and up vectors based on the current rotation matrix
	Vector3 right = { matRot_.m[0][0], matRot_.m[0][1], matRot_.m[0][2] };    // row 0
	Vector3 up = { matRot_.m[1][0], matRot_.m[1][1], matRot_.m[1][2] };       // row 1
	Vector3 forward = { -matRot_.m[2][0], -matRot_.m[2][1], -matRot_.m[2][2] }; // -row 2 (negate for DirectX -Z forward)

	if (key[DIK_W]) {
		translation_ = translation_ - forward * moveSpeed;
	}
	if (key[DIK_S]) {
		translation_ = translation_ + forward * moveSpeed;
	}
	if (key[DIK_A]) {
		translation_ = translation_ - right * moveSpeed;
	}
	if (key[DIK_D]) {
		translation_ = translation_ + right * moveSpeed;
	}
	if (key[DIK_SPACE]) {
		translation_ = translation_ + up * moveSpeed;
	}
	if (key[DIK_LSHIFT]) {
		translation_ = translation_ - up * moveSpeed;
	}	

	// Update the view matrix based on the new rotation and translation
	Matrix4x4 translateMatrix = Matrix4x4::MakeTranslateMatrix(translation_);
	Matrix4x4 scaleMatrix = Matrix4x4::MakeScaleMatrix({ 1.f, 1.f, 1.f });
	Matrix4x4 cameraMatrix = scaleMatrix * matRot_ * translateMatrix;
	viewMatrix_ = Matrix4x4::Inverse(cameraMatrix);

}