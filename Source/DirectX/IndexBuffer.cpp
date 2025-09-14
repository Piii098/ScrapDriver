#include "DirectX/IndexBuffer.h"


IndexBuffer::IndexBuffer(size_t size, const uint32_t* pInitData)
{
	auto prop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD); // ヒーププロパティ
	D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(size);	// リソースの設定

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
		printf("[OnInit] インデックスバッファリソースの生成に失敗");
		return;
	}

	// インデックスバッファビューの設定
	_view = {};
	_view.BufferLocation = _buffer->GetGPUVirtualAddress();
	_view.Format = DXGI_FORMAT_R32_UINT;
	_view.SizeInBytes = static_cast<UINT>(size);

	// マッピングする
	if (pInitData != nullptr)
	{
		void* ptr = nullptr;
		hr = _buffer->Map(0, nullptr, &ptr);
		if (FAILED(hr))
		{
			printf("[OnInit] インデックスバッファマッピングに失敗");
			return;
		}

		// インデックスデータをマッピング先に設定
		memcpy(ptr, pInitData, size);

		// マッピング解除
		_buffer->Unmap(0, nullptr);
	}
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