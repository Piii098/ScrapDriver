#include "DirectX/AccelerationStructure.h"

AccelerationStructure::AccelerationStructure() : _isValid(false) {
}

bool AccelerationStructure::BuildSimple(float* vertices, int vertexCount) {
    auto& device = DIRECT3D.DXRDevice();
    auto& commandList = DIRECT3D.CommandList();

    // 頂点バッファ作成（アップロードヒープ）
    UINT64 vertexBufferSize = sizeof(float) * 3 * vertexCount;
    
    SDL_Log("Building acceleration structure with %d vertices", vertexCount);
    SDL_Log("Vertex buffer size: %llu bytes", vertexBufferSize);

    auto uploadHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);

    if (FAILED(DIRECT3D.Device().CreateCommittedResource(
        &uploadHeap,
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&_vertexBuffer)))) {
        SDL_Log("Failed to create vertex buffer");
        return false;
    }

    // 頂点データコピー
    void* mapped;
    _vertexBuffer->Map(0, nullptr, &mapped);
    memcpy(mapped, vertices, vertexBufferSize);
    _vertexBuffer->Unmap(0, nullptr);

    // デバッグ: 頂点データを出力
    SDL_Log("Triangle vertices:");
    for (int i = 0; i < vertexCount; i++) {
        SDL_Log("  v%d: (%f, %f, %f)", i, vertices[i*3], vertices[i*3+1], vertices[i*3+2]);
    }

    // ジオメトリ記述
    D3D12_RAYTRACING_GEOMETRY_DESC geometryDesc = {};
    geometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
    geometryDesc.Triangles.VertexBuffer.StartAddress = _vertexBuffer->GetGPUVirtualAddress();
    geometryDesc.Triangles.VertexBuffer.StrideInBytes = sizeof(float) * 3;
    geometryDesc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
    geometryDesc.Triangles.VertexCount = vertexCount;
    geometryDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;  // OPAQUEに戻す
    
    // BLAS構築
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS blasInputs = {};
    blasInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
    blasInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
    blasInputs.pGeometryDescs = &geometryDesc;
    blasInputs.NumDescs = 1;
    blasInputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;

    // BLASバッファサイズ取得
    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO blasInfo = {};
    device.GetRaytracingAccelerationStructurePrebuildInfo(&blasInputs, &blasInfo);
    
    SDL_Log("BLAS scratch size: %llu bytes", blasInfo.ScratchDataSizeInBytes);
    SDL_Log("BLAS result size: %llu bytes", blasInfo.ResultDataMaxSizeInBytes);

    // BLASバッファ作成
    auto defaultHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    auto scratchDesc = CD3DX12_RESOURCE_DESC::Buffer(blasInfo.ScratchDataSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
    auto asDesc = CD3DX12_RESOURCE_DESC::Buffer(blasInfo.ResultDataMaxSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

    DIRECT3D.Device().CreateCommittedResource(&defaultHeap, D3D12_HEAP_FLAG_NONE, &scratchDesc, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr, IID_PPV_ARGS(&_blasScratch));
    DIRECT3D.Device().CreateCommittedResource(&defaultHeap, D3D12_HEAP_FLAG_NONE, &asDesc, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, nullptr, IID_PPV_ARGS(&_blas));

    // BLAS構築
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC blasDesc = {};
    blasDesc.Inputs = blasInputs;
    blasDesc.ScratchAccelerationStructureData = _blasScratch->GetGPUVirtualAddress();
    blasDesc.DestAccelerationStructureData = _blas->GetGPUVirtualAddress();

    commandList.BuildRaytracingAccelerationStructure(&blasDesc, 0, nullptr);

    // UAVバリア
    auto uavBarrier = CD3DX12_RESOURCE_BARRIER::UAV(_blas.Get());
    commandList.ResourceBarrier(1, &uavBarrier);

    // インスタンス記述（単位行列）
    D3D12_RAYTRACING_INSTANCE_DESC instanceDesc = {};
    instanceDesc.Transform[0][0] = instanceDesc.Transform[1][1] = instanceDesc.Transform[2][2] = 1.0f;
    instanceDesc.InstanceID = 0;
    instanceDesc.InstanceMask = 0xFF;
    instanceDesc.InstanceContributionToHitGroupIndex = 0;
    instanceDesc.Flags = D3D12_RAYTRACING_INSTANCE_FLAG_NONE;
    instanceDesc.AccelerationStructure = _blas->GetGPUVirtualAddress();

    SDL_Log("BLAS GPU address: 0x%llx", instanceDesc.AccelerationStructure);

    // インスタンスバッファ
    bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(D3D12_RAYTRACING_INSTANCE_DESC));
    DIRECT3D.Device().CreateCommittedResource(&uploadHeap, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&_instanceDesc));

    _instanceDesc->Map(0, nullptr, &mapped);
    memcpy(mapped, &instanceDesc, sizeof(instanceDesc));
    _instanceDesc->Unmap(0, nullptr);

    // TLAS構築
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS tlasInputs = {};
    tlasInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
    tlasInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
    tlasInputs.InstanceDescs = _instanceDesc->GetGPUVirtualAddress();
    tlasInputs.NumDescs = 1;
    tlasInputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;

    // TLASバッファサイズ取得
    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO tlasInfo = {};
    device.GetRaytracingAccelerationStructurePrebuildInfo(&tlasInputs, &tlasInfo);
    
    SDL_Log("TLAS scratch size: %llu bytes", tlasInfo.ScratchDataSizeInBytes);
    SDL_Log("TLAS result size: %llu bytes", tlasInfo.ResultDataMaxSizeInBytes);

    // TLASバッファ作成
    scratchDesc = CD3DX12_RESOURCE_DESC::Buffer(tlasInfo.ScratchDataSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
    asDesc = CD3DX12_RESOURCE_DESC::Buffer(tlasInfo.ResultDataMaxSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

    DIRECT3D.Device().CreateCommittedResource(&defaultHeap, D3D12_HEAP_FLAG_NONE, &scratchDesc, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr, IID_PPV_ARGS(&_tlasScratch));
    DIRECT3D.Device().CreateCommittedResource(&defaultHeap, D3D12_HEAP_FLAG_NONE, &asDesc, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, nullptr, IID_PPV_ARGS(&_tlas));

    // TLAS構築
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC tlasDesc = {};
    tlasDesc.Inputs = tlasInputs;
    tlasDesc.ScratchAccelerationStructureData = _tlasScratch->GetGPUVirtualAddress();
    tlasDesc.DestAccelerationStructureData = _tlas->GetGPUVirtualAddress();

    commandList.BuildRaytracingAccelerationStructure(&tlasDesc, 0, nullptr);

    uavBarrier = CD3DX12_RESOURCE_BARRIER::UAV(_tlas.Get());
    commandList.ResourceBarrier(1, &uavBarrier);

    SDL_Log("TLAS GPU address: 0x%llx", _tlas->GetGPUVirtualAddress());

    _isValid = true;
    SDL_Log("Acceleration structure build commands recorded");
    return true;
}

bool AccelerationStructure::BuildMultipleMeshes(const std::vector<std::vector<float>>& meshVertices) {
    auto& device = DIRECT3D.DXRDevice();
    auto& commandList = DIRECT3D.CommandList();
    
    if (meshVertices.empty()) {
        SDL_Log("No meshes to build");
        return false;
    }
    
    SDL_Log("Building acceleration structure with %zu meshes", meshVertices.size());
    
    // クリア
    _vertexBuffers.clear();
    _blasList.clear();
    _blasScratchList.clear();
    
    auto uploadHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    auto defaultHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    
    // 各メッシュのBLASを構築
    for (size_t meshIdx = 0; meshIdx < meshVertices.size(); meshIdx++) {
        const auto& vertices = meshVertices[meshIdx];
        UINT64 vertexBufferSize = vertices.size() * sizeof(float);
        
        // 頂点バッファ作成
        ComPtr<ID3D12Resource> vertexBuffer;
        auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);
        
        if (FAILED(DIRECT3D.Device().CreateCommittedResource(
            &uploadHeap,
            D3D12_HEAP_FLAG_NONE,
            &bufferDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&vertexBuffer)))) {
            SDL_Log("Failed to create vertex buffer for mesh %zu", meshIdx);
            return false;
        }
        
        // 頂点データコピー
        void* mapped;
        vertexBuffer->Map(0, nullptr, &mapped);
        memcpy(mapped, vertices.data(), vertexBufferSize);
        vertexBuffer->Unmap(0, nullptr);
        
        _vertexBuffers.push_back(vertexBuffer);
        
        // ジオメトリ記述
        D3D12_RAYTRACING_GEOMETRY_DESC geometryDesc = {};
        geometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
        geometryDesc.Triangles.VertexBuffer.StartAddress = vertexBuffer->GetGPUVirtualAddress();
        geometryDesc.Triangles.VertexBuffer.StrideInBytes = sizeof(float) * 3;
        geometryDesc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
        geometryDesc.Triangles.VertexCount = static_cast<UINT>(vertices.size() / 3);
        geometryDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;
        
        // BLAS構築情報
        D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS blasInputs = {};
        blasInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
        blasInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
        blasInputs.pGeometryDescs = &geometryDesc;
        blasInputs.NumDescs = 1;
        blasInputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
        
        // BLASバッファサイズ取得
        D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO blasInfo = {};
        device.GetRaytracingAccelerationStructurePrebuildInfo(&blasInputs, &blasInfo);
        
        // BLASバッファ作成
        ComPtr<ID3D12Resource> blasScratch;
        ComPtr<ID3D12Resource> blas;
        
        auto scratchDesc = CD3DX12_RESOURCE_DESC::Buffer(blasInfo.ScratchDataSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
        auto asDesc = CD3DX12_RESOURCE_DESC::Buffer(blasInfo.ResultDataMaxSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
        
        DIRECT3D.Device().CreateCommittedResource(&defaultHeap, D3D12_HEAP_FLAG_NONE, &scratchDesc, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr, IID_PPV_ARGS(&blasScratch));
        DIRECT3D.Device().CreateCommittedResource(&defaultHeap, D3D12_HEAP_FLAG_NONE, &asDesc, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, nullptr, IID_PPV_ARGS(&blas));
        
        // BLAS構築
        D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC blasDesc = {};
        blasDesc.Inputs = blasInputs;
        blasDesc.ScratchAccelerationStructureData = blasScratch->GetGPUVirtualAddress();
        blasDesc.DestAccelerationStructureData = blas->GetGPUVirtualAddress();
        
        commandList.BuildRaytracingAccelerationStructure(&blasDesc, 0, nullptr);
        
        // UAVバリア
        auto uavBarrier = CD3DX12_RESOURCE_BARRIER::UAV(blas.Get());
        commandList.ResourceBarrier(1, &uavBarrier);
        
        _blasScratchList.push_back(blasScratch);
        _blasList.push_back(blas);
        
        SDL_Log("Mesh %zu: %zu vertices, BLAS address: 0x%llx", meshIdx, vertices.size() / 3, blas->GetGPUVirtualAddress());
    }
    
    // 複数のインスタンス記述を作成
    std::vector<D3D12_RAYTRACING_INSTANCE_DESC> instanceDescs(meshVertices.size());
    
    for (size_t i = 0; i < meshVertices.size(); i++) {
        instanceDescs[i].Transform[0][0] = instanceDescs[i].Transform[1][1] = instanceDescs[i].Transform[2][2] = 1.0f;
        instanceDescs[i].InstanceID = static_cast<UINT>(i);  // 各メッシュに固有のID
        instanceDescs[i].InstanceMask = 0xFF;
        instanceDescs[i].InstanceContributionToHitGroupIndex = 0;
        instanceDescs[i].Flags = D3D12_RAYTRACING_INSTANCE_FLAG_NONE;
        instanceDescs[i].AccelerationStructure = _blasList[i]->GetGPUVirtualAddress();
        
        SDL_Log("Instance %zu: InstanceID=%u, BLAS=0x%llx", i, instanceDescs[i].InstanceID, instanceDescs[i].AccelerationStructure);
    }
    
    // インスタンスバッファ作成
    UINT64 instanceBufferSize = sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * meshVertices.size();
    auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(instanceBufferSize);
    
    DIRECT3D.Device().CreateCommittedResource(&uploadHeap, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&_instanceDesc));
    
    void* mapped;
    _instanceDesc->Map(0, nullptr, &mapped);
    memcpy(mapped, instanceDescs.data(), instanceBufferSize);
    _instanceDesc->Unmap(0, nullptr);
    
    // TLAS構築
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS tlasInputs = {};
    tlasInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
    tlasInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
    tlasInputs.InstanceDescs = _instanceDesc->GetGPUVirtualAddress();
    tlasInputs.NumDescs = static_cast<UINT>(meshVertices.size());  // 複数インスタンス
    tlasInputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
    
    // TLASバッファサイズ取得
    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO tlasInfo = {};
    device.GetRaytracingAccelerationStructurePrebuildInfo(&tlasInputs, &tlasInfo);
    
    SDL_Log("TLAS with %zu instances: scratch=%llu bytes, result=%llu bytes", 
        meshVertices.size(), tlasInfo.ScratchDataSizeInBytes, tlasInfo.ResultDataMaxSizeInBytes);
    
    // TLASバッファ作成
    auto scratchDesc = CD3DX12_RESOURCE_DESC::Buffer(tlasInfo.ScratchDataSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
    auto asDesc = CD3DX12_RESOURCE_DESC::Buffer(tlasInfo.ResultDataMaxSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
    
    DIRECT3D.Device().CreateCommittedResource(&defaultHeap, D3D12_HEAP_FLAG_NONE, &scratchDesc, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr, IID_PPV_ARGS(&_tlasScratch));
    DIRECT3D.Device().CreateCommittedResource(&defaultHeap, D3D12_HEAP_FLAG_NONE, &asDesc, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, nullptr, IID_PPV_ARGS(&_tlas));
    
    // TLAS構築
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC tlasDesc = {};
    tlasDesc.Inputs = tlasInputs;
    tlasDesc.ScratchAccelerationStructureData = _tlasScratch->GetGPUVirtualAddress();
    tlasDesc.DestAccelerationStructureData = _tlas->GetGPUVirtualAddress();
    
    commandList.BuildRaytracingAccelerationStructure(&tlasDesc, 0, nullptr);
    
    auto uavBarrier = CD3DX12_RESOURCE_BARRIER::UAV(_tlas.Get());
    commandList.ResourceBarrier(1, &uavBarrier);
    
    SDL_Log("TLAS GPU address: 0x%llx", _tlas->GetGPUVirtualAddress());
    
    _isValid = true;
    SDL_Log("Multi-mesh acceleration structure built successfully");
    return true;
}