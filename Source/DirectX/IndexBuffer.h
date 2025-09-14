#pragma once
#include "DirectX/DirectX.h"

class IndexBuffer
{
public:
	IndexBuffer(size_t size, const uint32_t* pInitData = nullptr);
	bool IsValid();
	D3D12_INDEX_BUFFER_VIEW View() const;

private:
	bool _isValid = false;
	ComPtr<ID3D12Resource> _buffer; // インデックスバッファ
	D3D12_INDEX_BUFFER_VIEW _view; // インデックスバッファビュー

	IndexBuffer(const IndexBuffer&) = delete;
	void operator = (const IndexBuffer&) = delete;
};
