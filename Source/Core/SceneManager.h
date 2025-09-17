#pragma once
#include "Framework.h"
#include "Core/Scene.h"
#include <unordered_map>
#include <memory>

class SceneManager {
public:
    static SceneManager& Instance();
    
    ~SceneManager();
    
    // シーンの管理
    void AddScene(const std::string& name, std::shared_ptr<Scene> scene);
    void RemoveScene(const std::string& name);
    bool ChangeScene(const std::string& name);
    
    // 現在のシーン
    std::shared_ptr<Scene> GetCurrentScene() const { return _currentScene; }
    std::shared_ptr<Scene> GetScene(const std::string& name) const;
    
    // 更新とレンダリング
    void Update(float deltaTime);
    void Render();
    void Cleanup();
    
private:
    SceneManager() = default;
    
    std::unordered_map<std::string, std::shared_ptr<Scene>> _scenes;
    std::shared_ptr<Scene> _currentScene;
    
    // シングルトンパターン
    SceneManager(const SceneManager&) = delete;
    SceneManager& operator=(const SceneManager&) = delete;
};
