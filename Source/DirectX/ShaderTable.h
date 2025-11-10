#pragma once
#include "DirectX/DirectX.h"

class ShaderTable {
public:
    ShaderTable();

    bool Initialize(ID3D12StateObjectProperties* stateObjectProps);

    D3D12_GPU_VIRTUAL_ADDRESS GetRayGenShaderRecord() const {
        return _rayGenShaderTable->GetGPUVirtualAddress();
    }

    D3D12_GPU_VIRTUAL_ADDRESS GetMissShaderRecord() const {
        return _missShaderTable->GetGPUVirtualAddress();
    }

    D3D12_GPU_VIRTUAL_ADDRESS GetHitGroupRecord() const {
        return _hitGroupShaderTable->GetGPUVirtualAddress();
    }

    UINT GetShaderRecordSize() const { return _shaderRecordSize; }

    bool IsValid() const { return _isValid; }

private:
    bool _isValid = false;
    ComPtr<ID3D12Resource> _rayGenShaderTable;
    ComPtr<ID3D12Resource> _missShaderTable;
    ComPtr<ID3D12Resource> _hitGroupShaderTable;
    UINT _shaderRecordSize;

    bool CreateShaderTable(
        ID3D12StateObjectProperties* props,
        const WCHAR* exportName,
        ID3D12Resource** buffer);
};