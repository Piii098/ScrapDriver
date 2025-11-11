#include "Renderer/RaytracingManager.h"
#include "DirectX/RaytracingRootSignature.h"
#include "DirectX/DescriptorHeap.h"
#include "DirectX/Texture2D.h"
#include "Component/CameraComponent.h"

RaytracingManager::RaytracingManager() : _isValid(false) {
}

RaytracingManager::~RaytracingManager() {
    if (_cameraBuffer && _cameraBufferMapped) {
        _cameraBuffer->Unmap(0, nullptr);
        _cameraBufferMapped = nullptr;
    }
    // Material Bufferのアンマップを追加
    if (_materialBuffer && _materialBufferMapped) {
        _materialBuffer->Unmap(0, nullptr);
        _materialBufferMapped = nullptr;
    }
}

bool RaytracingManager::Initialize() {
    // DXR�������`�F�b�N
    /*if (!DIRECT3D.DXRDevice() || !DIRECT3D.DXRCommandList()) {
        SDL_Log("DXR not initialized");
        return false;
    }*/

    // ���[�g�V�O�l�`���쐬
    if (!CreateRootSignatures()) {
        SDL_Log("Failed to create root signatures");
        return false;
    }

    // �p�C�v���C���쐬
    if (!CreatePipeline()) {
        SDL_Log("Failed to create pipeline");
        return false;
    }

    // �V�F�[�_�[�e�[�u���쐬
    if (!CreateShaderTable()) {
        SDL_Log("Failed to create shader table");
        return false;
    }

    // �o�̓��\�[�X�쐬
    if (!CreateOutput()) {
        SDL_Log("Failed to create output");
        return false;
    }

    // カメラ用Constant Buffer作成
    if (!CreateCameraBuffer()) {
        SDL_Log("Failed to create camera buffer");
        return false;
    }

    // Material用Constant Buffer作成
    if (!CreateMaterialBuffer()) {
        SDL_Log("Failed to create material buffer");
        return false;
    }

    _isValid = true;
    SDL_Log("RaytracingManager initialized successfully");
    return true;
}

bool RaytracingManager::CreateRootSignatures() {
    _rootSignature = std::make_unique<RaytracingRootSignature>();

    if (!_rootSignature->CreateGlobalRootSignature()) {
		SDL_Log("Failed to create global root signature");
        return false;
    }

    if (!_rootSignature->CreateLocalRootSignature()) {
		SDL_Log("Failed to create local root signature");
        return false;
    }

    return true;
}

bool RaytracingManager::CreatePipeline() {
    _pipeline = std::make_unique<RaytracingPipeline>();

    return _pipeline->Initialize(
        _rootSignature->GlobalRootSignature(),
        _rootSignature->LocalRootSignature());
}

bool RaytracingManager::CreateShaderTable() {
    _shaderTable = std::make_unique<ShaderTable>();

    return _shaderTable->Initialize(_pipeline->GetStateObjectProps());
}

bool RaytracingManager::CreateOutput() {
    _output = std::make_unique<RaytracingOutput>();

    return _output->Initialize(
        DIRECT3D.Width(),
        DIRECT3D.Height());
}

bool RaytracingManager::CreateCameraBuffer() {
    // Constant Buffer用のヒーププロパティ
    auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    
    // 256バイトアラインメント
    UINT bufferSize = (sizeof(RaytracingCameraData) + 255) & ~255;
    auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);
    
    HRESULT hr = DIRECT3D.Device().CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&_cameraBuffer));
    
    if (FAILED(hr)) {
        SDL_Log("Failed to create camera constant buffer");
        return false;
    }
    
    // 永続的にマップ
    hr = _cameraBuffer->Map(0, nullptr, &_cameraBufferMapped);
    if (FAILED(hr)) {
        SDL_Log("Failed to map camera constant buffer");
        return false;
    }
    
    SDL_Log("Camera constant buffer created successfully");
    return true;
}

bool RaytracingManager::CreateMaterialBuffer() {
    // Constant Buffer用のヒーププロパティ
    auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    
    // 256バイトアラインメント
    UINT bufferSize = (sizeof(RaytracingMaterialData) + 255) & ~255;
    auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);
    
    HRESULT hr = DIRECT3D.Device().CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&_materialBuffer));
    
    if (FAILED(hr)) {
        SDL_Log("Failed to create material constant buffer");
        return false;
    }
    
    // 永続的にマップ
    hr = _materialBuffer->Map(0, nullptr, &_materialBufferMapped);
    if (FAILED(hr)) {
        SDL_Log("Failed to map material constant buffer");
        return false;
    }
    
    SDL_Log("Material constant buffer created successfully");
    return true;
}

void RaytracingManager::UpdateCameraBuffer() {
    CameraComponent* activeCamera = CameraComponent::GetActiveCamera();
    
    RaytracingCameraData cameraData = {};
    
    if (activeCamera) {
        // アクティブなカメラから情報を取得
        cameraData.Position = activeCamera->GetPosition();
        cameraData.Forward = activeCamera->GetForward();
        cameraData.Right = activeCamera->GetRight();
        cameraData.Up = activeCamera->GetUp();
        cameraData.FOV = Math::ToRadians(activeCamera->GetFOV());
        cameraData.AspectRatio = static_cast<float>(DIRECT3D.Width()) / static_cast<float>(DIRECT3D.Height());
    } else {
        // デフォルトカメラ（原点から前を向く）
        cameraData.Position = Vector3(0.0f, 0.0f, -2.0f);
        cameraData.Forward = Vector3(0.0f, 0.0f, 1.0f);
        cameraData.Right = Vector3(1.0f, 0.0f, 0.0f);
        cameraData.Up = Vector3(0.0f, 1.0f, 0.0f);
        cameraData.FOV = Math::ToRadians(60.0f);
        cameraData.AspectRatio = static_cast<float>(DIRECT3D.Width()) / static_cast<float>(DIRECT3D.Height());
    }
    
    // Constant Bufferにデータをコピー
    memcpy(_cameraBufferMapped, &cameraData, sizeof(RaytracingCameraData));
}

void RaytracingManager::UpdateMaterialBuffer() {
    RaytracingMaterialData materialData = {};
    
    // より現実的な値に調整
    materialData.reflectanceCoef = 0.3f;  // 0.9 → 0.5に変更
    materialData.diffuseCoef = 0.9f;      // 0.3 → 0.5に変更（reflectanceCoefと足して1.0）
    materialData.specularCoef = 0.8f;     // 0.8 → 0.3に変更
    materialData.specularPower = 64.0f;
    materialData.lightPosition = Vector3(2.0f, 5.0f, -3.0f);
    materialData.lightColor = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
    
    // Constant Bufferにデータをコピー
    memcpy(_materialBufferMapped, &materialData, sizeof(RaytracingMaterialData));
}

bool RaytracingManager::AddTriangle(float* vertices, int vertexCount) {
    _accelerationStructure = std::make_unique<AccelerationStructure>();

    return _accelerationStructure->BuildSimple(vertices, vertexCount);
}

int RaytracingManager::AddMesh(const std::vector<Vertex>& vertices, const std::vector<unsigned short>& indices) {
    // 頂点データをfloat配列に変換（位置のみ）
    std::vector<float> meshVertices;
    meshVertices.reserve(indices.size() * 3); // 各インデックスから3つのfloat（xyz）
    
    for (unsigned short index : indices) {
        if (index < vertices.size()) {
            meshVertices.push_back(vertices[index].Position.x);
            meshVertices.push_back(vertices[index].Position.y);
            meshVertices.push_back(vertices[index].Position.z);
        }
    }
    
    // 新しいメッシュIDを生成
    int meshId = static_cast<int>(_meshVertices.size());
    
    SDL_Log("Adding mesh to raytracing: ID=%d, %zu vertices, %zu indices -> %zu triangle vertices",
        meshId, vertices.size(), indices.size(), meshVertices.size() / 3);
    
    // メッシュデータを保持（遅延構築用）
    _meshVertices.push_back(meshVertices);
    _needsRebuild = true;
    
    SDL_Log("Mesh data stored with ID=%d, will build AS on first render frame", meshId);
    return meshId;
}

bool RaytracingManager::UpdateMesh(int meshId, const std::vector<Vertex>& vertices, const std::vector<unsigned short>& indices) {
    // メッシュIDが有効範囲内かチェック
    if (meshId < 0 || meshId >= static_cast<int>(_meshVertices.size())) {
        SDL_Log("Invalid mesh ID: %d", meshId);
        return false;
    }
    
    // 頂点データをfloat配列に変換（位置のみ）
    std::vector<float> meshVertices;
    meshVertices.reserve(indices.size() * 3);
    
    for (unsigned short index : indices) {
        if (index < vertices.size()) {
            meshVertices.push_back(vertices[index].Position.x);
            meshVertices.push_back(vertices[index].Position.y);
            meshVertices.push_back(vertices[index].Position.z);
        }
    }
    
    SDL_Log("Updating mesh ID=%d: %zu vertices, %zu indices -> %zu triangle vertices",
        meshId, vertices.size(), indices.size(), meshVertices.size() / 3);
    
    // 既存のメッシュデータを更新
    _meshVertices[meshId] = meshVertices;
    _needsRebuild = true;
    
    return true;
}

void RaytracingManager::SetTexture(Texture2D* texture) {
    _currentTexture = texture;
    if (texture) {
        SDL_Log("Raytracing texture set: index %d", texture->GetDescriptorIndex());
    }
}

void RaytracingManager::Render() {
    if (!_isValid) return;

    static int frameCount = 0;
    frameCount++;

    // 10フレームに1回だけAS再構築を試行
    if (_needsRebuild && !_meshVertices.empty() && (frameCount % 10 == 0)) {
        SDL_Log("Building Acceleration Structure on render frame...");
        
        // 全てのメッシュを1つの配列に結合
        std::vector<float> combinedVertices;
        size_t totalVertices = 0;
        for (const auto& mesh : _meshVertices) {
            totalVertices += mesh.size();
        }
        
        if (totalVertices == 0) {
            SDL_Log("No vertices to build AS with");
            return;
        }
        
        combinedVertices.reserve(totalVertices);
        
        for (const auto& mesh : _meshVertices) {
            combinedVertices.insert(combinedVertices.end(), mesh.begin(), mesh.end());
        }
        
        SDL_Log("Combined %zu meshes into %zu total vertices", _meshVertices.size(), combinedVertices.size() / 3);
        
        // 結合されたメッシュでAcceleration Structureを構築
        if (!_accelerationStructure) {
            _accelerationStructure = std::make_unique<AccelerationStructure>();
        }
        
        if (_accelerationStructure->BuildSimple(combinedVertices.data(), static_cast<int>(combinedVertices.size() / 3))) {
            SDL_Log("Acceleration Structure built successfully with %zu meshes", _meshVertices.size());
            _needsRebuild = false;
        } else {
            SDL_Log("Failed to build Acceleration Structure - skipping raytracing this frame");
            return;
        }
    }

    // Acceleration Structureが構築されていない場合はレンダリングしない
    if (!_accelerationStructure || !_accelerationStructure->IsValid()) {
        SDL_Log("RaytracingManager not ready for rendering (AS not built)");
        return;
    }

    auto& commandList = DIRECT3D.CommandList();

    // コンピュートルートシグネチャを設定
    commandList.SetComputeRootSignature(_rootSignature->GlobalRootSignature());

    // パイプラインステートを設定
    commandList.SetPipelineState1(_pipeline->GetStateObject());

    // ディスクリプタヒープをバインド
    DESCRIPTORMANAGER.BindDescriptorHeaps();

    // カメラデータを更新
    UpdateCameraBuffer();
    
    // Material データを更新
    UpdateMaterialBuffer();

    // グローバルルートパラメータを設定
    commandList.SetComputeRootDescriptorTable(0, _output->GetUAVHandle());
    commandList.SetComputeRootShaderResourceView(1, _accelerationStructure->GetTLAS()->GetGPUVirtualAddress());
    
    // テクスチャをバインド
    if (_currentTexture && _currentTexture->IsValid()) {
        D3D12_GPU_DESCRIPTOR_HANDLE textureHandle = DESCRIPTORMANAGER.GetGPUHandleAt(_currentTexture->GetDescriptorIndex());
        commandList.SetComputeRootDescriptorTable(2, textureHandle);
        //SDL_Log("Texture bound to raytracing pipeline");
    } else {
        // デフォルトの白テクスチャを使用
        Texture2D* whiteTexture = Texture2D::GetWhite();
        if (whiteTexture && whiteTexture->IsValid()) {
            D3D12_GPU_DESCRIPTOR_HANDLE textureHandle = DESCRIPTORMANAGER.GetGPUHandleAt(whiteTexture->GetDescriptorIndex());
            commandList.SetComputeRootDescriptorTable(2, textureHandle);
        }
    }
    
    // カメラConstant Bufferをバインド
    commandList.SetComputeRootConstantBufferView(3, _cameraBuffer->GetGPUVirtualAddress());
    
    // Material Constant Bufferをバインド
    commandList.SetComputeRootConstantBufferView(4, _materialBuffer->GetGPUVirtualAddress());

    // DispatchRays設定
    D3D12_DISPATCH_RAYS_DESC dispatchDesc = {};
    
    dispatchDesc.RayGenerationShaderRecord.StartAddress = _shaderTable->GetRayGenShaderRecord();
    dispatchDesc.RayGenerationShaderRecord.SizeInBytes = _shaderTable->GetShaderRecordSize();
    
    dispatchDesc.MissShaderTable.StartAddress = _shaderTable->GetMissShaderRecord();
    dispatchDesc.MissShaderTable.SizeInBytes = _shaderTable->GetShaderRecordSize();
    dispatchDesc.MissShaderTable.StrideInBytes = _shaderTable->GetShaderRecordSize();
    
    dispatchDesc.HitGroupTable.StartAddress = _shaderTable->GetHitGroupRecord();
    dispatchDesc.HitGroupTable.SizeInBytes = _shaderTable->GetShaderRecordSize();
    dispatchDesc.HitGroupTable.StrideInBytes = _shaderTable->GetShaderRecordSize();
    
    dispatchDesc.Width = DIRECT3D.Width();
    dispatchDesc.Height = DIRECT3D.Height();
    dispatchDesc.Depth = 1;
    
    // レイトレーシング実行
    commandList.DispatchRays(&dispatchDesc);
    
    // UAVバリア
    auto uavBarrier = CD3DX12_RESOURCE_BARRIER::UAV(_output->GetResource());
    commandList.ResourceBarrier(1, &uavBarrier);
    
    // 結果をバックバッファにコピー
    _output->CopyToBackBuffer(DIRECT3D.CurrentRenderTarget());
    
}