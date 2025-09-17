#include "Core/SceneManager.h"
#include "Core/Scene.h"

SceneManager& SceneManager::Instance() {
    static SceneManager instance;
    return instance;
}

SceneManager::~SceneManager() {
    Cleanup();
}

void SceneManager::AddScene(const std::string& name, std::shared_ptr<Scene> scene) {
    if (scene) {
        scene->Name(name);
        _scenes[name] = scene;
    }
}

void SceneManager::RemoveScene(const std::string& name) {
    auto it = _scenes.find(name);
    if (it != _scenes.end()) {
        // 現在のシーンを削除する場合は、現在のシーンをnullptrにする
        if (_currentScene == it->second) {
            if (_currentScene) {
                _currentScene->Shutdown();
            }
            _currentScene = nullptr;
        }
        _scenes.erase(it);
    }
}

// メモ : ロードを実装する
// ロードにはパス&登録名が必要
// LoadingScene内でjsonを受け取りロードをする->シーン変更
bool SceneManager::ChangeScene(const std::string& name) {
    auto it = _scenes.find(name);
    if (it != _scenes.end()) {
        // 現在のシーンの終了処理
        if (_currentScene) {
            _currentScene->Shutdown();
        }
        
        // 新しいシーンの開始処理
        _currentScene = it->second;
        if (_currentScene) {
            if (!_currentScene->Initialize()) {
                if (!_currentScene->Initialize()) {
                    _currentScene = nullptr;
                    return false;
                }
            }
            //_currentScene->OnEnter();
        }
        return true;
    }
    return false;
}

std::shared_ptr<Scene> SceneManager::GetScene(const std::string& name) const {
    auto it = _scenes.find(name);
    if (it != _scenes.end()) {
        return it->second;
    }
    return nullptr;
}

void SceneManager::Update(float deltaTime) {
    if (_currentScene) {
        _currentScene->Update(deltaTime);
    }
}

void SceneManager::Render() {
    if (_currentScene) {
        _currentScene->Render();
    }
}

void SceneManager::Cleanup() {
    if (_currentScene) {
        //_currentScene->OnExit();
    }
    _currentScene = nullptr;
    _scenes.clear();
}
