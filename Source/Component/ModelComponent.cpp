#include "Component/ModelComponent.h"
#include "DirectX/Direct3D.h"
#include "Resource/AssetManager.h"
#include "Resource/Model.h"
#include "Core/GameObject.h"
#include "Core/Scene.h"
#include "DirectX/AssimpLoader.h"
#include "DirectX/VertexBuffer.h"
#include "DirectX/IndexBuffer.h"
#include "DirectX/Texture2D.h"
#include <filesystem>

std::wstring ReplaceExtension(const std::wstring& origin, const char* ext)
{
    std::filesystem::path p = origin.c_str();
    return p.replace_extension(ext).c_str();
}

ModelComponent::ModelComponent(class GameObject& parent, int updateLayer)
    : Component(parent, updateLayer) {

    ImportSettings importSetting = // ���ꎩ�͎̂���̓ǂݍ��ݐݒ�\����
    {
        L"Assets/Model/Alicia/FBX/Alicia_solid_MMD.FBX",
        _meshes,
        false,
        true
    };

    AssimpLoader loader;
    if (!loader.Load(importSetting))
    {
        return;
    }

    //Vertex vertices[4] = {};
    //vertices[0].Position = { -1.f,-1.f,0.0f }; vertices[0].UV = { 0.0f,1.0f };
    //vertices[1].Position = { -1.f, 1.f,0.0f }; vertices[1].UV = { 0.0f,0.0f };
    //vertices[2].Position = { 1.f,-1.f,0.0f }; vertices[2].UV = { 1.0f,1.0f };
    //vertices[3].Position = { 1.f, 1.f,0.0f }; vertices[3].UV = { 1.0f,0.0f };

    //unsigned short indices[] = {
    //    0,1,2,
    //    2,1,3
    //};

    //Mesh mesh = {};
    //mesh.Vertices = std::vector<Vertex>(std::begin(vertices), std::end(vertices));
    //mesh.Indices = std::vector<unsigned short>(std::begin(indices), std::end(indices));
    //_meshes.clear();
    //_meshes.shrink_to_fit();
    //_meshes.push_back(mesh);

    // �o�b�t�@�쐬�̐���/���s��ǐ�
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
        // �N���[���A�b�v
        for (auto vb : _vertexBuffers) {
            delete vb;
        }
        _vertexBuffers.clear();
        _meshes.clear();
        return;
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

    _textures.reserve(_meshes.size());
    _textureIndices.reserve(_meshes.size());

    for (size_t i = 0; i < _meshes.size(); ++i) {
        auto texPath = ReplaceExtension(_meshes[i].DiffuseMap, "tga");
        auto mainTex = Texture2D::Get(texPath);

        if (mainTex && mainTex->IsValid()) {
            _textures.push_back(mainTex);
            _textureIndices.push_back(mainTex->GetDescriptorIndex());
            SDL_Log("Loaded texture for mesh %zu: index %u", i, mainTex->GetDescriptorIndex());
        }
        else {
            // テクスチャが見つからない場合は白テクスチャを使用
            auto whiteTex = Texture2D::GetWhite();
            _textures.push_back(whiteTex);
            _textureIndices.push_back(whiteTex->GetDescriptorIndex());
            SDL_Log("Using white texture for mesh %zu", i);
        }
    }

    if (!allBuffersCreated) {
        // �N���[���A�b�v
        for (auto vb : _vertexBuffers) delete vb;
        for (auto ib : _indexBuffers) delete ib;
        _vertexBuffers.clear();
        _indexBuffers.clear();
        _meshes.clear();
        return;
    }
}

ModelComponent::~ModelComponent() {

}

bool ModelComponent::LoadModel(const std::string& name) {
	//_model = ASSETS.Models().Get(name);

	return true;
}

void ModelComponent::Render() {
    auto& commandList = DIRECT3D.CommandList();

    // 範囲チェックを追加
    if (_meshes.empty() || _vertexBuffers.empty() || _indexBuffers.empty()) {
        SDL_Log("Error: No meshes or buffers to render");
        return;
    }

    // サイズの整合性チェック
    if (_meshes.size() != _vertexBuffers.size() ||
        _meshes.size() != _indexBuffers.size() ||
        _meshes.size() != _textures.size()) {
        SDL_Log("Error: Mesh count mismatch - meshes: %zu, VB: %zu, IB: %zu, Textures: %zu",
            _meshes.size(), _vertexBuffers.size(), _indexBuffers.size(), _textures.size());
        return;
    }

    for (size_t i = 0; i < _meshes.size(); ++i) {
        // nullチェック
        if (!_vertexBuffers[i] || !_indexBuffers[i] || !_textures[i]) {
            SDL_Log("Error: Invalid buffer or texture at index %zu", i);
            continue;
        }

        DIRECT3D.CommandList().IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        // テクスチャをバインド（ルートパラメータ0）
        _textures[i]->Bind();

        // 頂点バッファとインデックスバッファをバインド
        _vertexBuffers[i]->Bind();
        _indexBuffers[i]->Bind();

        commandList.DrawIndexedInstanced(static_cast<UINT>(_meshes[i].Indices.size()), 1, 0, 0, 0);
    }
}