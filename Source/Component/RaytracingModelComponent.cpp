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
    // 後段の自動UV推定用に保存
    _lastTransformedVertices = transformedVertices;
    
    // レイトレーシングマネージャーにメッシュを追加/更新
    auto& rendererManager = GAME.GetRendererManager();
    if (rendererManager.GetRaytracingManager()) {
        if (_meshId == -1) {
            // 初回は新規追加
            _meshId = rendererManager.GetRaytracingManager()->AddMesh(transformedVertices, _indices);
            if (_meshId >= 0) {
                SDL_Log("RaytracingModelComponent: Quad mesh added with ID=%d at position (%.2f, %.2f, %.2f)",
                    _meshId, GetParent().Position().x, GetParent().Position().y, GetParent().Position().z);
                    // マテリアル自動UV推定を適用
                    PerMeshMaterialData matToApply = {};
                    if (_hasPendingMaterial) {
                        matToApply = _pendingMaterial; // 色などはユーザー指定を優先
                        _hasPendingMaterial = false;
                    } else {
                        // デフォルトマテリアル（白）
                        matToApply.baseColor = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
                        matToApply.metallic = 0.0f; matToApply.roughness = 0.5f; matToApply.reflectance = 0.3f;
                        matToApply.materialType = 0; matToApply.uvScale = Vector2(1.0f, 1.0f);
                    }
                    ComputeAutoUV(_lastTransformedVertices, matToApply);
                    rendererManager.GetRaytracingManager()->SetMeshMaterial(_meshId, matToApply);
                    SDL_Log("RaytracingModelComponent: Auto UV material applied to mesh ID=%d", _meshId);
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

void RaytracingModelComponent::SetMaterial(const PerMeshMaterialData& material) {
    if (_meshId < 0) {
        // メッシュ未登録なので遅延適用
        _pendingMaterial = material;
        _hasPendingMaterial = true;
        SDL_Log("RaytracingModelComponent: Material deferred until mesh creation");
        return;
    }
    
    auto& rendererManager = GAME.GetRendererManager();
    if (rendererManager.GetRaytracingManager()) {
        // 現在のトランスフォームから自動UV推定を反映
        PerMeshMaterialData mat = material;
        if (!_lastTransformedVertices.empty()) {
            ComputeAutoUV(_lastTransformedVertices, mat);
        }
        rendererManager.GetRaytracingManager()->SetMeshMaterial(_meshId, mat);
        SDL_Log("RaytracingModelComponent: Material set for mesh ID=%d", _meshId);
    }
}

// 面が四角形である前提で、0,1,2の辺から面内2軸を推定
void RaytracingModelComponent::ComputeAutoUV(const std::vector<Vertex>& transformedVertices, PerMeshMaterialData& inOutMaterial)
{
    if (transformedVertices.size() < 3) return;

    const Vector3 p0 = transformedVertices[0].Position;
    const Vector3 p1 = transformedVertices[1].Position;
    const Vector3 p2 = transformedVertices[2].Position;

    // 初期辺ベクトル
    Vector3 edgeU = Vector3(XMVectorSubtract(XMVECTOR(p2), XMVECTOR(p0)));
    Vector3 edgeVraw = Vector3(XMVectorSubtract(XMVECTOR(p1), XMVECTOR(p0)));

    float w = edgeU.Length();
    float hraw = edgeVraw.Length();
    if (w < 1e-6f || hraw < 1e-6f) return;

    // 法線
    Vector3 n = edgeVraw.Cross(edgeU);
    n.Normalize();

    // U軸は edgeU の方向、V軸は法線とU軸の外積で面内で直交に作る
    Vector3 eU = edgeU;
    eU.Normalize();
    Vector3 eV = n.Cross(eU);
    eV.Normalize();

    // 実際のV範囲は edgeVraw を eV に投影した長さ
    float h = fabs(edgeVraw.Dot(eV));
    if (h < 1e-6f) return;

    // シェーダー側の式: uv = dot(P, axis)/range + offset
    // => P = p0 で 0 になるよう offset = -dot(p0, axis)/range
    float uOff = - eU.Dot(p0) / w;
    float vOff = - eV.Dot(p0) / h;

    inOutMaterial.uvAxis1 = eU;
    inOutMaterial.uvAxis2 = eV;
    inOutMaterial.uvRange1 = w;
    inOutMaterial.uvRange2 = h;
    inOutMaterial.uvOffset = Vector2(uOff, vOff);
    if (inOutMaterial.uvScale.x == 0 && inOutMaterial.uvScale.y == 0) {
        inOutMaterial.uvScale = Vector2(1.0f, 1.0f);
    }
}
