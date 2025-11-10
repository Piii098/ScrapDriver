#include "Component/RaytracingModelComponent.h"
#include "Core/GameObject.h"
#include "Renderer/RendererManager.h"
#include "Core/Game.h"
#include "DirectX/Texture2D.h"

RaytracingModelComponent::RaytracingModelComponent(class GameObject& parent, int updateLayer)
    : Component(parent, updateLayer) {
    
    // 四角形メッシュを自動的に作成
    LoadQuad();
}

RaytracingModelComponent::~RaytracingModelComponent() {
}

bool RaytracingModelComponent::LoadQuad() {
    // 四角形の頂点データを作成（ローカル座標）
    Vertex vertices[4] = {};
    vertices[0].Position = { -0.5f, -0.5f, 0.0f }; vertices[0].UV = { 0.0f, 1.0f };
    vertices[1].Position = { -0.5f,  0.5f, 0.0f }; vertices[1].UV = { 0.0f, 0.0f };
    vertices[2].Position = {  0.5f, -0.5f, 0.0f }; vertices[2].UV = { 1.0f, 1.0f };
    vertices[3].Position = {  0.5f,  0.5f, 0.0f }; vertices[3].UV = { 1.0f, 0.0f };
    
    // インデックスデータ（2つの三角形で四角形を構成）
    unsigned short indices[] = {
        0, 1, 2,  // 1つ目の三角形
        2, 1, 3   // 2つ目の三角形
    };
    
    // ベース頂点データを保存（ローカル座標）
    _baseVertices = std::vector<Vertex>(std::begin(vertices), std::end(vertices));
    _indices = std::vector<unsigned short>(std::begin(indices), std::end(indices));
    
    // 初期位置でメッシュを追加（OnUpdateWorldTransformを使わない）
    _needsUpdate = true; // フラグを設定するだけ
    
    return true;
}

void RaytracingModelComponent::UpdateMesh() {
    // GameObjectのワールド変換行列を取得
    const auto& worldTransform = GetParent().WorldTransform();
    
    // ローカル座標の頂点をワールド座標に変換
    std::vector<Vertex> transformedVertices = _baseVertices;
    for (auto& vertex : transformedVertices) {
        // 頂点をワールド座標に変換
        Vector3 worldPos = Vector3::Transform(vertex.Position, worldTransform);
        vertex.Position = worldPos;
    }
    
    // レイトレーシングマネージャーにメッシュを追加/更新
    auto& rendererManager = GAME.GetRendererManager();
    if (rendererManager.GetRaytracingManager()) {
        if (_meshId == -1) {
            // 初回は新規追加
            _meshId = rendererManager.GetRaytracingManager()->AddMesh(transformedVertices, _indices);
            if (_meshId >= 0) {
                SDL_Log("RaytracingModelComponent: Quad mesh added with ID=%d at position (%.2f, %.2f, %.2f)",
                    _meshId, GetParent().Position().x, GetParent().Position().y, GetParent().Position().z);
            } else {
                SDL_Log("RaytracingModelComponent: Failed to add quad mesh");
            }
        } else {
            // 2回目以降は既存メッシュを更新
            bool success = rendererManager.GetRaytracingManager()->UpdateMesh(_meshId, transformedVertices, _indices);
            if (success) {
                SDL_Log("RaytracingModelComponent: Quad mesh ID=%d updated at position (%.2f, %.2f, %.2f)",
                    _meshId, GetParent().Position().x, GetParent().Position().y, GetParent().Position().z);
            } else {
                SDL_Log("RaytracingModelComponent: Failed to update quad mesh ID=%d", _meshId);
            }
        }
    } else {
        SDL_Log("RaytracingModelComponent: RaytracingManager not available");
    }
}

void RaytracingModelComponent::OnUpdateWorldTransform() {
    // GameObjectの位置が変わったらメッシュを更新
    if (_needsUpdate) {
        UpdateMesh();
        _needsUpdate = false; // フラグをリセット
    }
}

void RaytracingModelComponent::SetTexture(const std::string& filePath) {
    _texture = Texture2D::Get(filePath);
    
    if (_texture && _texture->IsValid()) {
        SDL_Log("RaytracingModelComponent: Texture loaded successfully: %s", filePath.c_str());
        
        // レイトレーシングマネージャーにテクスチャを設定
        auto& rendererManager = GAME.GetRendererManager();
        if (rendererManager.GetRaytracingManager()) {
            rendererManager.GetRaytracingManager()->SetTexture(_texture);
        }
    } else {
        SDL_Log("RaytracingModelComponent: Failed to load texture: %s", filePath.c_str());
    }
}
