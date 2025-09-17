#include "DirectX/VertexBuffer.h"
#include "Framework.h"

VertexBuffer::VertexBuffer(size_t size, size_t stride, const void* pInitData)
{

	auto prop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD); 	// ヒーププロパティ
	auto desc = CD3DX12_RESOURCE_DESC::Buffer(size); 	// リソースの設定

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
		printf("頂点バッファリソースの生成に失敗");
		return;
	}

	// 頂点バッファビューの設定
	_view.BufferLocation = _buffer->GetGPUVirtualAddress();
	_view.SizeInBytes = static_cast<UINT>(size);
	_view.StrideInBytes = static_cast<UINT>(stride);

	// マッピングする
	if (pInitData != nullptr)
	{
		void* ptr = nullptr;
		hr = _buffer->Map(0, nullptr, &ptr);
		if (FAILED(hr))
		{
			printf("頂点バッファマッピングに失敗");
			return;
		}

		// 頂点データをマッピング先に設定
		memcpy(ptr, pInitData, size);

		// マッピング解除
		_buffer->Unmap(0, nullptr);
	}

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