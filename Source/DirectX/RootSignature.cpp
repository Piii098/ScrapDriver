#include "DirectX/RootSignature.h"

RootSignature::RootSignature() {
}

bool RootSignature::Initialize() {
    // ルートパラメータの配列を作成
    D3D12_ROOT_PARAMETER rootParams[2] = {};
    
    // パラメータ0: テクスチャ (t0)
    D3D12_DESCRIPTOR_RANGE descRange0 = {};
    descRange0.NumDescriptors = 1;
    descRange0.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    descRange0.BaseShaderRegister = 0; // t0
    descRange0.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
    
    rootParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParams[0].DescriptorTable.NumDescriptorRanges = 1;
    rootParams[0].DescriptorTable.pDescriptorRanges = &descRange0;
    rootParams[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    
    // パラメータ1: 定数バッファ (b0)
    D3D12_DESCRIPTOR_RANGE descRange1 = {};
    descRange1.NumDescriptors = 1;
    descRange1.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
    descRange1.BaseShaderRegister = 0; // b0
    descRange1.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
    
    rootParams[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParams[1].DescriptorTable.NumDescriptorRanges = 1;
    rootParams[1].DescriptorTable.pDescriptorRanges = &descRange1;
    rootParams[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
    
    // サンプラーの設定
    D3D12_STATIC_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    samplerDesc.ShaderRegister = 0; // s0
    samplerDesc.RegisterSpace = 0;
    samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    
    // ルートシグネチャの設定
    D3D12_ROOT_SIGNATURE_DESC rootSigDesc = {};
    rootSigDesc.NumParameters = 2;
    rootSigDesc.pParameters = rootParams;
    rootSigDesc.NumStaticSamplers = 1;
    rootSigDesc.pStaticSamplers = &samplerDesc;
    rootSigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
    
    ComPtr<ID3DBlob> rootSigBlob;
    ComPtr<ID3DBlob> errorBlob;
    
    auto hr = D3D12SerializeRootSignature(
        &rootSigDesc,
        D3D_ROOT_SIGNATURE_VERSION_1,
        &rootSigBlob,
        &errorBlob);
    
    if (FAILED(hr)) {
        if (errorBlob) {
            SDL_Log("Root Signature Error: %s", (char*)errorBlob->GetBufferPointer());
        }
        return false;
    }
    
    hr = DIRECT3D.Device().CreateRootSignature(
        0,
        rootSigBlob->GetBufferPointer(),
        rootSigBlob->GetBufferSize(),
        IID_PPV_ARGS(&_rootSignature));
    
    if (FAILED(hr)) {
        SDL_Log("Failed to create root signature");
        return false;
    }
    
    _isValid = true;
    return true;
}

bool RootSignature::IsValid()
{
	return _isValid;
}

ID3D12RootSignature* RootSignature::Get()
{
	return _rootSignature.Get();
}