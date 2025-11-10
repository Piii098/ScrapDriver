#include "DirectX/VertexBuffer.h"
#include "Framework.h"

VertexBuffer::VertexBuffer(Vertex* vertices, size_t vertexCount)
{
    D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
    heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

    D3D12_RESOURCE_DESC desc = {};
    desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    desc.Alignment = 0;
    desc.Width = sizeof(Vertex) * vertexCount;
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
        SDL_Log("Failed to create vertex buffer resource.");
		_isValid = false;
        return;
    }

    Vertex* vertMap = nullptr;
    if (FAILED(_buffer->Map(0, nullptr, (void**)&vertMap))) {
        SDL_Log("Failed to map vertex buffer.");
		_isValid = false;
        return;
    }

    std::copy(vertices, vertices + vertexCount, vertMap);

    _buffer->Unmap(0, nullptr);

    _view = {};
    _view.BufferLocation = _buffer->GetGPUVirtualAddress();
    _view.SizeInBytes = static_cast<UINT>(sizeof(Vertex) * vertexCount);
    _view.StrideInBytes = sizeof(Vertex);

	_isValid = true;
}

D3D12_VERTEX_BUFFER_VIEW VertexBuffer::View() const
{
	return _view;
}

bool VertexBuffer::IsValid()
{
	return _isValid;
}

void VertexBuffer::Bind()
{
	DIRECT3D.CommandList().IASetVertexBuffers(0, 1, &_view);
}