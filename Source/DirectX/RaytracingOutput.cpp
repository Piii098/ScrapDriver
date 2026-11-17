#include "DirectX/RaytracingOutput.h"
#include "DirectX/DescriptorHeap.h"

RaytracingOutput::RaytracingOutput() : _isValid(false), _width(0), _height(0) {
}

bool RaytracingOutput::Initialize(UINT width, UINT height) {
    _width = width;
    _height = height;

    // UAVリソース作成
    auto defaultHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    auto resourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(
        DXGI_FORMAT_R8G8B8A8_UNORM,
        width,
        height,
        1, 1, 1, 0,
        D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

    HRESULT hr = DIRECT3D.Device().CreateCommittedResource(
        &defaultHeap,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
        nullptr,
        IID_PPV_ARGS(&_outputResource));

    if (FAILED(hr)) {
        SDL_Log("Failed to create raytracing output resource");
        return false;
    }

    // UAVをディスクリプタヒープに登録
    D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;

    UINT descriptorIndex = DESCRIPTORMANAGER.AllocateDescriptor();
    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = DESCRIPTORMANAGER.GetCPUHandleAt(descriptorIndex);

    DIRECT3D.Device().CreateUnorderedAccessView(
        _outputResource.Get(),
        nullptr,
        &uavDesc,
        cpuHandle);

    _uavHandle = DESCRIPTORMANAGER.GetGPUHandleAt(descriptorIndex);

    _isValid = true;
    return true;
}

void RaytracingOutput::CopyToBackBuffer(ID3D12Resource* backBuffer) {
    auto& commandList = DIRECT3D.CommandList();

    // リソースバリア: UAV -> COPY_SOURCE
    auto barrier1 = CD3DX12_RESOURCE_BARRIER::Transition(
        _outputResource.Get(),
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
        D3D12_RESOURCE_STATE_COPY_SOURCE);
    commandList.ResourceBarrier(1, &barrier1);

    // バックバッファ: RENDER_TARGET -> COPY_DEST
    auto barrier2 = CD3DX12_RESOURCE_BARRIER::Transition(
        backBuffer,
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_COPY_DEST);
    commandList.ResourceBarrier(1, &barrier2);

    // コピー
    commandList.CopyResource(backBuffer, _outputResource.Get());

    // バリアを戻す
    auto barrier3 = CD3DX12_RESOURCE_BARRIER::Transition(
        _outputResource.Get(),
        D3D12_RESOURCE_STATE_COPY_SOURCE,
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    commandList.ResourceBarrier(1, &barrier3);

    auto barrier4 = CD3DX12_RESOURCE_BARRIER::Transition(
        backBuffer,
        D3D12_RESOURCE_STATE_COPY_DEST,
        D3D12_RESOURCE_STATE_RENDER_TARGET);
    commandList.ResourceBarrier(1, &barrier4);
}