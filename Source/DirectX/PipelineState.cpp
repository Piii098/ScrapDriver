#include "DirectX/PipelineState.h"

PipelineState::PipelineState()
{
	// パイプラインステートの設定
	_desc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT); // ラスタライザーはデフォルト
	_desc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE; // カリングはなし
	
	_desc.BlendState.AlphaToCoverageEnable = FALSE; // 4xMSAAのアルファトゥカバレッジは無効
	_desc.BlendState.IndependentBlendEnable = FALSE; // 独立ブレンドは無効
	D3D12_RENDER_TARGET_BLEND_DESC renderTargetBlendDesc = {};
	renderTargetBlendDesc.BlendEnable = FALSE; // ブレンドしない
	renderTargetBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL; // RGBA全てのチャンネルを描画
	renderTargetBlendDesc.LogicOpEnable = FALSE;
	_desc.BlendState.RenderTarget[0] = renderTargetBlendDesc;

	_desc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT); // 深度ステンシルはデフォルトを使う
	_desc.DepthStencilState.DepthEnable = TRUE;
	_desc.DepthStencilState.StencilEnable = FALSE;
	_desc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL; // 深度値は書き込む
	_desc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS; // 小さい値を採用

	_desc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	_desc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED; // ストリップカット無効
	_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE; // 三角形を描画
	_desc.NumRenderTargets = 1; // 描画対象は1
	_desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	_desc.DSVFormat = DXGI_FORMAT_D32_FLOAT;

	_desc.SampleDesc.Count = 1; // サンプラーは1
	_desc.SampleDesc.Quality = 0;

}

bool PipelineState::IsValid()
{
	return _isValid;
}

// 修正案: SetInputLayoutの引数と実装をD3D12_INPUT_LAYOUT_DESC構造体に合わせる
void PipelineState::SetInputLayout()
{
	_desc.InputLayout.pInputElementDescs = Vertex::InputLayout.data();
	_desc.InputLayout.NumElements = static_cast<UINT>(Vertex::InputLayout.size());
}    

void PipelineState::SetRootSignature(ID3D12RootSignature* rootSignature)
{
	_desc.pRootSignature = rootSignature;
}

void PipelineState::SetVS(std::wstring filePath)
{	
	ID3D10Blob* errorBlob = nullptr;
	// 頂点シェーダー読み込み
	auto hr = D3DCompileFromFile(
		filePath.c_str(),
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"VS", "vs_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		_vsBlob.GetAddressOf(),
		&errorBlob);

	if (FAILED(hr))
	{
		if(hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
		{
			SDL_Log("ファイルが見つかりません");
		}else{
			std::string error;
			error.resize(errorBlob->GetBufferSize());
			memcpy((void*)error.data(), errorBlob->GetBufferPointer(), errorBlob->GetBufferSize());
			error += "\n";
			SDL_Log(error.c_str());
		}
		SDL_Log("頂点シェーダーの読み込みに失敗");
		return;
	}

	_desc.VS.pShaderBytecode = _vsBlob->GetBufferPointer();
	_desc.VS.BytecodeLength = _vsBlob->GetBufferSize();
}
void PipelineState::SetPS(std::wstring filePath)
{
	ID3D10Blob* errorBlob = nullptr;
	// ピクセルシェーダー読み込み
	auto hr = D3DCompileFromFile(
		filePath.c_str(),
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"PS", "ps_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		_psBlob.GetAddressOf(),
		&errorBlob);

	if (FAILED(hr)) {
		if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
		{
			SDL_Log("ファイルが見つかりません");
		}
		else {
			std::string error;
			error.resize(errorBlob->GetBufferSize());
			memcpy((void*)error.data(), errorBlob->GetBufferPointer(), errorBlob->GetBufferSize());
			error += "\n";
			SDL_Log(error.c_str());
		}
		SDL_Log("ピクセルシェーダーの読み込みに失敗");
		return;
	}
	
	_desc.PS.pShaderBytecode = _psBlob->GetBufferPointer();
	_desc.PS.BytecodeLength = _psBlob->GetBufferSize();
}
void PipelineState::Create()
{
	// パイプラインステートを生成
	auto hr = DIRECT3D.Device().CreateGraphicsPipelineState(&_desc, IID_PPV_ARGS(&_pipelineState));
	if (FAILED(hr))
	{
		printf("パイプラインステートの生成に失敗 hr=0x%08X\n", static_cast<unsigned>(hr));
		return;
	}

	_isValid = true;
}

ID3D12PipelineState* PipelineState::Get()
{
	return _pipelineState.Get();
}