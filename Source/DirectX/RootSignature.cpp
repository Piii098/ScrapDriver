#include "DirectX/RootSignature.h"

RootSignature::RootSignature() {
}

bool RootSignature::Initialize() {
    // ルートパラメータの配列を作成
    D3D12_ROOT_PARAMETER rootParams[6] = {};

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

    // パラメータ1: シーン定数バッファ (b0) : View / Projection など
    D3D12_DESCRIPTOR_RANGE descRange1 = {};
    descRange1.NumDescriptors = 1;
    descRange1.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
    descRange1.BaseShaderRegister = 0; // b0
    descRange1.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    rootParams[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParams[1].DescriptorTable.NumDescriptorRanges = 1;
    rootParams[1].DescriptorTable.pDescriptorRanges = &descRange1;
    rootParams[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    // パラメータ2: ワールド行列（ルート定数 b1）
    // 32bit定数 16個 = 64byte (matrix 4x4)
    rootParams[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
    rootParams[2].Constants.Num32BitValues = 16; // 行列分
    rootParams[2].Constants.ShaderRegister = 1; // b1 として扱う
    rootParams[2].Constants.RegisterSpace = 0;
    rootParams[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

    // パラメータ3: マテリアル色（ルート定数 b2）
    // 32bit定数 8個 = 32byte (float4 baseColor + metallic/roughness/reflectance + pad)
    rootParams[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
    rootParams[3].Constants.Num32BitValues = 8;
    rootParams[3].Constants.ShaderRegister = 2; // b2
    rootParams[3].Constants.RegisterSpace = 0;
    rootParams[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    // パラメータ4: シャドウマップ (t1)
    D3D12_DESCRIPTOR_RANGE descRange4 = {};
    descRange4.NumDescriptors = 1;
    descRange4.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    descRange4.BaseShaderRegister = 1; // t1
    descRange4.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    rootParams[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParams[4].DescriptorTable.NumDescriptorRanges = 1;
    rootParams[4].DescriptorTable.pDescriptorRanges = &descRange4;
    rootParams[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    // パラメータ5: ライト定数バッファ (b3)
    D3D12_DESCRIPTOR_RANGE descRange5 = {};
    descRange5.NumDescriptors = 1;
    descRange5.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
    descRange5.BaseShaderRegister = 3; // b3
    descRange5.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    rootParams[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParams[5].DescriptorTable.NumDescriptorRanges = 1;
    rootParams[5].DescriptorTable.pDescriptorRanges = &descRange5;
    rootParams[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    
    // サンプラーの設定
    D3D12_STATIC_SAMPLER_DESC samplers[2] = {};
    
    // s0: 通常のサンプラー
    samplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    samplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    samplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    samplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    samplers[0].ShaderRegister = 0; // s0
    samplers[0].RegisterSpace = 0;
    samplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    
    // s1: シャドウマップ用比較サンプラー
    samplers[1].Filter = D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
    samplers[1].AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    samplers[1].AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    samplers[1].AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    samplers[1].ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
    samplers[1].BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
    samplers[1].ShaderRegister = 1; // s1
    samplers[1].RegisterSpace = 0;
    samplers[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    
    // ルートシグネチャの設定
    D3D12_ROOT_SIGNATURE_DESC rootSigDesc = {};
    rootSigDesc.NumParameters = 6;
    rootSigDesc.pParameters = rootParams;
    rootSigDesc.NumStaticSamplers = 2;
    rootSigDesc.pStaticSamplers = samplers;
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