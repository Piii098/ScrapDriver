#pragma once
#include "DirectX/DirectX.h"
#include <vector>

class DescriptorHandle
{
public:
	D3D12_CPU_DESCRIPTOR_HANDLE HandleCPU;
	D3D12_GPU_DESCRIPTOR_HANDLE HandleGPU;
};

class DescriptorHeap
{
public:
	DescriptorHeap();
	ID3D12DescriptorHeap* GetHeap();
	DescriptorHandle* Register(class Texture2D* texture);

	bool IsValid() const { return _isValid; }

private:
	bool _isValid = false;
	UINT _incrementSize = 0;
	ComPtr<ID3D12DescriptorHeap> _heap = nullptr;
	std::vector<DescriptorHandle*> _handles;

};