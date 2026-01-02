#include "DirectX/ShadowRootSignature.h"
#include "DirectX/Direct3D.h"
#include <SDL3/SDL.h>

ShadowRootSignature::ShadowRootSignature() {
}

bool ShadowRootSignature::Initialize() {
    // Shadow pass only needs light view-projection matrix (b0) and world matrix (b1)
    D3D12_ROOT_PARAMETER rootParams[2] = {};

    // Parameter 0: Light view-projection matrix (b0)
    D3D12_DESCRIPTOR_RANGE descRange0 = {};
    descRange0.NumDescriptors = 1;
    descRange0.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
    descRange0.BaseShaderRegister = 0; // b0
    descRange0.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    rootParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParams[0].DescriptorTable.NumDescriptorRanges = 1;
    rootParams[0].DescriptorTable.pDescriptorRanges = &descRange0;
    rootParams[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

    // Parameter 1: World matrix (root constants b1)
    rootParams[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
    rootParams[1].Constants.Num32BitValues = 16; // Matrix 4x4
    rootParams[1].Constants.ShaderRegister = 1; // b1
    rootParams[1].Constants.RegisterSpace = 0;
    rootParams[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

    // Root signature description
    D3D12_ROOT_SIGNATURE_DESC rootSigDesc = {};
    rootSigDesc.NumParameters = 2;
    rootSigDesc.pParameters = rootParams;
    rootSigDesc.NumStaticSamplers = 0;
    rootSigDesc.pStaticSamplers = nullptr;
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
            SDL_Log("Shadow Root Signature Error: %s", (char*)errorBlob->GetBufferPointer());
        }
        return false;
    }

    hr = DIRECT3D.Device().CreateRootSignature(
        0,
        rootSigBlob->GetBufferPointer(),
        rootSigBlob->GetBufferSize(),
        IID_PPV_ARGS(&_rootSignature));

    if (FAILED(hr)) {
        SDL_Log("Failed to create shadow root signature");
        return false;
    }

    _isValid = true;
    return true;
}
