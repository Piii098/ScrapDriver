#include "DirectX/ShaderTable.h"

ShaderTable::ShaderTable() : _isValid(false), _shaderRecordSize(0) {
}

bool ShaderTable::CreateShaderTable(
    ID3D12StateObjectProperties* props,
    const WCHAR* exportName,
    ID3D12Resource** buffer) {

    // シェーダー識別子を取得
    void* shaderId = props->GetShaderIdentifier(exportName);
    if (!shaderId) {
        SDL_Log("Failed to get shader identifier for %ls", exportName);
        return false;
    }

    // シェーダーレコードサイズ（32バイトアラインメント）
    _shaderRecordSize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
    _shaderRecordSize = (_shaderRecordSize + 31) & ~31; // 32バイトアラインメント

    // バッファ作成
    auto uploadHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(_shaderRecordSize);

    HRESULT hr = DIRECT3D.Device().CreateCommittedResource(
        &uploadHeap,
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(buffer));

    if (FAILED(hr)) {
        SDL_Log("Failed to create shader table buffer");
        return false;
    }

    // シェーダー識別子をコピー
    void* mapped;
    (*buffer)->Map(0, nullptr, &mapped);
    memcpy(mapped, shaderId, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
    (*buffer)->Unmap(0, nullptr);

    return true;
}

bool ShaderTable::Initialize(ID3D12StateObjectProperties* stateObjectProps) {
    // RayGenシェーダーテーブル
    if (!CreateShaderTable(stateObjectProps, L"MyRaygenShader", &_rayGenShaderTable)) {
        return false;
    }

    // Missシェーダーテーブル
    if (!CreateShaderTable(stateObjectProps, L"MyMissShader", &_missShaderTable)) {
        return false;
    }

    // HitGroupシェーダーテーブル
    if (!CreateShaderTable(stateObjectProps, L"MyHitGroup", &_hitGroupShaderTable)) {  // ← L"HitGroup"からL"MyHitGroup"に変更
        return false;
    }

    _isValid = true;
    return true;
}