#include "Resource/Model.h"
#include "DirectX/DirectX.h"
#include "DirectX/VertexBuffer.h"
#include "DirectX/IndexBuffer.h"
#include "DirectX/AssimpLoader.h"
#include "DirectX/Texture2D.h"
#include "DirectX/DescriptorHeap.h"
#include "Utility/Math.h"
#include <filesystem>

// ヘルパー関数（Scene.cppから移行）
std::wstring ReplaceExtension(const std::wstring& origin, const char* ext)
{
	std::filesystem::path p = origin.c_str();
	return p.replace_extension(ext).c_str();
}

Model::Model() : _descriptorHeap(nullptr), _isLoaded(false)
{
}

Model::~Model() {
	Unload();
}

bool Model::Load(const std::string& path) {
	// std::stringからstd::wstringに変換
	std::wstring wpath(path.begin(), path.end());
	
	LoadSettings settings;
	settings.filePath = wpath;
	settings.inverseU = false;
	settings.inverseV = true; // アリシアモデル用のデフォルト設定
	
	return LoadFromFile(settings);
}

bool Model::LoadFromFile(const LoadSettings& settings) {
	if (_isLoaded) {
		Unload(); // 既にロードされている場合はクリア
	}

	// ディスクリプタヒープの作成
	_descriptorHeap = new DescriptorHeap();
	if (!_descriptorHeap->IsValid()) {
		printf("Failed to create descriptor heap for model\n");
		return false;
	}

	// Assimpを使用してモデルをロード
	ImportSettings importSetting = {
		settings.filePath.c_str(),
		_meshes,
		settings.inverseU,
		settings.inverseV
	};

	AssimpLoader loader;
	if (!loader.Load(importSetting)) {
		printf("Failed to load model: %ls\n", settings.filePath.c_str());
		CleanupResources();
		return false;
	}

	// バッファとテクスチャの作成
	if (!CreateBuffers() || !LoadTextures()) {
		CleanupResources();
		return false;
	}

	_isLoaded = true;
	printf("Model loaded successfully: %ls\n", settings.filePath.c_str());
	printf("Number of meshes loaded: %zu\n", _meshes.size());
	
	return true;
}

bool Model::CreateBuffers() {
	// 頂点バッファの作成
	_vertexBuffers.reserve(_meshes.size());
	for (size_t i = 0; i < _meshes.size(); i++) {
		auto size = sizeof(Vertex) * _meshes[i].Vertices.size();
		auto stride = sizeof(Vertex);
		auto vertices = _meshes[i].Vertices.data();
		auto pVB = new VertexBuffer(size, stride, vertices);
		if (!pVB->IsValid()) {
			printf("頂点バッファの生成に失敗 (Mesh %zu)\n", i);
			return false;
		}
		_vertexBuffers.push_back(pVB);
	}

	// インデックスバッファの作成
	_indexBuffers.reserve(_meshes.size());
	for (size_t i = 0; i < _meshes.size(); i++) {
		auto size = sizeof(uint32_t) * _meshes[i].Indices.size();
		auto indices = _meshes[i].Indices.data();
		auto pIB = new IndexBuffer(size, indices);
		if (!pIB->IsValid()) {
			printf("インデックスバッファの生成に失敗 (Mesh %zu)\n", i);
			return false;
		}
		_indexBuffers.push_back(pIB);
	}

	return true;
}

bool Model::LoadTextures() {
	_materialHandles.clear();
	
	for (size_t i = 0; i < _meshes.size(); i++) {
		printf("Mesh %zu - Original DiffuseMap: %ls\n", i, _meshes[i].DiffuseMap.c_str());
		
		auto texturePath = ReplaceExtension(_meshes[i].DiffuseMap, ".tga");
		
		// デバッグ出力: テクスチャパスを確認
		wprintf(L"Loading texture: %s\n", texturePath.c_str());
		
		auto mainTexture = Texture2D::Get(texturePath);
		if (!mainTexture->IsValid()) {
			wprintf(L"Failed to load texture: %s\n", texturePath.c_str());
		}
		
		auto handle = _descriptorHeap->Register(mainTexture);
		_materialHandles.push_back(handle);
	}
	
	return true;
}

void Model::CleanupResources() {
	// 頂点バッファの削除
	for (auto* vb : _vertexBuffers) {
		delete vb;
	}
	_vertexBuffers.clear();

	// インデックスバッファの削除
	for (auto* ib : _indexBuffers) {
		delete ib;
	}
	_indexBuffers.clear();

	// マテリアルハンドルのクリア
	_materialHandles.clear();

	// ディスクリプタヒープの削除
	if (_descriptorHeap) {
		delete _descriptorHeap;
		_descriptorHeap = nullptr;
	}

	// メッシュデータのクリア
	_meshes.clear();
	
	_isLoaded = false;
}

bool Model::Unload() {
	if (_isLoaded) {
		CleanupResources();
		printf("Model unloaded successfully\n");
	}
	return true;
}