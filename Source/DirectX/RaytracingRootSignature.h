#pragma once
#include "DirectX/DirectX.h"

class RaytracingRootSignature {
public:
    RaytracingRootSignature();
    ~RaytracingRootSignature() = default;

    bool CreateGlobalRootSignature();
    bool CreateLocalRootSignature();

    ID3D12RootSignature* GlobalRootSignature() const { return _globalRootSignature.Get(); }
    ID3D12RootSignature* LocalRootSignature() const { return _localRootSignature.Get(); }
    bool IsValid() const { return _isValid; }
private:
    bool _isValid;
    ComPtr<ID3D12RootSignature> _globalRootSignature;
    ComPtr<ID3D12RootSignature> _localRootSignature;
};