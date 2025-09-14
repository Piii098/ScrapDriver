#include "DescriptorHeap.h"
#include "Texture2D.h"
#include <d3dx12.h>

const UINT HANDLE_MAX = 512;

DescriptorHeap::DescriptorHeap()
{
	_handles.clear();
	_handles.reserve(HANDLE_MAX);

	D3D12_DESCRIPTOR_HEAP_DESC desc{};
	desc.NodeMask = 1;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	desc.NumDescriptors = HANDLE_MAX;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	auto& device = DIRECT3D.Device();

	// ディスクリプタヒープを生成
	auto hr = device.CreateDescriptorHeap(
		&desc,
		IID_PPV_ARGS(_heap.ReleaseAndGetAddressOf()));

	if (FAILED(hr))
	{
		_isValid = false;
		return;
	}

	_incrementSize = device.GetDescriptorHandleIncrementSize(desc.Type); // ディスクリプタヒープ1個のメモリサイズを返す
	_isValid = true;
}

ID3D12DescriptorHeap* DescriptorHeap::GetHeap()
{
	return _heap.Get();
}

DescriptorHandle* DescriptorHeap::Register(Texture2D* texture)
{
	auto count = _handles.size();
	if (HANDLE_MAX <= count)
	{
		return nullptr;
	}

	DescriptorHandle* pHandle = new DescriptorHandle();

	auto handleCPU = _heap->GetCPUDescriptorHandleForHeapStart(); // ディスクリプタヒープの最初のアドレス
	handleCPU.ptr += _incrementSize * count; // 最初のアドレスからcount番目が今回追加されたリソースのハンドル

	auto handleGPU = _heap->GetGPUDescriptorHandleForHeapStart(); // ディスクリプタヒープの最初のアドレス
	handleGPU.ptr += _incrementSize * count; // 最初のアドレスからcount番目が今回追加されたリソースのハンドル

	pHandle->HandleCPU = handleCPU;
	pHandle->HandleGPU = handleGPU;

	auto& device = DIRECT3D.Device();
	auto resource = texture->Resource();
	auto desc = texture->ViewDesc();
	device.CreateShaderResourceView(resource, &desc, pHandle->HandleCPU); // シェーダーリソースビュー作成

	_handles.push_back(pHandle);
	return pHandle; // ハンドルを返す
}