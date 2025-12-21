#include "Component/ModelComponent.h"
#include "DirectX/Direct3D.h"
#include "DirectX/VertexBuffer.h"
#include "DirectX/IndexBuffer.h"
#include "DirectX/Texture2D.h"
#include "DirectX/AssimpLoader.h"
#include "DirectX/DescriptorHeap.h"
#include "Renderer/RendererManager.h"
#include "Renderer/RaytracingManager.h"
#include "Core/GameObject.h"
#include "Core/Scene.h"
#include "Core/Game.h"
#include <filesystem>
#include <algorithm>
#include <cmath>
#include <limits>

namespace {

// 球メッシュをスライス×スタック分割で生成する
void GenerateSphereMesh(float radius, int slices, int stacks, std::vector<Vertex>& outVertices, std::vector<unsigned short>& outIndices) {
    slices = Math::Max(3, slices);
    stacks = Math::Max(2, stacks);

    const int vertCols = slices + 1;
    const int vertRows = stacks + 1;

    outVertices.clear();
    outIndices.clear();
    outVertices.reserve(static_cast<size_t>(vertCols) * vertRows);
    outIndices.reserve(static_cast<size_t>(slices) * stacks * 6);

    for (int stack = 0; stack <= stacks; ++stack) {
        float v = static_cast<float>(stack) / static_cast<float>(stacks);
        float phi = XM_PI * v;
        float sinPhi = sinf(phi);
        float cosPhi = cosf(phi);

        for (int slice = 0; slice <= slices; ++slice) {
            float u = static_cast<float>(slice) / static_cast<float>(slices);
            float theta = XM_2PI * u;
            float sinTheta = sinf(theta);
            float cosTheta = cosf(theta);

            Vector3 pos(radius * sinPhi * cosTheta,
                        radius * cosPhi,
                        radius * sinPhi * sinTheta);

            Vector3 normal = pos;
            normal.Normalize();

            Vector3 tangent(-sinTheta, 0.0f, cosTheta);
            float tangentLenSq = tangent.Dot(tangent);
            if (tangentLenSq < 1e-6f) {
                tangent = Vector3(1.0f, 0.0f, 0.0f);
            }
            tangent.Normalize();

            Vertex vtx = {};
            vtx.Position = pos;
            vtx.Normal = normal;
            vtx.Tangent = tangent;
            vtx.UV = Vector2(u, 1.0f - v);
            vtx.Color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);

            outVertices.push_back(vtx);
        }
    }

    // インデックス生成
    const size_t maxIndexValue = (std::numeric_limits<unsigned short>::max)();
    if (outVertices.size() > maxIndexValue) {
        SDL_Log("GenerateSphereMesh: vertex count exceeds 16-bit index limit");
        outVertices.clear();
        outIndices.clear();
        return;
    }

    for (int stack = 0; stack < stacks; ++stack) {
        for (int slice = 0; slice < slices; ++slice) {
            unsigned short i0 = static_cast<unsigned short>(stack * vertCols + slice);
            unsigned short i1 = static_cast<unsigned short>(i0 + 1);
            unsigned short i2 = static_cast<unsigned short>(i0 + vertCols);
            unsigned short i3 = static_cast<unsigned short>(i2 + 1);

            outIndices.push_back(i0);
            outIndices.push_back(i2);
            outIndices.push_back(i1);

            outIndices.push_back(i1);
            outIndices.push_back(i2);
            outIndices.push_back(i3);
        }
    }
}

void GenerateCubeMesh(float size, std::vector<Vertex>& outVertices, std::vector<unsigned short>& outIndices) {
    const float half = size * 0.5f;

    outVertices.clear();
    outIndices.clear();

    outVertices.reserve(24);
    outIndices.reserve(36);

    const Vector4 color(1.0f, 1.0f, 1.0f, 1.0f);

    auto addFace = [&](const Vector3& normal, const Vector3& tangent,
               const Vector3& p0, const Vector3& p1, const Vector3& p2, const Vector3& p3) {
    const unsigned short base = static_cast<unsigned short>(outVertices.size());

    Vertex v0 = {}; v0.Position = p0; v0.Normal = normal; v0.Tangent = tangent; v0.UV = Vector2(0.0f, 1.0f); v0.Color = color;
    Vertex v1 = {}; v1.Position = p1; v1.Normal = normal; v1.Tangent = tangent; v1.UV = Vector2(0.0f, 0.0f); v1.Color = color;
    Vertex v2 = {}; v2.Position = p2; v2.Normal = normal; v2.Tangent = tangent; v2.UV = Vector2(1.0f, 1.0f); v2.Color = color;
    Vertex v3 = {}; v3.Position = p3; v3.Normal = normal; v3.Tangent = tangent; v3.UV = Vector2(1.0f, 0.0f); v3.Color = color;

    outVertices.push_back(v0);
    outVertices.push_back(v1);
    outVertices.push_back(v2);
    outVertices.push_back(v3);

    // 0,1,2 / 2,1,3
    outIndices.push_back(base + 0);
    outIndices.push_back(base + 1);
    outIndices.push_back(base + 2);
    outIndices.push_back(base + 2);
    outIndices.push_back(base + 1);
    outIndices.push_back(base + 3);
    };

    // +Z (front)
    addFace(Vector3(0.0f, 0.0f, 1.0f), Vector3(1.0f, 0.0f, 0.0f),
        Vector3(-half, -half,  half), Vector3(-half,  half,  half),
        Vector3( half, -half,  half), Vector3( half,  half,  half));

    // -Z (back)
    addFace(Vector3(0.0f, 0.0f, -1.0f), Vector3(-1.0f, 0.0f, 0.0f),
        Vector3( half, -half, -half), Vector3( half,  half, -half),
        Vector3(-half, -half, -half), Vector3(-half,  half, -half));

    // +X (right)
    addFace(Vector3(1.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, -1.0f),
        Vector3( half, -half,  half), Vector3( half,  half,  half),
        Vector3( half, -half, -half), Vector3( half,  half, -half));

    // -X (left)
    addFace(Vector3(-1.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 1.0f),
        Vector3(-half, -half, -half), Vector3(-half,  half, -half),
        Vector3(-half, -half,  half), Vector3(-half,  half,  half));

    // +Y (top)
    addFace(Vector3(0.0f, 1.0f, 0.0f), Vector3(1.0f, 0.0f, 0.0f),
        Vector3(-half,  half,  half), Vector3(-half,  half, -half),
        Vector3( half,  half,  half), Vector3( half,  half, -half));

    // -Y (bottom)
    addFace(Vector3(0.0f, -1.0f, 0.0f), Vector3(1.0f, 0.0f, 0.0f),
        Vector3(-half, -half, -half), Vector3(-half, -half,  half),
        Vector3( half, -half, -half), Vector3( half, -half,  half));
}

} // namespace

ModelComponent::ModelComponent(class GameObject& parent, int updateLayer)
    : Component(parent, updateLayer) {
    
    // デフォルトマテリアル設定
    _material.baseColor = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
    _material.metallic = 0.0f;
    _material.roughness = 0.5f;
    _material.reflectance = 0.3f;
    _material.materialType = 0;
    _material.uvScale = Vector2(1.0f, 1.0f);
    
    // RendererManagerに登録
    RendererManager& renderer = GAME.GetRendererManager();

    renderer.RegisterModel(this);
    
}

ModelComponent::~ModelComponent() {
    // RendererManagerから登録解除
    RendererManager& renderer = GAME.GetRendererManager();
    renderer.UnregisterModel(this);
    
    // ラスタライズ用バッファの解放
    for (auto vb : _vertexBuffers) delete vb;
    for (auto ib : _indexBuffers) delete ib;
    for (auto tex : _textures) delete tex;
    _vertexBuffers.clear();
    _indexBuffers.clear();
    _textures.clear();
    
    if (_texture) delete _texture;
}

std::wstring ReplaceExtension(const std::wstring& origin, const char* ext) {
    std::filesystem::path p = origin.c_str();
    return p.replace_extension(ext).c_str();
}

bool ModelComponent::LoadModel(const std::string& name) {
    _modelType = ModelType::MESH_FILE;
    
    ImportSettings importSetting = {
        std::wstring(name.begin(), name.end()).c_str(),
        _meshes,
        false,
        true
    };

    AssimpLoader loader;
    if (!loader.Load(importSetting)) {
        SDL_Log("Failed to load model: %s", name.c_str());
        return false;
    }

    // ラスタライズ用バッファ作成
    bool allBuffersCreated = true;

    _vertexBuffers.reserve(_meshes.size());
    for (size_t i = 0; i < _meshes.size(); ++i) {
        auto vb = new VertexBuffer(_meshes[i].Vertices.data(), _meshes[i].Vertices.size());
        if (!vb->IsValid()) {
            SDL_Log("Failed to create vertex buffer at index %zu", i);
            delete vb;
            allBuffersCreated = false;
            break;
        }
        _vertexBuffers.push_back(vb);
    }

    if (!allBuffersCreated) {
        for (auto vb : _vertexBuffers) delete vb;
        _vertexBuffers.clear();
        _meshes.clear();
        return false;
    }

    _indexBuffers.reserve(_meshes.size());
    for (size_t i = 0; i < _meshes.size(); ++i) {
        auto pIB = new IndexBuffer(_meshes[i].Indices.data(), _meshes[i].Indices.size());
        if (!pIB->IsValid()) {
            SDL_Log("Failed to create index buffer at index %zu", i);
            delete pIB;
            allBuffersCreated = false;
            break;
        }
        _indexBuffers.push_back(pIB);
    }

    // テクスチャ読み込み
    _textures.reserve(_meshes.size());
    _textureIndices.reserve(_meshes.size());

    for (size_t i = 0; i < _meshes.size(); ++i) {
        auto texPath = ReplaceExtension(_meshes[i].DiffuseMap, "tga");
        auto mainTex = Texture2D::Get(texPath);

        if (mainTex && mainTex->IsValid()) {
            _textures.push_back(mainTex);
            _textureIndices.push_back(mainTex->GetDescriptorIndex());
        } else {
            auto whiteTex = Texture2D::GetWhite();
            _textures.push_back(whiteTex);
            _textureIndices.push_back(whiteTex->GetDescriptorIndex());
        }
    }

    if (!allBuffersCreated) {
        for (auto vb : _vertexBuffers) delete vb;
        for (auto ib : _indexBuffers) delete ib;
        _vertexBuffers.clear();
        _indexBuffers.clear();
        _meshes.clear();
        return false;
    }

    _needsRaytracingUpdate = true;
    return true;
}

bool ModelComponent::LoadQuad() {
    _modelType = ModelType::QUAD;
    
    // 四角形の頂点データを作成（ローカル座標）
    Vertex vertices[4] = {};
    vertices[0].Position = { -0.5f, -0.5f, 0.0f }; vertices[0].UV = { 0.0f, 1.0f };
    vertices[1].Position = { -0.5f,  0.5f, 0.0f }; vertices[1].UV = { 0.0f, 0.0f };
    vertices[2].Position = {  0.5f, -0.5f, 0.0f }; vertices[2].UV = { 1.0f, 1.0f };
    vertices[3].Position = {  0.5f,  0.5f, 0.0f }; vertices[3].UV = { 1.0f, 0.0f };
    
    // インデックスデータ
    unsigned short indices[] = {
        0, 1, 2,
        2, 1, 3
    };
    
    _quadVertices = std::vector<Vertex>(std::begin(vertices), std::end(vertices));
    _quadIndices = std::vector<unsigned short>(std::begin(indices), std::end(indices));
    
    // ラスタライズ用バッファ作成
    auto vb = new VertexBuffer(_quadVertices.data(), _quadVertices.size());
    if (vb->IsValid()) {
        _vertexBuffers.push_back(vb);
    } else {
        delete vb;
        return false;
    }
    
    auto ib = new IndexBuffer(_quadIndices.data(), _quadIndices.size());
    if (ib->IsValid()) {
        _indexBuffers.push_back(ib);
    } else {
        delete ib;
        return false;
    }
    
    _needsRaytracingUpdate = true;
    return true;
}

bool ModelComponent::LoadCube(float size) {
    _modelType = ModelType::CUBE;

    for (auto vb : _vertexBuffers) delete vb;
    for (auto ib : _indexBuffers) delete ib;
    for (auto tex : _textures) delete tex;
    _vertexBuffers.clear();
    _indexBuffers.clear();
    _textures.clear();
    _textureIndices.clear();
    _meshes.clear();
    _quadVertices.clear();
    _quadIndices.clear();

    std::vector<Vertex> vertices;
    std::vector<unsigned short> indices;
    GenerateCubeMesh(size, vertices, indices);

    if (vertices.empty() || indices.empty()) {
        SDL_Log("ModelComponent: Failed to generate cube mesh");
        return false;
    }

    Mesh cubeMesh = {};
    cubeMesh.Vertices = std::move(vertices);
    cubeMesh.Indices = std::move(indices);
    _meshes.push_back(std::move(cubeMesh));

    auto vb = new VertexBuffer(_meshes[0].Vertices.data(), _meshes[0].Vertices.size());
    if (!vb->IsValid()) {
        delete vb;
        _meshes.clear();
        return false;
    }
    _vertexBuffers.push_back(vb);

    auto ib = new IndexBuffer(_meshes[0].Indices.data(), _meshes[0].Indices.size());
    if (!ib->IsValid()) {
        delete ib;
        delete _vertexBuffers.back();
        _vertexBuffers.clear();
        _meshes.clear();
        return false;
    }
    _indexBuffers.push_back(ib);

    Texture2D* mainTex = nullptr;
    if (_texture && _texture->IsValid()) {
        mainTex = _texture;
    } else {
        mainTex = Texture2D::GetWhite();
    }

    if (mainTex) {
        _textures.push_back(mainTex);
        _textureIndices.push_back(mainTex->GetDescriptorIndex());
    }

    _needsRaytracingUpdate = true;
    return true;
}

bool ModelComponent::LoadSphere(float radius, int slices, int stacks) {
    _modelType = ModelType::SPHERE;

    for (auto vb : _vertexBuffers) delete vb;
    for (auto ib : _indexBuffers) delete ib;
    for (auto tex : _textures) delete tex;
    _vertexBuffers.clear();
    _indexBuffers.clear();
    _textures.clear();
    _textureIndices.clear();
    _meshes.clear();
    _quadVertices.clear();
    _quadIndices.clear();

    std::vector<Vertex> vertices;
    std::vector<unsigned short> indices;
    GenerateSphereMesh(radius, slices, stacks, vertices, indices);

    if (vertices.empty() || indices.empty()) {
        SDL_Log("ModelComponent: Failed to generate sphere mesh");
        return false;
    }

    Mesh sphereMesh = {};
    sphereMesh.Vertices = std::move(vertices);
    sphereMesh.Indices = std::move(indices);
    _meshes.push_back(std::move(sphereMesh));

    auto vb = new VertexBuffer(_meshes[0].Vertices.data(), _meshes[0].Vertices.size());
    if (!vb->IsValid()) {
        delete vb;
        _meshes.clear();
        return false;
    }
    _vertexBuffers.push_back(vb);

    auto ib = new IndexBuffer(_meshes[0].Indices.data(), _meshes[0].Indices.size());
    if (!ib->IsValid()) {
        delete ib;
        delete _vertexBuffers.back();
        _vertexBuffers.clear();
        _meshes.clear();
        return false;
    }
    _indexBuffers.push_back(ib);

    Texture2D* mainTex = nullptr;
    if (_texture && _texture->IsValid()) {
        mainTex = _texture;
    } else {
        mainTex = Texture2D::GetWhite();
    }

    if (mainTex) {
        _textures.push_back(mainTex);
        _textureIndices.push_back(mainTex->GetDescriptorIndex());
    }

    _needsRaytracingUpdate = true;
    return true;
}

void ModelComponent::SetTexture(const std::string& filePath) {
    Texture2D* loaded = Texture2D::Get(filePath);
    if (loaded && loaded->IsValid()) {
        SDL_Log("ModelComponent: Texture loaded successfully: %s", filePath.c_str());
        _texture = loaded;
    } else {
        SDL_Log("ModelComponent: Failed to load texture: %s", filePath.c_str());
        _texture = Texture2D::GetWhite();
    }

    Texture2D* effectiveTex = _texture;
    if (effectiveTex && effectiveTex->IsValid()) {
        if (_textures.empty()) {
            _textures.push_back(effectiveTex);
            _textureIndices.push_back(effectiveTex->GetDescriptorIndex());
        } else {
            _textures[0] = effectiveTex;
            if (_textureIndices.empty()) {
                _textureIndices.push_back(effectiveTex->GetDescriptorIndex());
            } else {
                _textureIndices[0] = effectiveTex->GetDescriptorIndex();
            }
        }
    }
}

void ModelComponent::SetMaterial(const PerMeshMaterialData& material) {
    _material = material;
    
    // レイトレーシング側にも反映
    if (_raytracingMeshId >= 0) {
        auto& rendererManager = GAME.GetRendererManager();
        if (rendererManager.GetRaytracingManager()) {
            PerMeshMaterialData mat = _material;
            // 自動UV推定は必要に応じて
            rendererManager.GetRaytracingManager()->SetMeshMaterial(_raytracingMeshId, mat);
        }
    }
}

void ModelComponent::OnUpdateWorldTransform() {
    // ワールド変換が変わったらフラグを立てる
    _needsRaytracingUpdate = true;
}

void ModelComponent::UpdateRaytracingMesh() {
    if (!_needsRaytracingUpdate) return;
    
    RendererManager& renderer = GAME.GetRendererManager();
    if (!renderer.GetRaytracingManager()) return;
    
    const auto& worldTransform = GetParent().WorldTransform();
    
    if (_modelType == ModelType::QUAD) {
        // Quad用の処理
        std::vector<Vertex> transformedVertices = _quadVertices;
        for (auto& vertex : transformedVertices) {
            Vector3 worldPos = Vector3::Transform(vertex.Position, worldTransform);
            vertex.Position = worldPos;
        }
        
        if (_raytracingMeshId == -1) {
            // 新規追加
            _raytracingMeshId = renderer.GetRaytracingManager()->AddMesh(transformedVertices, _quadIndices);
            
            if (_raytracingMeshId >= 0) {
                // マテリアル設定（自動UV推定込み）
                PerMeshMaterialData mat = _material;
                ComputeAutoUV(transformedVertices, mat);
                
                // レイトレーシング用テクスチャインデックスを設定
                if (_texture && _texture->IsValid() && _texture->HasRaytracingIndex()) {
                    mat.textureIndex = _texture->GetRaytracingIndex();
                } else {
                    mat.textureIndex = static_cast<unsigned int>(-1); // 無効な値
                }
                
                renderer.GetRaytracingManager()->SetMeshMaterial(_raytracingMeshId, mat);
                
                // メッシュごとのテクスチャ設定
                if (_texture) {
                    renderer.GetRaytracingManager()->SetMeshTexture(_raytracingMeshId, _texture);
                }
            }
        } else {
            // 更新
            renderer.GetRaytracingManager()->UpdateMesh(_raytracingMeshId, transformedVertices, _quadIndices);
            
            // マテリアルとテクスチャも更新
            PerMeshMaterialData mat = _material;
            ComputeAutoUV(transformedVertices, mat);
            
            if (_texture && _texture->IsValid() && _texture->HasRaytracingIndex()) {
                mat.textureIndex = _texture->GetRaytracingIndex();
            } else {
                mat.textureIndex = static_cast<unsigned int>(-1);
            }
            
            renderer.GetRaytracingManager()->SetMeshMaterial(_raytracingMeshId, mat);
            
            if (_texture) {
                renderer.GetRaytracingManager()->SetMeshTexture(_raytracingMeshId, _texture);
            }
        }
    } else if (_modelType == ModelType::MESH_FILE || _modelType == ModelType::SPHERE || _modelType == ModelType::CUBE) {
        // メッシュファイル/生成メッシュ用の処理（TODO: 複数メッシュ対応）
        // 現状は最初のメッシュのみ
        if (!_meshes.empty()) {
            std::vector<Vertex> transformedVertices = _meshes[0].Vertices;
            for (auto& vertex : transformedVertices) {
                Vector3 worldPos = Vector3::Transform(vertex.Position, worldTransform);
                vertex.Position = worldPos;
            }
            
            if (_raytracingMeshId == -1) {
                _raytracingMeshId = renderer.GetRaytracingManager()->AddMesh(
                    transformedVertices, _meshes[0].Indices);
                
                if (_raytracingMeshId >= 0) {
                    // マテリアル設定
                    PerMeshMaterialData mat = _material;
                    if (_texture && _texture->IsValid() && _texture->HasRaytracingIndex()) {
                        mat.textureIndex = _texture->GetRaytracingIndex();
                    } else {
                        mat.textureIndex = static_cast<unsigned int>(-1);
                    }
                    
                    renderer.GetRaytracingManager()->SetMeshMaterial(_raytracingMeshId, mat);
                    
                    if (_texture) {
                        renderer.GetRaytracingManager()->SetMeshTexture(_raytracingMeshId, _texture);
                    }
                }
            } else {
                renderer.GetRaytracingManager()->UpdateMesh(
                    _raytracingMeshId, transformedVertices, _meshes[0].Indices);
                
                // マテリアルとテクスチャも更新
                PerMeshMaterialData mat = _material;
                if (_texture && _texture->IsValid() && _texture->HasRaytracingIndex()) {
                    mat.textureIndex = _texture->GetRaytracingIndex();
                } else {
                    mat.textureIndex = static_cast<unsigned int>(-1);
                }
                
                renderer.GetRaytracingManager()->SetMeshMaterial(_raytracingMeshId, mat);
                
                if (_texture) {
                    renderer.GetRaytracingManager()->SetMeshTexture(_raytracingMeshId, _texture);
                }
            }
        }
    }
    
    _needsRaytracingUpdate = false;
}

void ModelComponent::ComputeAutoUV(const std::vector<Vertex>& transformedVertices, PerMeshMaterialData& inOutMaterial) {
    if (transformedVertices.size() < 3) return;

    const Vector3 p0 = transformedVertices[0].Position;
    const Vector3 p1 = transformedVertices[1].Position;
    const Vector3 p2 = transformedVertices[2].Position;

    Vector3 edgeU = Vector3(XMVectorSubtract(XMVECTOR(p2), XMVECTOR(p0)));
    Vector3 edgeVraw = Vector3(XMVectorSubtract(XMVECTOR(p1), XMVECTOR(p0)));

    float w = edgeU.Length();
    float hraw = edgeVraw.Length();
    if (w < 1e-6f || hraw < 1e-6f) return;

    Vector3 n = edgeVraw.Cross(edgeU);
    n.Normalize();

    Vector3 eU = edgeU;
    eU.Normalize();
    Vector3 eV = n.Cross(eU);
    eV.Normalize();

    float h = fabs(edgeVraw.Dot(eV));
    if (h < 1e-6f) return;

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

void ModelComponent::Render() {
    auto& commandList = DIRECT3D.CommandList();

    if (_vertexBuffers.empty() || _indexBuffers.empty()) {
        return;
    }

    // ワールド行列を設定（b1 RootConstants に送る）
    const Matrix4x4& worldTransform = GetParent().WorldTransform();
    // 32bit定数としてセット（行列=16要素）
    commandList.SetGraphicsRoot32BitConstants(2, 16, &worldTransform, 0);

    // シーン定数（b0のCBV）を毎ドロー前に再バインド（保守的）
    D3D12_GPU_DESCRIPTOR_HANDLE cbvHandle = DESCRIPTORMANAGER.GetGPUHandleAt(
        GAME.GetRendererManager().GetConstantBufferIndex()
    );
    commandList.SetGraphicsRootDescriptorTable(1, cbvHandle);

    // Quadモードの場合
    if (_modelType == ModelType::QUAD) {
        commandList.IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        
        if (_texture && _texture->IsValid()) {
            _texture->Bind();
        } else {
            auto whiteTex = Texture2D::GetWhite();
            if (whiteTex) whiteTex->Bind();
        }
        
        _vertexBuffers[0]->Bind();
        _indexBuffers[0]->Bind();
        commandList.DrawIndexedInstanced(static_cast<UINT>(_quadIndices.size()), 1, 0, 0, 0);
        return;
    }

    // メッシュファイルモード
    if (_meshes.empty() || _textures.empty()) {
        return;
    }

    for (size_t i = 0; i < _meshes.size(); ++i) {
        if (!_vertexBuffers[i] || !_indexBuffers[i] || !_textures[i]) {
            continue;
        }

        commandList.IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        _textures[i]->Bind();
        _vertexBuffers[i]->Bind();
        _indexBuffers[i]->Bind();
        commandList.DrawIndexedInstanced(static_cast<UINT>(_meshes[i].Indices.size()), 1, 0, 0, 0);
    }
}
