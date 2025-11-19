# 統一レンダリングアーキテクチャ使用ガイド

レイトレーシングとラスタライズを統一的に扱えるようになりました。

## 変更点の概要

### 1. ModelComponentの統一
- `RaytracingModelComponent`を廃止
- `ModelComponent`が両方のレンダリングモードに対応
- シーン/オブジェクト側はレンダリング方式を意識不要

### 2. レンダリングモード切り替え
- `RendererManager`でモード切り替え
- ゲームクラスやシーンで動的に変更可能

### 3. 自動管理
- ModelComponentは自動的にRendererManagerに登録
- モード切り替え時も自動的に対応

## 使用方法

### 基本的な使い方（Quad）

```cpp
// シーンでの使用例
void TestScene::LoadObjects() {
    // カメラ（CameraComponentが自動的にRendererManagerに認識される）
    _camera = CreateGameObject<CameraObject>();
    _camera->Name("MainCamera");
    auto cameraComp = _camera->CreateComponent<CameraComponent>();
    cameraComp->SetFOV(60.0f);
    cameraComp->SetAsActiveCamera(); // アクティブカメラに設定（自動的に最初のカメラがアクティブになる）
    _camera->Position(0.0f, 3.0f, -8.0f);

    // オブジェクト作成（統一されたModelComponent）
    GameObject* floor = CreateGameObject<GameObject>();
    floor->Name("Floor");
    floor->Position(0.0f, -1.0f, 0.0f);
    floor->Rotation(Quaternion::FromAxisAngle(Vector3::UnitX, Math::ToRadians(90.0f)));
    floor->Scale(100.0f, 100.0f, 1.0f);
    
    // ModelComponentを追加
    auto* modelComp = floor->CreateComponent<ModelComponent>();
    modelComp->LoadQuad(); // 四角形メッシュ
    modelComp->SetTexture("Assets/texture.jpg");
    
    // マテリアル設定
    PerMeshMaterialData material = {};
    material.baseColor = Vector4(0.9f, 0.9f, 0.9f, 1.0f);
    material.metallic = 0.0f;
    material.roughness = 0.4f;
    material.reflectance = 0.6f;
    material.materialType = 0;
    material.uvScale = Vector2(1.0f, 1.0f);
    modelComp->SetMaterial(material);
}
```

### メッシュファイル読み込み

```cpp
// FBXなどのモデルファイルを読み込む
auto* modelComp = gameObject->CreateComponent<ModelComponent>();
modelComp->LoadModel("Assets/Model/Alicia/FBX/Alicia_solid_MMD.FBX");
```

### レンダリングモードの切り替え

```cpp
// ゲームクラスで切り替え
void Game::SwitchToRaytracing() {
    _rendererManager->SetRenderingMode(RenderingMode::RAYTRACING);
}

void Game::SwitchToRasterization() {
    _rendererManager->SetRenderingMode(RenderingMode::RASTERIZATION);
}

// 実行時に切り替え（例：キー入力）
void Game::ProcessInput(const InputAction& input) {
    if (input.IsKeyPressed(SDL_SCANCODE_F1)) {
        // レイトレーシング
        _rendererManager->SetRenderingMode(RenderingMode::RAYTRACING);
    }
    if (input.IsKeyPressed(SDL_SCANCODE_F2)) {
        // ラスタライズ
        _rendererManager->SetRenderingMode(RenderingMode::RASTERIZATION);
    }
}
```

### 現在のモード確認

```cpp
if (_rendererManager->IsRaytracing()) {
    SDL_Log("Currently using raytracing");
} else {
    SDL_Log("Currently using rasterization");
}
```

## JSON統合

JSONからもModelComponentを使用できます：

```json
{
  "version": 1,
  "gameObjects": [
    {
      "name": "Floor",
      "type": "ModelObject",
      "position": [0.0, -1.0, 0.0],
      "rotation": [0.7071068, 0.0, 0.0, 0.7071068],
      "scale": [100.0, 100.0, 1.0],
      "properties": {
        "modelType": "quad",
        "texturePath": "Assets/texture.jpg",
        "material": {
          "baseColor": [0.9, 0.9, 0.9, 1.0],
          "metallic": 0.0,
          "roughness": 0.4,
          "reflectance": 0.6,
          "materialType": 0,
          "uvScale": [1.0, 1.0]
        }
      }
    }
  ]
}
```

## 移行ガイド

### 既存のRaytracingModelComponentから

**変更前：**
```cpp
auto* rayComp = gameObject->CreateComponent<RaytracingModelComponent>();
rayComp->SetTexture("Assets/texture.jpg");
rayComp->SetMaterial(material);
```

**変更後：**
```cpp
auto* modelComp = gameObject->CreateComponent<ModelComponent>();
modelComp->LoadQuad();
modelComp->SetTexture("Assets/texture.jpg");
modelComp->SetMaterial(material);
```

## カメラ設定

### CameraComponentの使用
カメラは`CameraComponent::GetActiveCamera()`から自動取得されます：

```cpp
// シーンでカメラを作成
_camera = CreateGameObject<CameraObject>();
auto cameraComp = _camera->CreateComponent<CameraComponent>();
cameraComp->SetFOV(60.0f);
cameraComp->SetAsActiveCamera(); // アクティブに設定
_camera->Position(0.0f, 3.0f, -8.0f);

// RendererManagerは自動的にアクティブカメラを使用
// 追加の設定は不要
```

### 複数カメラの切り替え
```cpp
// カメラ1
auto camera1 = gameObject1->CreateComponent<CameraComponent>();
camera1->SetAsActiveCamera();

// カメラ2
auto camera2 = gameObject2->CreateComponent<CameraComponent>();

// 切り替え
camera2->SetAsActiveCamera(); // これでカメラ2がアクティブになる
```

## 内部動作

### レイトレーシングモード
1. ModelComponentがRendererManagerに自動登録
2. ワールド変換更新時に自動的にレイトレーシングメッシュを更新
3. Render()は呼ばれない（レイトレーシングで描画）
4. CameraComponent::GetActiveCamera()からカメラ情報を取得

### ラスタライズモード
1. CameraComponent::GetActiveCamera()からビュー・プロジェクション行列を取得
2. GameObject::Render() → Component::Render()が呼ばれる
3. ModelComponent::Render()で従来のラスタライズ描画
4. レイトレーシングメッシュは更新されない

## 注意事項

- モード切り替え時は自動的に全モデルが再構築されます
- 大量のモデルがある場合、切り替えに時間がかかる可能性があります
- 両方のモード用のリソース（バッファ）が作成されます

## パフォーマンス最適化

### メモリ使用量削減
どちらか一方のモードのみ使用する場合：

```cpp
// レイトレーシングのみ使用
_rendererManager->SetRenderingMode(RenderingMode::RAYTRACING);
// この場合、ラスタライズ用バッファは作成されるが使用されない

// ラスタライズのみ使用
_rendererManager->SetRenderingMode(RenderingMode::RASTERIZATION);
// レイトレーシング初期化を無効化すればメモリ節約
```

### 動的切り替えを無効化
固定モードの場合は`RendererManager`初期化時に設定：

```cpp
bool RendererManager::Initialize() {
    // ...
    
    // レイトレーシングを使わない場合はコメントアウト
    // InitializeRaytracing();
    
    // デフォルトモード設定
    _renderingMode = RenderingMode::RASTERIZATION;
    
    return true;
}
```

## 今後の拡張予定

- [ ] JSON統合の完全実装
- [ ] マルチメッシュモデルのレイトレーシング対応
- [ ] ハイブリッドレンダリング（一部のみレイトレーシング）
- [ ] モード切り替え時のリソース最適化
