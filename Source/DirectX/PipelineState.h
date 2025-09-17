#pragma once
#include "DirectX/DirectX.h"
#include <string.h>

class PipelineState
{
public:
	PipelineState(); // コンストラクタである程度の設定をする
	bool IsValid(); // 生成に成功したかどうかを返す

	void SetInputLayout(D3D12_INPUT_LAYOUT_DESC  layout); // 入力レイアウトを設定
	void SetRootSignature(ID3D12RootSignature* rootSignature); // ルートシグネチャを設定
	void SetVS(std::wstring filePath); // 頂点シェーダーを設定
	void SetPS(std::wstring filePath); // ピクセルシェーダーを設定
	void Create(); // パイプラインステートを生成

	ID3D12PipelineState* Get();

private:
	bool _isValid = false; // 生成に成功したかどうか
	D3D12_GRAPHICS_PIPELINE_STATE_DESC _desc = {}; // パイプラインステートの設定
	ComPtr<ID3D12PipelineState> _pipelineState = nullptr; // パイプラインステート
	ComPtr<ID3DBlob> _vsBlob; // 頂点シェーダー
	ComPtr<ID3DBlob> _psBlob; // ピクセルシェーダー
};