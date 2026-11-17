#pragma once
#include "Core/Scene.h"

#include <vector>

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

	// カメラ巡回（ウェイポイント）
	bool _cameraTourEnabled = true;
	std::vector<Vector3> _cameraTourPoints;
	Vector3 _cameraTourLookAt = Vector3(0.0f, 60.0f, 50.0f);
	float _cameraTourSegmentDurationSec = 4.0f;
	float _cameraTourSegmentTimeSec = 0.0f;
	int _cameraTourIndex = 0;
};
