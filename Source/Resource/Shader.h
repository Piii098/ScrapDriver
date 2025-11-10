#pragma once
#include "DirectX/DirectX.h"
#include "DirectX/Direct3D.h"
#include "Framework.h"
#include "Utility/Math.h"

class Shader {
public:
	Shader();
	~Shader();

	bool Load(const std::string& shaderPath);

	void Bind();
	
	// 行列を設定してシェーダーに送信
	void SetMatrices(const Matrix4x4& world, const Matrix4x4& view, const Matrix4x4& projection);

private:

	ComPtr<ID3D11VertexShader> _spriteVS;
	ComPtr<ID3D11PixelShader> _spritePS;
	ComPtr<ID3D11InputLayout> _spriteInputLayout;
	ComPtr<ID3D11Buffer> _matrixBuffer;  // 定数バッファ	

};