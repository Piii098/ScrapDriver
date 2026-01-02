#include "DirectX/ShadowMap.h"
#include "DirectX/Direct3D.h"
#include "DirectX/DescriptorHeap.h"
#include <SDL3/SDL.h>

ShadowMap::ShadowMap() : _dsvHeap(nullptr) {
}

ShadowMap::~ShadowMap() {
}

bool ShadowMap::Initialize(UINT width, UINT height) {
    _width = width;
    _height = height;

    // Create depth buffer resource for shadow map
    D3D12_HEAP_PROPERTIES heapProp = {};
    heapProp.Type = D3D12_HEAP_TYPE_DEFAULT;
    heapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

    D3D12_RESOURCE_DESC resDesc = {};
    resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    resDesc.Width = width;
    resDesc.Height = height;
    resDesc.DepthOrArraySize = 1;
    resDesc.MipLevels = 1;
    resDesc.Format = DXGI_FORMAT_R32_TYPELESS; // Use typeless format for DSV and SRV
    resDesc.SampleDesc.Count = 1;
    resDesc.SampleDesc.Quality = 0;
    resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    resDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_CLEAR_VALUE clearValue = {};
    clearValue.Format = DXGI_FORMAT_D32_FLOAT;
    clearValue.DepthStencil.Depth = 1.0f;
    clearValue.DepthStencil.Stencil = 0;

    auto hr = DIRECT3D.Device().CreateCommittedResource(
        &heapProp,
        D3D12_HEAP_FLAG_NONE,
        &resDesc,
        D3D12_RESOURCE_STATE_DEPTH_WRITE,
        &clearValue,
        IID_PPV_ARGS(&_shadowMapResource)
    );

    if (FAILED(hr)) {
        SDL_Log("Failed to create shadow map resource");
        return false;
    }

    // Create DSV descriptor heap
    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
    dsvHeapDesc.NumDescriptors = 1;
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    hr = DIRECT3D.Device().CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&_dsvHeap));
    if (FAILED(hr)) {
        SDL_Log("Failed to create DSV descriptor heap for shadow map");
        return false;
    }

    // Create DSV (Depth Stencil View)
    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
    
    _dsvHandle = _dsvHeap->GetCPUDescriptorHandleForHeapStart();
    DIRECT3D.Device().CreateDepthStencilView(_shadowMapResource.Get(), &dsvDesc, _dsvHandle);

    // Create SRV (Shader Resource View) for sampling shadow map in pixel shader
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Texture2D.MipLevels = 1;
    srvDesc.Texture2D.MostDetailedMip = 0;

    UINT srvIndex = DESCRIPTORMANAGER.AllocateDescriptor();
    if (srvIndex == static_cast<UINT>(-1)) {
        SDL_Log("Failed to allocate SRV for shadow map");
        return false;
    }

    _srvHandle = DESCRIPTORMANAGER.GetCPUHandleAt(srvIndex);
    _gpuSrvHandle = DESCRIPTORMANAGER.GetGPUHandleAt(srvIndex);
    _srvIndex = srvIndex;
    DIRECT3D.Device().CreateShaderResourceView(_shadowMapResource.Get(), &srvDesc, _srvHandle);

    _isValid = true;
    SDL_Log("Shadow map initialized: %dx%d", width, height);
    return true;
}

void ShadowMap::Clear() {
    if (!_isValid) return;
    
    DIRECT3D.CommandList().ClearDepthStencilView(_dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
}
