#pragma once
#include "Core/Scene.h"
#include <rapidjson/document.h>

class LoadingScene : public Scene {
public:
	LoadingScene(class SceneManager& manager);
	~LoadingScene();
	bool Initialize() override;
	void Shutdown() override;

	void LoadAssets(const rapidjson::Value& inObj);
	void UpdateScene(float deltaTime) override;

private:
	float _progress;

};