#pragma once
#include "Core/GameObject.h"

class CameraObject : public GameObject {
public:
	CameraObject(Scene& scene);
	~CameraObject();

	void UpdateObject(const float deltaTime) override;	
	void InputObject(InputAction& input) override;

private:
	Vector2 _direction;
	class CameraComponent* _cameraComp;
	class RigidbodyComponent* _rigidbodyComp;
};
