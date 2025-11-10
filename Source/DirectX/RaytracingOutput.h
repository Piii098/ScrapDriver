#pragma once
#include "DirectX/DirectX.h"

class RaytracingOutput {
public:
    RaytracingOutput();

    bool Initialize(UINT width, UINT height);
    void CopyToBackBuffer(ID3D12Resource* backBuffer);

    ID3D12Resource* GetResource() { return _outputResource.Get(); }
    D3D12_GPU_DESCRIPTOR_HANDLE GetUAVHandle() const { return _uavHandle; }

    bool IsValid() const { return _isValid; }

private:
    bool _isValid = false;
    ComPtr<ID3D12Resource> _outputResource;
    D3D12_GPU_DESCRIPTOR_HANDLE _uavHandle;
    UINT _width;
    UINT _height;
};