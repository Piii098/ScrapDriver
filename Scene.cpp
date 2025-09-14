#include "Scene.h"
#include "DirectX.h"
#include "VertexBuffer.h"
#include "ConstantBuffer.h"
#include "RootSignature.h"
#include "PipelineState.h"
#include "IndexBuffer.h"
#include "AssimpLoader.h"
#include "Texture2D.h"
#include <filesystem>

std::wstring ReplaceExtension(const std::wstring& origin, const char* ext)
{
	std::filesystem::path p = origin.c_str();
	return p.replace_extension(ext).c_str();
}


bool Scene::Initialize() {

	_descriptorHeap = new DescriptorHeap();
    if (!_descriptorHeap->IsValid()) {
        SDL_Log("Failed to create descriptor heap");
        return false;
    }

	ImportSettings importSetting = // これ自体は自作の読み込み設定構造体
	{
		L"Assets/Model/Alicia/FBX/Alicia_solid_Unity.FBX",
		_meshes,
		false,
		true // アリシアのモデルは、テクスチャのUVのVだけ反転してるっぽい？ので読み込み時にUV座標を逆転させる
	};

	AssimpLoader loader;
	if (!loader.Load(importSetting))
	{
		return false;
	}

	// メッシュの数だけ頂点バッファを用意する
	_vertexBuffers.reserve(_meshes.size());
	for (size_t i = 0; i < _meshes.size(); i++)
	{
		auto size = sizeof(Vertex) * _meshes[i].Vertices.size();
		auto stride = sizeof(Vertex);
		auto vertices = _meshes[i].Vertices.data();
		auto pVB = new VertexBuffer(size, stride, vertices);
		if (!pVB->IsValid())
		{
			printf("頂点バッファの生成に失敗\n");
			return false;
		}

		_vertexBuffers.push_back(pVB);
	}

	// メッシュの数だけインデックスバッファを用意する
	_indexBuffers.reserve(_meshes.size());
	for (size_t i = 0; i < _meshes.size(); i++)
	{
		auto size = sizeof(uint32_t) * _meshes[i].Indices.size();
		auto indices = _meshes[i].Indices.data();
		auto pIB = new IndexBuffer(size, indices);
		if (!pIB->IsValid())
		{
			printf("インデックスバッファの生成に失敗\n");
			return false;
		}

		_indexBuffers.push_back(pIB);
	}

	auto eyePos = XMVectorSet(0.0f, 120.0, 75.0, 0.0f); // 視点の位置
	auto targetPos = XMVectorSet(0.0f, 120.0, 0.0f, 0.0f); // 注視点の位置
	auto upward = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f); // 上方向を表すベクトル
	auto fov = XMConvertToRadians(60); // 視野角
	auto aspect = static_cast<float>(DIRECT3D.Width()) / static_cast<float>(DIRECT3D.Height()); // アスペクト比

	for (size_t i = 0; i < DIRECT3D.FRAME_BUFFER_COUNT; i++)
	{
		_constantBuffer[i] = new ConstantBuffer(sizeof(Transform));
		if (!_constantBuffer[i]->IsValid())
		{
			printf("変換行列用定数バッファの生成に失敗\n");
			return false;
		}

		// 変換行列の登録
		auto ptr = _constantBuffer[i]->GetPtr<Transform>();
		ptr->World = XMMatrixIdentity();
		ptr->View = XMMatrixLookAtRH(eyePos, targetPos, upward);
		ptr->Proj = XMMatrixPerspectiveFovRH(fov, aspect, 0.3f, 1000.0f);
	}

	_materialHandles.clear();
	printf("Number of meshes loaded: %zu\n", _meshes.size());
	
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

	_rootSignature = new RootSignature();
	if(!_rootSignature->IsValid()) {
		SDL_Log("Failed to create root signature");
		return false;
	}

	_pipelineState = new PipelineState();
	_pipelineState->SetInputLayout(Vertex::InputLayout);
	_pipelineState->SetRootSignature(_rootSignature->Get());
	_pipelineState->SetVS(L"bin/x64/Debug/SimpleVS.cso");
	_pipelineState->SetPS(L"bin/x64/Debug/SimplePS.cso");
	_pipelineState->Create();
	if (!_pipelineState->IsValid())
	{
		printf("パイプラインステートの生成に失敗\n");
		return false;
	}

	SDL_Log("Scene::Initialize()\n");
	return true;
}

void Scene::Update() {
	_rotationY += 0.01f;
	auto currentIndex = DIRECT3D.CurrentBackBufferIndex();
	auto currentTransform = _constantBuffer[currentIndex]->GetPtr<Transform>();
	currentTransform->World = XMMatrixRotationY(_rotationY);
}

void Scene::Render() {
	auto currentIndex = DIRECT3D.CurrentBackBufferIndex();
	auto& commandList = DIRECT3D.CommandList();
	auto materialHeap = _descriptorHeap->GetHeap(); // ディスクリプタヒープ

	for (size_t i = 0; i < _meshes.size(); i++)
	{
		auto vbView = _vertexBuffers[i]->View();
		auto ibView = _indexBuffers[i]->View();

		commandList.SetGraphicsRootSignature(_rootSignature->Get());
		commandList.SetPipelineState(_pipelineState->Get());
		commandList.SetGraphicsRootConstantBufferView(0, _constantBuffer[currentIndex]->GetAddress());

		commandList.IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		commandList.IASetVertexBuffers(0, 1, &vbView);
		commandList.IASetIndexBuffer(&ibView);

		commandList.SetDescriptorHeaps(1, &materialHeap); // 使用するディスクリプタヒープをセット
		commandList.SetGraphicsRootDescriptorTable(1, _materialHandles[i]->HandleGPU); // そのメッシュに対応するディスクリプタテーブルをセット

		commandList.DrawIndexedInstanced(_meshes[i].Indices.size(), 1, 0, 0, 0);
	}
}