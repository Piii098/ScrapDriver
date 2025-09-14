#pragma once
#include "Framework.h"
#include "DirectX.h"

class Camera {
public:
	Camera();
	~Camera();

	void Render();

	void Position(const Vector3& position) { _position = position; }
	const Vector3& Position() const { return _position; }
	const Matrix4x4& ViewMatrix() const { return _viewMatrix; }

private:
	Vector3 _position;
	Matrix4x4 _viewMatrix;
};