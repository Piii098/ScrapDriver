#pragma once
#include "DirectX/DirectX.h"
#include "Framework.h"

class Scene {
public:
	Scene() = default;
	~Scene();
	
	bool Initialize();
	void Update();
	void Render();
	
	// ゲッター
	class Model* GetModel() const { return _model; }

private:
	// モデル管理
	class Model* _model = nullptr;
};