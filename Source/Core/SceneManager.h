#pragma once
#include "Framework.h"
#include "Core/Scene.h"
#include <unordered_map>
#include <memory>

class SceneManager {
public:

	bool Initialize();
    
    // シーンの管理
    void AddScene(const std::string& name, std::shared_ptr<Scene> scene);
    void RemoveScene(const std::string& name);
    bool ChangeScene(const std::string& name);
    
    // 現在のシーン
    std::shared_ptr<Scene> GetCurrentScene() const { return _currentScene; }
    std::shared_ptr<Scene> GetScene(const std::string& name) const;
    
    // 更新とレンダリング
	void Input(class InputAction& input);
    void Update(float deltaTime);
    void Render();
    void Cleanup();
    
private:
    SceneManager();
    ~SceneManager();
    
    std::unordered_map<std::string, std::shared_ptr<Scene>> _scenes;
    std::shared_ptr<Scene> _currentScene;
    
	static SceneManager* s_instance;

public:

    static void CreateInstance() {
        DestroyInstance();
		s_instance = new SceneManager();
    }

    static void DestroyInstance() {
        if (s_instance) {
            delete s_instance;
            s_instance = nullptr;
        }
    }

    static SceneManager& Instance() {
        return *s_instance;
    }
};

#define SCENEMANAGER SceneManager::Instance()