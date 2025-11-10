#pragma once
#include "DirectX/DirectX.h"

class IndexBuffer
{
public:
	IndexBuffer(unsigned short* indeces, size_t indexCount);
	bool IsValid();
	D3D12_INDEX_BUFFER_VIEW View() const;

	void Bind();

private:
	bool _isValid = false;
	ComPtr<ID3D12Resource> _buffer; // インデックスバッファ
	D3D12_INDEX_BUFFER_VIEW _view; // インデックスバッファビュー

};
