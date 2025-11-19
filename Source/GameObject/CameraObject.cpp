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

	const float mouseSensitivity = 0.003f; // マウス感度（0.001〜0.01ぐらいで調整）

	// Yaw（左右回転）- Y軸回転をGameObjectのRotationに適用
	if (mouse.x != 0) {
		float yawDelta = mouse.x * mouseSensitivity;
		Quaternion yawRotation = Quaternion(Vector3::UnitY, yawDelta);
		Rotation(Quaternion::Concatenate(Rotation(), yawRotation));
	}

	// Pitch（上下回転）- マウスの相対移動をpitchに加算
	if (mouse.y != 0) {
		float pitchDelta = mouse.y * mouseSensitivity;
		_cameraComp->AddPitch(pitchDelta);
		
		// // デバッグ: マウス入力をログ出力
		// static int logCounter = 0;
		// if (++logCounter >= 30) {
		// 	SDL_Log("Mouse Y: %.2f, PitchDelta: %.4f, Current Pitch: %.3f", 
		// 		mouse.y, pitchDelta, _cameraComp->GetPitch());
		// 	logCounter = 0;
		// }
	}

	//if (input.IsKey(SDL_SCANCODE_LEFT) || input.IsKey(SDL_SCANCODE_A)) {
	//	// 左移動（カメラの左方向）
	//	_position -= right * moveSpeed;
	//}

	//if (input.IsKey(SDL_SCANCODE_RIGHT) || input.IsKey(SDL_SCANCODE_D)) {
	//	// 右移動（カメラの右方向）
	//	_position += right * moveSpeed;
	//}

	if (input.IsKey(SDL_SCANCODE_R)) {
		_position += Vector3(0.0f, moveSpeed, 0.0f);
	}
	if (input.IsKey(SDL_SCANCODE_F)) {
		_position -= Vector3(0.0f, moveSpeed, 0.0f);
	}
}

void CameraObject::UpdateObject(const float deltaTime) {
	Vector2 newVec = _direction * 5.0f;

	_rigidbodyComp->Move(newVec.x, newVec.y);
}