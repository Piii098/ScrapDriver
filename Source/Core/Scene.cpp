#include "Core/Scene.h"
#include "DirectX/DirectX.h"
#include "Resource/Model.h"
#include "Utility/Math.h"

Scene::~Scene() {
	if (_model) {
		delete _model;
		_model = nullptr;
	}
}

bool Scene::Initialize() {
	// Modelクラスを使用してモデルをロード
	_model = new Model();
	
	// アリシアモデルのロード
	Model::LoadSettings settings;
	settings.filePath = L"Assets/Model/Alicia/FBX/Alicia_solid_Unity.FBX";
	settings.inverseU = false;
	settings.inverseV = true; // アリシアのモデルは、テクスチャのUVのVだけ反転してるっぽい？

	if (!_model->LoadFromFile(settings)) {
		printf("Failed to load model\n");
		return false;
	}

	SDL_Log("Scene::Initialize()\n");
	return true;
}

void Scene::Update() {
	// モデル関連の更新処理
	// 現在は特に処理なし
}

void Scene::Render() {
	// モデルのレンダリング処理
	// ModelComponentで行う予定のため、現在は空
}