#pragma once
#include "DirectX/DirectX.h"

class ShadowRootSignature
{
public:
	ShadowRootSignature();
	bool Initialize();
	bool IsValid() { return _isValid; }
	ID3D12RootSignature* Get() { return _rootSignature.Get(); }

private:
	bool _isValid = false;
	ComPtr<ID3D12RootSignature> _rootSignature = nullptr;
};
