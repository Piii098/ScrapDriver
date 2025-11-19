# レイトレーシング マテリアルシステム

## 概要

オブジェクトごとに異なるマテリアル設定を適用できるシステムを実装しました。
ハードコードされた位置判定を排除し、InstanceIDベースの柔軟なマテリアル管理を実現しています。

## 主な変更点

### 1. データ構造の追加

**`ShaderStruct.h`**に新しい構造体を追加:

```cpp
// オブジェクトごとのマテリアルデータ
struct PerObjectMaterialData {
    Vector4 baseColor;          // 基本色（RGBA）
    float reflectanceCoef;      // 反射係数
    float diffuseCoef;          // 拡散反射係数
    float specularCoef;         // 鏡面反射係数
    float specularPower;        // 鏡面反射の鋭さ
    Vector2 uvScale;            // UVスケール
    uint textureIndex;          // テクスチャインデックス（0=なし、1以上=あり）
    uint materialType;          // マテリアルタイプ（0=標準, 1=発光など）
};
```

### 2. シェーダー側の変更

**`Raytracing.hlsl`**:
- `StructuredBuffer<PerObjectMaterial> MaterialBuffer : register(t2)` を追加
- `MyClosestHitShader`内で`InstanceID()`を使用してマテリアルを取得
- ハードコードされた位置判定を削除

```hlsl
// オブジェクトのIDを取得
uint instanceID = InstanceID();

// マテリアルデータを取得
PerObjectMaterial material = MaterialBuffer[instanceID];

// マテリアルに基づいて描画
float4 baseColor = material.baseColor;
if (material.textureIndex > 0) {
    baseColor = DiffuseTexture.SampleLevel(DiffuseSampler, uv, 0);
}
```

### 3. C++側の実装

**`RaytracingManager`**に以下の機能を追加:

- `CreatePerObjectMaterialBuffer()`: オブジェクトごとのマテリアルバッファを作成
- `UpdatePerObjectMaterialBuffer()`: マテリアルデータを更新
- `SetMeshMaterial()`: 特定のメッシュのマテリアルを設定
- `AddMesh()`にマテリアルパラメータを追加

**Root Signature**にパラメータを追加:
- `Root Parameter 5`: SRV Descriptor Table (t2 - MaterialBuffer)

## 使用方法

### 基本的な使い方

```cpp
// RaytracingModelComponentでオブジェクトを作成
auto* raytracingModel = new RaytracingModelComponent(*gameObject);

// マテリアルを設定
PerObjectMaterialData material = {};
material.baseColor = Vector4(1.0f, 0.0f, 0.0f, 1.0f); // 赤色
material.reflectanceCoef = 0.5f;  // 反射率50%
material.diffuseCoef = 0.7f;      // 拡散反射70%
material.specularCoef = 0.8f;     // 鏡面反射80%
material.specularPower = 64.0f;   // 鋭い鏡面反射
material.uvScale = Vector2(2.0f, 2.0f); // UVスケール
material.textureIndex = 0;        // テクスチャなし
material.materialType = 0;        // 標準マテリアル

raytracingModel->SetMaterial(material);
```

### テクスチャ付きマテリアル

```cpp
// テクスチャを設定（自動的にtextureIndexが1になる）
raytracingModel->SetTexture("Assets/Textures/brick.png");

// マテリアルをカスタマイズ
PerObjectMaterialData material = {};
material.baseColor = Vector4(1.0f, 1.0f, 1.0f, 1.0f); // 白（テクスチャ色そのまま）
material.reflectanceCoef = 0.2f;  // 反射率20%
material.diffuseCoef = 0.8f;
material.uvScale = Vector2(4.0f, 4.0f); // タイル4x4
material.textureIndex = 1;        // テクスチャあり

raytracingModel->SetMaterial(material);
```

### 発光マテリアル

```cpp
PerObjectMaterialData emissiveMaterial = {};
emissiveMaterial.baseColor = Vector4(1.0f, 0.8f, 0.2f, 1.0f); // オレンジ色
emissiveMaterial.reflectanceCoef = 0.0f;
emissiveMaterial.diffuseCoef = 0.0f;
emissiveMaterial.materialType = 1; // 発光マテリアル

raytracingModel->SetMaterial(emissiveMaterial);
```

### 複数オブジェクトの設定例

```cpp
// 床
auto* floor = new RaytracingModelComponent(*floorObject);
PerObjectMaterialData floorMaterial = {};
floorMaterial.baseColor = Vector4(0.8f, 0.8f, 0.8f, 1.0f);
floorMaterial.reflectanceCoef = 0.1f;
floorMaterial.diffuseCoef = 0.9f;
floorMaterial.uvScale = Vector2(10.0f, 10.0f);
floor->SetMaterial(floorMaterial);
floor->SetTexture("Assets/Textures/floor.png");

// 鏡面オブジェクト
auto* mirror = new RaytracingModelComponent(*mirrorObject);
PerObjectMaterialData mirrorMaterial = {};
mirrorMaterial.baseColor = Vector4(0.9f, 0.9f, 1.0f, 1.0f);
mirrorMaterial.reflectanceCoef = 0.9f; // 高い反射率
mirrorMaterial.diffuseCoef = 0.1f;
mirrorMaterial.specularCoef = 1.0f;
mirrorMaterial.specularPower = 128.0f;
mirror->SetMaterial(mirrorMaterial);

// マットな壁
auto* wall = new RaytracingModelComponent(*wallObject);
PerObjectMaterialData wallMaterial = {};
wallMaterial.baseColor = Vector4(0.7f, 0.5f, 0.3f, 1.0f);
wallMaterial.reflectanceCoef = 0.05f; // ほぼ反射なし
wallMaterial.diffuseCoef = 0.95f;
wallMaterial.specularCoef = 0.1f;
wallMaterial.specularPower = 8.0f; // ぼやけた鏡面反射
wall->SetMaterial(wallMaterial);
```

## 今後の改善案

1. **頂点データの完全サポート**: 現在は位置のみを使用していますが、法線とUVを頂点データから補間する
2. **複数テクスチャのサポート**: Texture2DArrayを使用して各オブジェクトに異なるテクスチャを割り当て
3. **マテリアルテンプレート**: よく使うマテリアル設定をプリセットとして保存
4. **PBRマテリアル**: Metallic/Roughnessワークフローの実装
5. **法線マップ**: より詳細な表面の表現

## 注意事項

- 最大100オブジェクトまでサポート（`CreatePerObjectMaterialBuffer`で設定）
- マテリアルを変更した後は自動的にGPUバッファが更新されます
- InstanceIDはメッシュ追加順に割り当てられます
