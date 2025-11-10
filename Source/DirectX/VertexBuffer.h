#pragma once
#include "DirectX/DirectX.h"
#include "Utility/Math.h"

class VertexBuffer
{
public:
	VertexBuffer(Vertex* vertices, size_t vertexCount); // コンストラクタでバッファを生成
	D3D12_VERTEX_BUFFER_VIEW View() const; // 頂点バッファビューを取得
	bool IsValid(); // バッファの生成に成功したかを取得

	void Bind();

private:
	bool _isValid = false; // バッファの生成に成功したかを取得
	ComPtr<ID3D12Resource> _buffer = nullptr; // バッファ
	D3D12_VERTEX_BUFFER_VIEW _view = {}; // 頂点バッファビュー

};