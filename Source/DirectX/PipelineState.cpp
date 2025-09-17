#include "DirectX/PipelineState.h"

PipelineState::PipelineState()
{
	// パイプラインステートの設定
	_desc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT); // ラスタライザーはデフォルト
	_desc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE; // カリングはなし
	_desc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT); // ブレンドステートもデフォルト
	_desc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT); // 深度ステンシルはデフォルトを使う
	_desc.SampleMask = UINT_MAX;
	_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE; // 三角形を描画
	_desc.NumRenderTargets = 1; // 描画対象は1
	_desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	_desc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	_desc.SampleDesc.Count = 1; // サンプラーは1
	_desc.SampleDesc.Quality = 0;
}

bool PipelineState::IsValid()
{
	return _isValid;
}

void PipelineState::SetInputLayout(D3D12_INPUT_LAYOUT_DESC layout)
{
	_desc.InputLayout = layout;
}

void PipelineState::SetRootSignature(ID3D12RootSignature* rootSignature)
{
	_desc.pRootSignature = rootSignature;
}

void PipelineState::SetVS(std::wstring filePath)
{
	// 頂点シェーダー読み込み
	auto hr = D3DReadFileToBlob(filePath.c_str(), _vsBlob.GetAddressOf());
	if (FAILED(hr))
	{
		printf("頂点シェーダーの読み込みに失敗");
		return;
	}

	_desc.VS = CD3DX12_SHADER_BYTECODE(_vsBlob.Get());
}

void PipelineState::SetPS(std::wstring filePath)
{
	// ピクセルシェーダー読み込み
	auto hr = D3DReadFileToBlob(filePath.c_str(), _psBlob.GetAddressOf());
	if (FAILED(hr))
	{
		printf("ピクセルシェーダーの読み込みに失敗");
		return;
	}

	_desc.PS = CD3DX12_SHADER_BYTECODE(_psBlob.Get());
}

void PipelineState::Create()
{
	// パイプラインステートを生成
	auto hr = DIRECT3D.Device().CreateGraphicsPipelineState(&_desc, IID_PPV_ARGS(_pipelineState.ReleaseAndGetAddressOf()));
	if (FAILED(hr))
	{
		printf("パイプラインステートの生成に失敗");
		return;
	}

	_isValid = true;
}

ID3D12PipelineState* PipelineState::Get()
{
	return _pipelineState.Get();
}