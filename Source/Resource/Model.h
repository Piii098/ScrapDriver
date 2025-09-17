#pragma once
#include "Resource/Asset.h"
#include "DirectX/ShaderStruct.h"
#include "DirectX/DescriptorHeap.h"
#include <vector>
#include <string>

class Model : public Asset
{
public:
	Model();
	~Model();
	
	bool Load(const std::string& path) override;
	bool Unload() override;
	
	// モデル読み込み用の設定構造体
	struct LoadSettings {
		std::wstring filePath;
		bool inverseU = false;
		bool inverseV = false;
	};
	
	bool LoadFromFile(const LoadSettings& settings);
	
	// ゲッター
	const std::vector<Mesh>& GetMeshes() const { return _meshes; }
	const std::vector<class VertexBuffer*>& GetVertexBuffers() const { return _vertexBuffers; }
	const std::vector<class IndexBuffer*>& GetIndexBuffers() const { return _indexBuffers; }
	const std::vector<DescriptorHandle*>& GetMaterialHandles() const { return _materialHandles; }
	class DescriptorHeap* GetDescriptorHeap() const { return _descriptorHeap; }
	
	bool IsLoaded() const { return _isLoaded; }

private:
	// モデルデータ
	std::vector<Mesh> _meshes;
	std::vector<class VertexBuffer*> _vertexBuffers;
	std::vector<class IndexBuffer*> _indexBuffers;
	class DescriptorHeap* _descriptorHeap;
	std::vector<DescriptorHandle*> _materialHandles;
	
	bool _isLoaded = false;
	
	// 内部処理用
	bool CreateBuffers();
	bool LoadTextures();
	void CleanupResources();
};
