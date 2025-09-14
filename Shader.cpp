#include "Shader.h"
#include "Vertex.h"

Shader::Shader() {

}

Shader::~Shader() {
}

bool Shader::Load(const std::string& shaderPath) {

	std::wstring wShaderPath(shaderPath.begin(), shaderPath.end());

	ComPtr<ID3DBlob> compiledVS;
	if (FAILED(D3DCompileFromFile(wShaderPath.c_str(), nullptr, nullptr, "VSMain", "vs_5_0", 0, 0, &compiledVS, nullptr)))
	{
		return false;
	}

	ComPtr<ID3DBlob> compiledPS;
	if (FAILED(D3DCompileFromFile(wShaderPath.c_str(), nullptr, nullptr, "PSMain", "ps_5_0", 0, 0, &compiledPS, nullptr)))
	{
		return false;
	}

	if (FAILED(DIRECT3D.Device().CreateVertexShader(compiledVS->GetBufferPointer(), compiledVS->GetBufferSize(), nullptr, &_spriteVS)))
	{
		return false;
	}
	
	if (FAILED(DIRECT3D.Device().CreatePixelShader(compiledPS->GetBufferPointer(), compiledPS->GetBufferSize(), nullptr, &_spritePS)))
	{
		return false;
	}

	std::vector<D3D11_INPUT_ELEMENT_DESC> layout = Vertex::GetInputLayout();

	if (FAILED(DIRECT3D.Device().CreateInputLayout(&layout[0], layout.size(), compiledVS->GetBufferPointer(), compiledVS->GetBufferSize(), &_spriteInputLayout)))
	{
		return false;
	}

	// 定数バッファの作成
	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = sizeof(MatrixBuffer);
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	
	if (FAILED(DIRECT3D.Device().CreateBuffer(&bufferDesc, nullptr, &_matrixBuffer)))
	{
		return false;
	}

	return true;
}

void Shader::Bind() {

	DIRECT3D.Context().VSSetShader(_spriteVS.Get(), nullptr, 0);
	DIRECT3D.Context().PSSetShader(_spritePS.Get(), nullptr, 0);

	DIRECT3D.Context().IASetInputLayout(_spriteInputLayout.Get());

}

void Shader::SetMatrices(const Matrix4x4& world, const Matrix4x4& view, const Matrix4x4& projection) {
	
	// 定数バッファにデータを書き込み
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	if (SUCCEEDED(DIRECT3D.Context().Map(_matrixBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
	{
		MatrixBuffer* dataPtr = (MatrixBuffer*)mappedResource.pData;
		
		// DirectXMathでは、シェーダーに渡す前に行列を転置する必要があります
		dataPtr->world = XMMatrixTranspose(world);
		dataPtr->view = XMMatrixTranspose(view);
		dataPtr->projection = XMMatrixTranspose(projection);

		DIRECT3D.Context().Unmap(_matrixBuffer.Get(), 0);
		
		// 定数バッファを頂点シェーダーにバインド
		DIRECT3D.Context().VSSetConstantBuffers(0, 1, _matrixBuffer.GetAddressOf());
	}
}