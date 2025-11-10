#include "DirectX/RaytracingRootSignature.h"

RaytracingRootSignature::RaytracingRootSignature()
    : _isValid(false)
{
}

bool RaytracingRootSignature::CreateGlobalRootSignature() {

    CD3DX12_DESCRIPTOR_RANGE uavRange;
    uavRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0); // u0

    CD3DX12_DESCRIPTOR_RANGE srvRange;
    srvRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1); // t1 (テクスチャ)

    CD3DX12_ROOT_PARAMETER rootParams[5]; // 4から5に増加
    rootParams[0].InitAsDescriptorTable(1, &uavRange);      // UAV (u0) - RenderTarget
    rootParams[1].InitAsShaderResourceView(0);              // SRV (t0) - SceneBVH
    rootParams[2].InitAsDescriptorTable(1, &srvRange);      // SRV (t1) - DiffuseTexture
    rootParams[3].InitAsConstantBufferView(0);              // CBV (b0) - Camera Data
    rootParams[4].InitAsConstantBufferView(1);              // CBV (b1) - Material Data // 新規追加
    
    // スタティックサンプラーを追加
    D3D12_STATIC_SAMPLER_DESC sampler = {};
    sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampler.MipLODBias = 0;
    sampler.MaxAnisotropy = 0;
    sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
    sampler.MinLOD = 0.0f;
    sampler.MaxLOD = D3D12_FLOAT32_MAX;
    sampler.ShaderRegister = 0; // s0
    sampler.RegisterSpace = 0;
    sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    
    CD3DX12_ROOT_SIGNATURE_DESC globalRootSigDesc;
    globalRootSigDesc.Init(_countof(rootParams), rootParams, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_NONE);

    ComPtr<ID3DBlob> blob;
    ComPtr<ID3DBlob> errorBlob;

    if(FAILED(D3D12SerializeRootSignature(&globalRootSigDesc
        , D3D_ROOT_SIGNATURE_VERSION_1
        , &blob
        , &errorBlob))) {
        if (errorBlob) {
            SDL_Log("Error: %s", static_cast<const char*>(errorBlob->GetBufferPointer()));
        }
        SDL_Log("Failed to serialize global root signature");
        return false;
    }

    if(FAILED(DIRECT3D.Device().CreateRootSignature(
        0,
        blob->GetBufferPointer(),
        blob->GetBufferSize(),
        IID_PPV_ARGS(&_globalRootSignature)))) {
        SDL_Log("Failed to create global root signature");
        return false;
    }

    SDL_Log("Global root signature created successfully");
    SDL_Log("  Root Parameter 0: UAV Descriptor Table (u0)");
    SDL_Log("  Root Parameter 1: SRV Root Descriptor (t0)");
    SDL_Log("  Root Parameter 2: SRV Descriptor Table (t1 - Texture)");
    SDL_Log("  Root Parameter 3: CBV Root Descriptor (b0 - Camera)");
    SDL_Log("  Root Parameter 4: CBV Root Descriptor (b1 - Material)");
    SDL_Log("  Static Sampler 0: s0");

    return true;
}

bool RaytracingRootSignature::CreateLocalRootSignature() {

    CD3DX12_ROOT_PARAMETER rootParam;
    rootParam.InitAsConstants(1, 0); // b0

    CD3DX12_ROOT_SIGNATURE_DESC localRootSigDesc;
    localRootSigDesc.Init(1, 
        &rootParam, 
        0, 
        nullptr, 
        D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE);

    ComPtr<ID3DBlob> blob;
    ComPtr<ID3DBlob> errorBlob;

    if(FAILED(D3D12SerializeRootSignature(&localRootSigDesc
        , D3D_ROOT_SIGNATURE_VERSION_1
        , &blob
        , &errorBlob))) {
        if (errorBlob) {
            SDL_Log("Error: %s", static_cast<const char*>(errorBlob->GetBufferPointer()));
        }
		SDL_Log("Failed to serialize local root signature");
        return false;
    }

    if(FAILED(DIRECT3D.Device().CreateRootSignature(
        0,
        blob->GetBufferPointer(),
        blob->GetBufferSize(),
        IID_PPV_ARGS(&_localRootSignature)))) {
        SDL_Log("Failed to create local root signature");
        return false;
    }

    return true;
}