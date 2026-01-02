#pragma once
#include "DirectX.h"

// Generic constant buffer template
template<typename T>
class GenericConstantBuffer
{
public:
    GenericConstantBuffer() {
        _constantBuffer = nullptr;
        
        // Create constant buffer with proper alignment (256 bytes)
        D3D12_HEAP_PROPERTIES heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        auto desc = CD3DX12_RESOURCE_DESC::Buffer((sizeof(T) + 0xff) & ~0xff);
        
        if (FAILED(DIRECT3D.Device().CreateCommittedResource(
            &heapProp,
            D3D12_HEAP_FLAG_NONE,
            &desc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(_constantBuffer.GetAddressOf())))) {
            SDL_Log("Failed to create generic constant buffer resource.");
            _mappedPtr = nullptr;
            _isValid = false;
            return;
        }
        
        _constantBuffer->Map(0, nullptr, (void**)&_mappedPtr);
        _isValid = true;
    }
    
    bool IsValid() const { return _isValid; }
    ID3D12Resource* GetResource() const { return _constantBuffer.Get(); }
    T* GetPtr() const { return _mappedPtr; }
    
private:
    bool _isValid = false;
    ComPtr<ID3D12Resource> _constantBuffer;
    T* _mappedPtr = nullptr;
};
