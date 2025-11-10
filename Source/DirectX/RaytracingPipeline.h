#pragma once
#include "DirectX/DirectX.h"

class RaytracingPipeline {
public:
    RaytracingPipeline();

    bool Initialize(ID3D12RootSignature* globalRootSig, ID3D12RootSignature* localRootSig);

    ID3D12StateObject* GetStateObject() { return _stateObject.Get(); }
    ID3D12StateObjectProperties* GetStateObjectProps() { return _stateObjectProps.Get(); }

    bool IsValid() const { return _isValid; }

private:
    bool _isValid = false;
    ComPtr<ID3D12StateObject> _stateObject;
    ComPtr<ID3D12StateObjectProperties> _stateObjectProps;
    ComPtr<IDxcBlob> _shaderBlob;

    bool CompileShaders();
};