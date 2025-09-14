#include "DirectX/ConstantBuffer.h"
#include "DirectX/DirectX.h"

ConstantBuffer::ConstantBuffer(size_t size)
{
    size_t align = D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT;
    UINT64 sizeAligned = (size + (align - 1)) & ~(align - 1); // alignに切り上げる.

    auto prop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD); // ヒーププロパティ
    auto desc = CD3DX12_RESOURCE_DESC::Buffer(sizeAligned); // リソースの設定

    // リソースを生成
    auto hr = DIRECT3D.Device().CreateCommittedResource(
        &prop,
        D3D12_HEAP_FLAG_NONE,
        &desc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(_buffer.GetAddressOf()));
    if (FAILED(hr))
    {
        printf("定数バッファリソースの生成に失敗\n");
        return;
    }

    hr = _buffer->Map(0, nullptr, &_mappedPtr);
    if (FAILED(hr))
    {
        printf("定数バッファのマッピングに失敗\n");
        return;
    }

    _desc = {};
    _desc.BufferLocation = _buffer->GetGPUVirtualAddress();
    _desc.SizeInBytes = UINT(sizeAligned);

    _isValid = true;
}

bool ConstantBuffer::IsValid()
{
    return _isValid;
}

D3D12_GPU_VIRTUAL_ADDRESS ConstantBuffer::GetAddress() const
{
    return _desc.BufferLocation;
}

D3D12_CONSTANT_BUFFER_VIEW_DESC ConstantBuffer::ViewDesc()
{
    return _desc;
}

void* ConstantBuffer::GetPtr() const
{
    return _mappedPtr;
}
