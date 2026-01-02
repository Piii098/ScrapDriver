#pragma once
#include "DirectX/DirectX.h"

class ShadowMap {
public:
    ShadowMap();
    ~ShadowMap();

    bool Initialize(UINT width, UINT height);
    void Clear();
    
    // Getters
    ID3D12Resource* GetResource() const { return _shadowMapResource.Get(); }
    D3D12_CPU_DESCRIPTOR_HANDLE GetDSV() const { return _dsvHandle; }
    D3D12_CPU_DESCRIPTOR_HANDLE GetSRV() const { return _srvHandle; }
    D3D12_GPU_DESCRIPTOR_HANDLE GetGPUSRV() const { return _gpuSrvHandle; }
    UINT GetWidth() const { return _width; }
    UINT GetHeight() const { return _height; }
    UINT GetSRVIndex() const { return _srvIndex; }
    
private:
    ComPtr<ID3D12Resource> _shadowMapResource;
    ComPtr<ID3D12DescriptorHeap> _dsvHeap;
    D3D12_CPU_DESCRIPTOR_HANDLE _dsvHandle = {};
    D3D12_CPU_DESCRIPTOR_HANDLE _srvHandle = {};
    D3D12_GPU_DESCRIPTOR_HANDLE _gpuSrvHandle = {};
    UINT _width = 0;
    UINT _height = 0;
    UINT _srvIndex = 0;
    bool _isValid = false;
};
