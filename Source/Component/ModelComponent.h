#pragma once
#include "Core/Component.h"
#include "Framework.h"
#include "DirectX/ShaderStruct.h"

enum class ModelType {
	QUAD,        // 四角形プリミティブ
	SPHERE,      // 球プリミティブ
	MESH_FILE    // メッシュファイル（FBX等）
};

class ModelComponent : public Component {
public:
	ModelComponent(class GameObject& parent, int updateLayer = 100);
	~ModelComponent();
	
	// モデル読み込み
	bool LoadModel(const std::string& name);
	bool LoadQuad(); // 四角形メッシュを作成
	bool LoadSphere(float radius = 0.5f, int slices = 16, int stacks = 16); // 球メッシュを作成
	
	// レンダリング（ラスタライズ用）
	void Render() override;
	
	// レイトレーシング用のゲッター
	const std::vector<Mesh>& GetMeshes() const { return _meshes; }
	const std::vector<Vertex>& GetQuadVertices() const { return _quadVertices; }
	const std::vector<unsigned short>& GetQuadIndices() const { return _quadIndices; }
	ModelType GetModelType() const { return _modelType; }
	
	// テクスチャ・マテリアル設定
	void SetTexture(const std::string& filePath);
	void SetMaterial(const PerMeshMaterialData& material);
	const PerMeshMaterialData& GetMaterial() const { return _material; }
	class Texture2D* GetTexture() const { return _texture; }
	
	// ワールド変換更新通知
	virtual void OnUpdateWorldTransform() override;
	
	// レイトレーシング用メッシュID
	int GetRaytracingMeshId() const { return _raytracingMeshId; }
	void SetRaytracingMeshId(int id) { _raytracingMeshId = id; }
	void UpdateRaytracingMesh();
private:
	ModelType _modelType = ModelType::QUAD;
	
	// メッシュデータ
	std::vector<Mesh> _meshes;
	std::vector<Vertex> _quadVertices;  // Quad用頂点（ローカル座標）
	std::vector<unsigned short> _quadIndices; // Quad用インデックス
	
	// ラスタライズ用リソース
	std::vector<class VertexBuffer*> _vertexBuffers;
	std::vector<class IndexBuffer*> _indexBuffers;
	std::vector<class Texture2D*> _textures;
	std::vector<UINT> _textureIndices;
	
	// 共通リソース
	class Texture2D* _texture = nullptr;
	PerMeshMaterialData _material = {};
	
	// レイトレーシング用
	int _raytracingMeshId = -1;
	bool _needsRaytracingUpdate = false;
	
	// 内部ヘルパー
	void ComputeAutoUV(const std::vector<Vertex>& transformedVertices, PerMeshMaterialData& inOutMaterial);
};