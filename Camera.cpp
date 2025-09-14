#include "Camera.h"

Camera::Camera()
	: _position(Vector3::Zero) {
}

Camera::~Camera() {
}

void Camera::Render() {
	Vector3 up = Vector3::Up;
	Vector3 lookAt = Vector3::Forward;

	float yaw, pitch, roll;

	Vector3 target = _position + lookAt;

	_viewMatrix = XMMatrixLookAtLH(_position, target, up);

}