#include "GameObject/CameraObject.h"
#include "Input/InputAction.h"
#include "Component/CameraComponent.h"
#include "Component/RigidbodyComponent.h"

CameraObject::CameraObject(Scene& scene)
	: GameObject(scene) {
	_cameraComp = CreateComponent<CameraComponent>();
	_rigidbodyComp = CreateComponent<RigidbodyComponent>();
	_cameraComp->SetControlMode(CameraComponent::ControlMode::FIRST_PERSON);
}

CameraObject::~CameraObject() {
}

void CameraObject::InputObject(InputAction& input) {
	float moveSpeed = 0.5f;

	float straife = input.GetActionValue("Horizontal");
	float forward = input.GetActionValue("Vertical");

	if (!_cameraComp) return;

	_direction = Vector2(forward, straife);

	// nomalize
	if (_direction.Length() > 1.0f) {
		_direction.Normalize();
	}

	//input.SetRelativeMouseMode(true);

	//SDL_Log("mouse x: %.2f, y: %.2f", input.GetMousePosition().x, input.GetMousePosition().y);
	
	input.SetRelativeMouseMode(true);
	Vector2 mouse = input.GetMousePosition();

	const float mouseSensitivity = 0.007f; // ここで感度を調整（0.05〜0.2ぐらいで好みに合わせて）

	// yaw 左右回転 
	// pitch 上下回転

	const float maxYawSpeed = Math::Pi * 3.f;
	float yawSpeed = 0.0f;
	if (mouse.x != 0) {
		yawSpeed = mouse.x * maxYawSpeed * mouseSensitivity * 0.01f;
	}

	Quaternion q(Quaternion::Concatenate(Rotation(), Quaternion(Vector3::UnitY, yawSpeed)));

	Rotation(q);

	const float maxPitchSpeed = Math::Pi * 3.f;
	float pitchSpeed = 0.0f;
	if (mouse.y != 0) {
		pitchSpeed = mouse.y * maxPitchSpeed * mouseSensitivity * 0.3f;
	}

	_cameraComp->SetPitchSpeed(pitchSpeed);

	//if (input.IsKey(SDL_SCANCODE_LEFT) || input.IsKey(SDL_SCANCODE_A)) {
	//	// 左移動（カメラの左方向）
	//	_position -= right * moveSpeed;
	//}

	//if (input.IsKey(SDL_SCANCODE_RIGHT) || input.IsKey(SDL_SCANCODE_D)) {
	//	// 右移動（カメラの右方向）
	//	_position += right * moveSpeed;
	//}
}

void CameraObject::UpdateObject(const float deltaTime) {
	Vector2 newVec = _direction * 5.0f;

	_rigidbodyComp->Move(newVec.x, newVec.y);
}