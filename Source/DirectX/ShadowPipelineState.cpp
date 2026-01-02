#include "DirectX/ShadowPipelineState.h"
#include "DirectX/Direct3D.h"
#include "DirectX/ShaderStruct.h"
#include <SDL3/SDL.h>
#include <d3dcompiler.h>

ShadowPipelineState::ShadowPipelineState() {
}

bool ShadowPipelineState::Initialize(ID3D12RootSignature* rootSignature) {
    // Compile shadow vertex shader
    ID3D10Blob* errorBlob = nullptr;
    auto hr = D3DCompileFromFile(
        L"Assets/Shader/ShadowVS.hlsl",
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "main", "vs_5_0",
        D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
        0,
        _vsBlob.GetAddressOf(),
        &errorBlob);

    if (FAILED(hr)) {
        if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) {
            SDL_Log("Shadow VS file not found");
        } else if (errorBlob) {
            std::string error;
            error.resize(errorBlob->GetBufferSize());
            memcpy((void*)error.data(), errorBlob->GetBufferPointer(), errorBlob->GetBufferSize());
            SDL_Log("Shadow VS compile error: %s", error.c_str());
        }
        return false;
    }

    // Create pipeline state for shadow pass
    D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
    desc.pRootSignature = rootSignature;
    
    // Input layout (only position needed for shadows)
    desc.InputLayout.pInputElementDescs = Vertex::InputLayout.data();
    desc.InputLayout.NumElements = static_cast<UINT>(Vertex::InputLayout.size());
    
    // Vertex shader
    desc.VS.pShaderBytecode = _vsBlob->GetBufferPointer();
    desc.VS.BytecodeLength = _vsBlob->GetBufferSize();
    
    // No pixel shader for depth-only pass
    desc.PS.pShaderBytecode = nullptr;
    desc.PS.BytecodeLength = 0;
    
    // Rasterizer state
    desc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    desc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK; // Back-face culling for shadows
    desc.RasterizerState.DepthBias = 100; // Depth bias to reduce shadow acne
    desc.RasterizerState.DepthBiasClamp = 0.0f;
    desc.RasterizerState.SlopeScaledDepthBias = 1.0f;
    
    // Blend state (no color output)
    desc.BlendState.AlphaToCoverageEnable = FALSE;
    desc.BlendState.IndependentBlendEnable = FALSE;
    D3D12_RENDER_TARGET_BLEND_DESC renderTargetBlendDesc = {};
    renderTargetBlendDesc.BlendEnable = FALSE;
    renderTargetBlendDesc.RenderTargetWriteMask = 0; // No color writes
    renderTargetBlendDesc.LogicOpEnable = FALSE;
    desc.BlendState.RenderTarget[0] = renderTargetBlendDesc;
    
    // Depth stencil state
    desc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    desc.DepthStencilState.DepthEnable = TRUE;
    desc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    desc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
    desc.DepthStencilState.StencilEnable = FALSE;
    
    desc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
    desc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
    desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    
    // No render targets, only depth
    desc.NumRenderTargets = 0;
    desc.RTVFormats[0] = DXGI_FORMAT_UNKNOWN;
    desc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    
    hr = DIRECT3D.Device().CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&_pipelineState));
    if (FAILED(hr)) {
        SDL_Log("Failed to create shadow pipeline state");
        return false;
    }
    
    _isValid = true;
    SDL_Log("Shadow pipeline state created successfully");
    return true;
}
