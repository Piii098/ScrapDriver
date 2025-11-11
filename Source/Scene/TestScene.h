#pragma once
#include "Core/Scene.h"

class TestScene : public Scene {
public:
	TestScene();
	~TestScene() override;
	bool Initialize(const std::string& name = "") override;


private:
	//void Input(const class InputAction& input) override;
	void InputScene(const class InputAction& input) override;
	void UpdateScene(float deltaTime) override;
	void LoadData() override;
	void LoadObjects() override;

	class CameraObject* _camera;
};