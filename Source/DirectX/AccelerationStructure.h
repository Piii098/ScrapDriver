#pragma once
#include "DirectX/DirectX.h"

class AccelerationStructure {
public:
    AccelerationStructure();

    // �O�p�`1��BLAS��TLAS�������\�z
    bool BuildSimple(float* vertices, int vertexCount);

    ID3D12Resource* GetTLAS() { return _tlas.Get(); }
    bool IsValid() const { return _isValid; }

private:
    bool _isValid = false;
    ComPtr<ID3D12Resource> _vertexBuffer;
    ComPtr<ID3D12Resource> _blas;
    ComPtr<ID3D12Resource> _tlas;
    ComPtr<ID3D12Resource> _blasScratch;
    ComPtr<ID3D12Resource> _tlasScratch;
    ComPtr<ID3D12Resource> _instanceDesc;
};