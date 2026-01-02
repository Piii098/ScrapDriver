#pragma once
#include "DirectX/DirectX.h"

class ShadowPipelineState
{
public:
    ShadowPipelineState();
    bool Initialize(ID3D12RootSignature* rootSignature);
    bool IsValid() { return _isValid; }
    ID3D12PipelineState* Get() { return _pipelineState.Get(); }

private:
    bool _isValid = false;
    ComPtr<ID3D12PipelineState> _pipelineState = nullptr;
    ComPtr<ID3DBlob> _vsBlob;
};
