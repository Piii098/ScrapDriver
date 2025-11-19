#pragma once
#include "DirectX/DirectX.h"

class AccelerationStructure {
public:
    AccelerationStructure();

    // �O�p�`1��BLAS��TLAS�������\�z
    bool BuildSimple(float* vertices, int vertexCount);
    
    // 複数メッシュ用のBLAS/TLAS構築
    bool BuildMultipleMeshes(const std::vector<std::vector<float>>& meshVertices);

    ID3D12Resource* GetTLAS() { return _tlas.Get(); }
    bool IsValid() const { return _isValid; }

private:
    bool _isValid = false;
    std::vector<ComPtr<ID3D12Resource>> _vertexBuffers;  // 複数メッシュ用
    std::vector<ComPtr<ID3D12Resource>> _blasList;       // 各メッシュのBLAS
    ComPtr<ID3D12Resource> _tlas;
    std::vector<ComPtr<ID3D12Resource>> _blasScratchList; // 各BLASのスクラッチ
    ComPtr<ID3D12Resource> _tlasScratch;
    ComPtr<ID3D12Resource> _instanceDesc;
    
    // 旧式（後方互換性のため残す）
    ComPtr<ID3D12Resource> _vertexBuffer;
    ComPtr<ID3D12Resource> _blas;
    ComPtr<ID3D12Resource> _blasScratch;
};