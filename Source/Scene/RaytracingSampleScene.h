#pragma once
#include "Core/Scene.h"

class RaytracingSampleScene : public Scene {
public:
	RaytracingSampleScene();
	~RaytracingSampleScene() override;
	bool Initialize(const std::string& name = "") override;

private:
	void InputScene(const class InputAction& input) override;
	void UpdateScene(float deltaTime) override;
	void LoadData() override;
	void LoadObjects() override;

	class CameraObject* _camera;
};
