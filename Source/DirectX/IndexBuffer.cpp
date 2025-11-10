#include "DirectX/IndexBuffer.h"


IndexBuffer::IndexBuffer(unsigned short* indeces, size_t indexCount)
{
    D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
    heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

    D3D12_RESOURCE_DESC desc = {};
    desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    desc.Alignment = 0;
    desc.Width = sizeof(unsigned short) * indexCount;
    desc.Height = 1;
    desc.DepthOrArraySize = 1;
    desc.MipLevels = 1;
    desc.SampleDesc.Count = 1;
    desc.Format = DXGI_FORMAT_UNKNOWN;
    desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    desc.Flags = D3D12_RESOURCE_FLAG_NONE;

    if (FAILED(DIRECT3D.Device().CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &desc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&_buffer)))) {
        SDL_Log("Failed to create index buffer resource.");
		_isValid = false;
        return;
    }
    //作ったバッファにインデックスデータをコピー
    unsigned short* mappedIdx = nullptr;
    _buffer->Map(0, nullptr, (void**)&mappedIdx);
    std::copy(indeces, indeces + indexCount, mappedIdx);
    _buffer->Unmap(0, nullptr);

    //インデックスバッファビューを作成
    _view = {};
    _view.BufferLocation = _buffer->GetGPUVirtualAddress();
    _view.Format = DXGI_FORMAT_R16_UINT;
    _view.SizeInBytes = static_cast<UINT>(sizeof(unsigned short) * indexCount);

	_isValid = true;
}

bool IndexBuffer::IsValid()
{
	return _isValid;
}

D3D12_INDEX_BUFFER_VIEW IndexBuffer::View() const
{
	return _view;
}

void IndexBuffer::Bind() {
    DIRECT3D.CommandList().IASetIndexBuffer(&_view);
}