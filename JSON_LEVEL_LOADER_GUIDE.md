# JSONレベルローダー使用ガイド

ScrapDriverプロジェクトにJSONからゲームオブジェクトをロードする機能が追加されました。

## 機能概要

- JSONファイルからゲームオブジェクトを読み込み
- 位置、回転、スケールの設定
- 階層構造（親子関係）のサポート
- グローバルプロパティの読み込み

## 使用方法

### 1. JSONファイルの作成

`Assets/Levels/` ディレクトリにJSONファイルを作成します。

```json
{
  "version": 1,
  "globalProperties": {
    "sceneName": "MyScene",
    "ambientLight": [0.2, 0.2, 0.2]
  },
  "gameObjects": [
    {
      "name": "Floor",
      "position": [0.0, -1.0, 0.0],
      "rotation": [0.7071068, 0.0, 0.0, 0.7071068],
      "scale": [100.0, 100.0, 1.0],
      "children": [
        {
          "name": "ChildObject",
          "position": [0.0, 1.0, 0.0],
          "scale": [1.0, 1.0, 1.0]
        }
      ]
    }
  ]
}
```

### 2. シーンでの読み込み

シーンの `LoadObjects()` メソッドで以下のように呼び出します：

```cpp
#include "LevelLoader/LevelLoader.h"

void MyScene::LoadObjects() {
    // JSONファイルからレベルを読み込む
    LevelLoader::LoadLevel(*this, "Assets/Levels/MyLevel.json");
}
```

## JSONフォーマット詳細

### グローバルプロパティ

```json
"globalProperties": {
    "sceneName": "文字列",
    "ambientLight": [R, G, B]
}
```

### ゲームオブジェクト

各ゲームオブジェクトは以下のプロパティを持ちます：

- **name** (string): オブジェクト名
- **position** (array[3]): 位置 [x, y, z]
- **rotation** (array[4]): 回転（クォータニオン） [x, y, z, w]
- **scale** (array[3]): スケール [x, y, z]
- **children** (array): 子オブジェクトの配列（オプション）

### 回転値の計算

Quaternionの値は以下のように計算できます：

```cpp
// 例: Y軸周りに45度回転
Quaternion q = Quaternion::FromAxisAngle(Vector3::Up, Math::ToRadians(45.0f));
// q.x, q.y, q.z, q.w をJSONに記述
```

主な回転値の例：
- 90度X軸回転: `[0.7071068, 0.0, 0.0, 0.7071068]`
- 90度Y軸回転: `[0.0, 0.7071068, 0.0, 0.7071068]`
- 180度回転: `[1.0, 0.0, 0.0, 0.0]`

## JsonHelperユーティリティ

独自のプロパティを読み込む場合、`JsonHelper`クラスを使用できます：

```cpp
void MyGameObject::LoadProperties(const rapidjson::Value& inObj) {
    // 基本的な型の読み込み
    int health;
    JsonHelper::GetInt(inObj, "health", health);
    
    float speed;
    JsonHelper::GetFloat(inObj, "speed", speed);
    
    std::string texturePath;
    JsonHelper::GetString(inObj, "texture", texturePath);
    
    bool isActive;
    JsonHelper::GetBool(inObj, "active", isActive);
    
    // ベクトルの読み込み
    Vector3 velocity;
    JsonHelper::GetVector3(inObj, "velocity", velocity);
    
    Vector4 color;
    JsonHelper::GetVector4(inObj, "color", color);
}
```

## サンプル

`Assets/Levels/TestLevel.json` にサンプルファイルが用意されています。

## 注意事項

- JSONファイルのバージョンは現在1のみサポート
- パスは相対パスで指定（プロジェクトルートからの相対パス）
- 数値配列のサイズは厳密にチェックされます
- コンポーネントの追加は別途手動で行う必要があります（今後の拡張予定）

## 今後の拡張予定

- [ ] コンポーネントのJSONからの読み込み
- [ ] プレハブシステム
- [ ] マテリアル設定のサポート
- [ ] レイトレーシングモデルのパラメータ設定
