#pragma once
#include "Core/Component.h"
#include "Framework.h"
#include "DirectX/ShaderStruct.h"

class Texture2D; // 前方宣言

class RaytracingModelComponent : public Component {
public:
    RaytracingModelComponent(class GameObject& parent, int updateLayer = 100);
    ~RaytracingModelComponent();
    
    bool LoadQuad(); // 四角形メッシュを作成してレイトレーシングに追加
    void UpdateMesh(); // GameObjectの位置が変わったらメッシュを更新
    void SetTexture(const std::string& filePath); // テクスチャを設定
    void SetMaterial(const PerMeshMaterialData& material); // マテリアルを設定
    
    // Componentのオーバーライド
    virtual void OnUpdateWorldTransform() override;
    
private:
    // 自動UV推定（トランスフォーム後の頂点から面内2軸と範囲、オフセットを求める）
    void ComputeAutoUV(const std::vector<Vertex>& transformedVertices, PerMeshMaterialData& inOutMaterial);

    std::vector<Vertex> _baseVertices; // 元の頂点データ（ローカル座標）
    std::vector<Vertex> _lastTransformedVertices; // 直近のワールド座標頂点（UV推定用）
    std::vector<unsigned short> _indices;
    bool _needsUpdate = false;
    Texture2D* _texture = nullptr; // テクスチャ
    int _meshId = -1; // RaytracingManagerでのメッシュID
    bool _hasPendingMaterial = false; // まだ適用されていないマテリアルがあるか
    PerMeshMaterialData _pendingMaterial; // 遅延適用用マテリアル
};
