#pragma once
#include <unordered_map>
#include <string>
#include <SDL3/SDL.h>
#include <memory>
#include "Resource/Model.h"

/*
使用方法
このクラスはアセットの管理を行います。アセットをロード、取得、アンロードする機能を提供します。
*/

/// @brief アセットマネージャークラス
/// アセットの管理を行う
/// ロード、アンロード、取得を行う
/// テンプレートでアセットクラスを指定する
/// @tparam T Load関数とUnload関数を持つアセットクラス
template <typename T>
class AssetManager {
public:

    // ===== コンストラクタ・デストラクタ ===== //

    AssetManager(class Game& game) : _game(game) {}

    ~AssetManager() {
        UnloadAll(); // 全アセットをアンロード
    }


    // ===== ロード・アンロード ===== //

    /// @brief アセットのロード
    /// アセットをロードする
    /// @param assetName 登録するアセット名
    /// @param fileName ファイル名
    /// @return ロード成功ならtrue
    bool Load(const std::string& assetName, const std::string& fileName) { // アセット名とファイル名を受け取る
        // 既に存在するか確認
        auto iter = _assets.find(assetName);
        if (iter != _assets.end()) {
            // 既に存在する場合は成功と見なす
            return true; // 成功
        }

        // 新しいインスタンスを作成
        std::shared_ptr<T> asset = std::make_shared<T>();

        // ロード処理
        if (asset->Load(fileName)) { // ファイルからアセットをロード
            // ロード成功
            _assets.emplace(assetName, asset); // アセットをマップに追加
            return true; // 成功
        }
        else {
            // ロード失敗
            SDL_Log("Failed to load asset %s from file %s", assetName.c_str(), fileName.c_str()); // ログ出力
            return false; // 失敗
        }
    }

    /// @brief アセットの参照を取得
    /// アセットの参照を返し、存在しない場合はダミーを返す
    /// @param assetName 
    /// @return 
    std::shared_ptr<T> Get(const std::string& assetName) { // アセット名を受け取る
        auto iter = _assets.find(assetName);
        if (iter != _assets.end()) {
            return iter->second; // アセットが存在する場合は参照を返す
        }
        else {
            SDL_Log("Asset %s not found", assetName.c_str());
            static std::shared_ptr<T> dummy = std::make_shared<T>(); // ダミーアセットを作成
            return dummy; // ダミーアセットを返す
        }
	}

    /// @brief アセットのアンロード
    /// 
    /// @param assetName アンロードするアセット名
    void Unload(const std::string& assetName) { // アセット名を受け取る
        SDL_Log("Unload");
        auto iter = _assets.find(assetName);
        if (iter != _assets.end()) {
            iter->second.reset(); // メモリを解放
            _assets.erase(iter); // マップから削除
        }
    }

    /// @brief 全アセットのアンロード
    void UnloadAll() { // 全アセットをアンロード
        SDL_Log("Unload");
        for (auto& asset : _assets) {
            asset.second.reset(); // メモリを解放
        }
        _assets.clear(); // マップをクリア
    }

protected:

    class Game& _game; // ゲームクラスへの参照

    std::unordered_map<std::string, std::shared_ptr<T>> _assets; // アセットを格納するマップ
};

// シングルトン、モデル、テクスチャ、サウンドなどのアセットマネージャーを定義

using ModelManager = AssetManager<class Model>;
//using TextureManager = AssetManager<class Texture2D>;
//using SoundManager = AssetManager<class Sound>;
//using ShaderManager = AssetManager<class Shader>;

class AssetManagers {
public:
    // ModelManager& Models() { return _modelManager; }
    //TextureManager& Textures() { return _textureManager; }
    //SoundManager& Sounds() { return _soundManager; }
    //ShaderManager& Shaders() { return _shaderManager; }
    
private:
    AssetManagers(class Game& game) {
    }
    ~AssetManagers() {}
    static AssetManagers* s_instance;
    //ModelManager _modelManager;
    //TextureManager _textureManager;
    //SoundManager _soundManager;
    //ShaderManager _shaderManager;

public:
    static void CreateInstance(class Game& game) {
        if (!s_instance) {
            s_instance = new AssetManagers(game);
        }
    }
    static void DestroyInstance() {
        delete s_instance;
        s_instance = nullptr;
    }
    static AssetManagers& Instance() {
        return *s_instance;
    }
};

#define ASSETS AssetManagers::Instance()