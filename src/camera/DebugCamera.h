#pragma once

#include "Matrix4x4.hpp"

// ================= temp include =========================
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

// ========================================================


class DebugCamera {
private:
	Matrix4x4 matRot_ = Matrix4x4::Identity();
	Vector3 translation_ = { 0,0,0 };
	Matrix4x4 viewMatrix_ = Matrix4x4::Identity();
	Matrix4x4 projectionMatrix_ = Matrix4x4::Identity();

public:

	void Initialize(int kClientWidth, int kClientHeight, Transform transform);

	void Update(BYTE key[256]);
	
	Matrix4x4 getViewProjectionMatrix() const {
		return viewMatrix_ * projectionMatrix_ ;
	}
};