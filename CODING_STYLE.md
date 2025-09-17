# ScrapDriver コーディング規約

## 1. ファイル構造・命名規則

### ファイル拡張子
- **ヘッダーファイル**: `.h`
- **実装ファイル**: `.cpp`
- **シェーダーファイル**: `.hlsl`

### ディレクトリ構造
```
Source/
├── Core/           # ゲームコア機能（Game, Scene等）
├── DirectX/        # DirectX関連（レンダリング、バッファ管理等）
├── Renderer/       # レンダラー管理（RendererManager, Camera等）
├── Resource/       # リソース管理（Model, Asset等）
├── Utility/        # ユーティリティ（Math等）
├── Input/          # 入力管理（InputManager等）
└── LevelLoader/    # レベルローディング
```

## 2. インクルード規則

```cpp
#pragma once
#include "対応するヘッダー.h"        // 最初に自分のヘッダー
#include "同プロジェクト内ヘッダー"    // プロジェクト内ヘッダー
#include <システムヘッダー>           // システム・外部ライブラリ
```

### 例
```cpp
#pragma once
#include "Core/Scene.h"
#include "DirectX/DirectX.h"
#include "Resource/Model.h"
#include "Utility/Math.h"
#include <filesystem>
```

## 3. 命名規則

### クラス名
- **PascalCase**: `ClassName`, `RendererManager`, `InputManager`

### メンバ変数
- **アンダースコア + camelCase**: `_memberVariable`
- **bool値**: `_isValid`, `_hasTexture`, `_canRender`

```cpp
class Example {
private:
    Type* _memberVariable;
    std::vector<Type*> _items;
    bool _isValid = false;
    int _frameCount = 0;
};
```

### メソッド名
- **PascalCase**: `Initialize()`, `Update()`, `Render()`
- **Get/Set**: `GetModel()`, `SetProjection()`, `GetVertexBuffers()`
- **Is/Has**: `IsValid()`, `IsLoaded()`, `HasTexture()`

### 定数・列挙型
- **SCREAMING_SNAKE_CASE**: `BUTTON_STATE`, `FRAME_BUFFER_COUNT`

```cpp
enum class BUTTON_STATE {
    NONE,
    PRESSED,
    HELD,
    RELEASED
};
```

## 4. クラス設計パターン

### 基本構造
```cpp
class ClassName {
public:
    // コンストラクタ・デストラクタ
    ClassName();
    ~ClassName();
    
    // パブリックメソッド
    bool Initialize();
    void Update();
    void Render();
    
    // ゲッター
    Type* Member() const { return _member; }

private:
    // プライベートメンバ変数
    Type* _member = nullptr;
    
    // プライベートメソッド
    bool CreateResources();
};
```

### 前方宣言の使用
- ヘッダーファイルでは可能な限り前方宣言を使用
```cpp
class Scene {
private:
    class Model* _model = nullptr;           // 前方宣言
    class RendererManager* _renderer;        // 前方宣言
    std::vector<class VertexBuffer*> _vbs;   // 前方宣言
};
```

### RAII原則
- コンストラクタでリソース取得、デストラクタで解放
```cpp
Class::~Class() {
    if (_resource) {
        delete _resource;
        _resource = nullptr;
    }
}
```

## 5. エラーハンドリング・ログ出力

### ログ出力の使い分け
```cpp
// デバッグ情報
printf("Number of meshes loaded: %zu\n", meshCount);

// エラーメッセージ（日本語可）
printf("頂点バッファの生成に失敗\n");

// 英語エラーメッセージ
printf("Failed to load model\n");

// SDL公式ログ
SDL_Log("Scene::Initialize()\n");

// ワイド文字列用
wprintf(L"Loading texture: %s\n", texturePath.c_str());
```

### エラーハンドリングパターン
```cpp
bool Class::Initialize() {
    _resource = new Resource();
    if (!_resource->IsValid()) {
        printf("リソースの生成に失敗\n");
        return false;
    }
    return true;
}
```

## 6. 構造体設計

### 設定用構造体
```cpp
struct LoadSettings {
    std::wstring filePath;
    bool inverseU = false;     // デフォルト値を設定
    bool inverseV = false;
};
```

### GPU用データ構造
```cpp
struct alignas(256) Transform {
    Matrix4x4 World;
    Matrix4x4 View;
    Matrix4x4 Proj;
};
```

## 7. STLコンテナ使用パターン

```cpp
// ポインタ配列
std::vector<Type*> _collection;

// 値型配列
std::vector<Mesh> _meshes;

// 文字列
std::string _path;           // ASCII文字列
std::wstring _widePath;      // ワイド文字列（ファイルパス等）
```

## 8. DirectX/グラフィックス固有

### COMポインタ
```cpp
#include <wrl/client.h>
using Microsoft::WRL::ComPtr;

ComPtr<ID3D12Resource> _resource;
```

### 数学ライブラリ
```cpp
// DirectXMath使用
XMMATRIX matrix = XMMatrixIdentity();
XMVECTOR vector = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

// カスタム数学型使用
Vector3 position(0.0f, 1.0f, 0.0f);
Matrix4x4 worldMatrix = Matrix4x4::Identity;
```

## 9. コメント規約

### 日本語コメント
- 主要機能の説明
- アルゴリズムの解説
- 注意事項

```cpp
// アリシアのモデルは、テクスチャのUVのVだけ反転してるっぽい？
bool inverseV = true;
```

### 英語コメント
- デバッグ用
- 一時的な処理

```cpp
// Debug: Check texture loading status
if (!texture->IsValid()) {
    // Handle error
}
```

## 10. プリプロセッサ

### ヘッダーガード
```cpp
#pragma once
```

### プラットフォーム固有
```cpp
#ifdef _WIN32
#include <Windows.h>
#endif
```

## 11. 継承・ポリモーフィズム

### 基底クラス
```cpp
class Asset {
public:
    virtual bool Load(const std::string& path) = 0;
    virtual bool Unload() = 0;
    virtual ~Asset() = default;
};
```

### 派生クラス
```cpp
class Model : public Asset {
public:
    bool Load(const std::string& path) override;
    bool Unload() override;
};
```

## 12. 定数定義

### クラス内定数
```cpp
class Direct3D {
public:
    static const int FRAME_BUFFER_COUNT = 3;
};
```

### グローバル定数（最小限に）
```cpp
constexpr int MAX_TEXTURES = 256;
```

---

この規約に従って開発を行うことで、コードの一貫性と保守性を保つことができます。
