#include "DirectX/DescriptorHeap.h"
#include "DirectX/Texture2D.h"
#include <d3dx12.h>

DescriptorManager* DescriptorManager::s_instance = nullptr;

bool DescriptorManager::Initialize() {
    D3D12_DESCRIPTOR_HEAP_DESC desc = {};
    desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    desc.NumDescriptors = MAX_DESCRIPTORS;  // 2ではなくMAX_DESCRIPTORSに変更
    desc.NodeMask = 0;
    desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

    if (FAILED(DIRECT3D.Device().CreateDescriptorHeap(
        &desc,
        IID_PPV_ARGS(&_heap)))) {
        printf("Error: Failed to create descriptor heap.\n");
        return false;
    }

    _cpuHandleStart = _heap->GetCPUDescriptorHandleForHeapStart();

    _descriptorSize = DIRECT3D.Device().GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    return true;
}

UINT DescriptorManager::ResisterTexture(ID3D12Resource* texture, DXGI_FORMAT format) {
    if (_currentSVRIndex >= MAX_DESCRIPTORS) {
        SDL_Log("Error: Descriptor heap is full (index: %d)", _currentSVRIndex);
        return -1;
    }

    // 現在のインデックス位置のハンドルを計算
    D3D12_CPU_DESCRIPTOR_HANDLE handle = _heap->GetCPUDescriptorHandleForHeapStart();
    handle.ptr += _descriptorSize * _currentSVRIndex;

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = format;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;

    DIRECT3D.Device().CreateShaderResourceView(
        texture,
        &srvDesc,
        handle);

    UINT index = _currentSVRIndex;
    _currentSVRIndex++;
    SDL_Log("Registered texture at index: %d", index);
    return index;
}

UINT DescriptorManager::ResisterConstantBuffer(ID3D12Resource* constBuffer) {
    if (_currentSVRIndex >= MAX_DESCRIPTORS) {
        SDL_Log("Error: Descriptor heap is full (index: %d)", _currentSVRIndex);
        return -1;
    }

    // 現在のインデックス位置のハンドルを計算
    D3D12_CPU_DESCRIPTOR_HANDLE handle = _heap->GetCPUDescriptorHandleForHeapStart();
    handle.ptr += _descriptorSize * _currentSVRIndex;

    D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
    cbvDesc.BufferLocation = constBuffer->GetGPUVirtualAddress();
    cbvDesc.SizeInBytes = static_cast<UINT>(constBuffer->GetDesc().Width);

    DIRECT3D.Device().CreateConstantBufferView(
        &cbvDesc,
        handle);

    UINT index = _currentSVRIndex;
    _currentSVRIndex++;
    SDL_Log("Registered constant buffer at index: %d", index);
    return index;
}

D3D12_GPU_DESCRIPTOR_HANDLE DescriptorManager::GetGPUHandle() {
    D3D12_GPU_DESCRIPTOR_HANDLE handle = _heap->GetGPUDescriptorHandleForHeapStart();
    return handle;
}

D3D12_GPU_DESCRIPTOR_HANDLE DescriptorManager::GetGPUHandleAt(UINT index) {
    D3D12_GPU_DESCRIPTOR_HANDLE handle = _heap->GetGPUDescriptorHandleForHeapStart();
    handle.ptr += _descriptorSize * index;
    return handle;
}

D3D12_CPU_DESCRIPTOR_HANDLE DescriptorManager::GetCPUHandleAt(UINT index) {
    D3D12_CPU_DESCRIPTOR_HANDLE handle = _heap->GetCPUDescriptorHandleForHeapStart();
    handle.ptr += _descriptorSize * index;
    return handle;
}

void DescriptorManager::BindDescriptorHeaps() {
	ID3D12DescriptorHeap* heaps[] = { _heap.Get() };
	DIRECT3D.CommandList().SetDescriptorHeaps(_countof(heaps), heaps);
}

DescriptorHeap::DescriptorHeap()
{
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	heapDesc.NodeMask = 0;
	heapDesc.NumDescriptors = 2;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	auto hr = DIRECT3D.Device().CreateDescriptorHeap(
		&heapDesc,
		IID_PPV_ARGS(&_heap)
	);

	if (FAILED(hr)) {
		printf("Error: Failed to create descriptor heap. HRESULT = 0x%08X\n", hr);
		return;
	}

	return;

	//_handles.clear();
	//_handles.reserve(HANDLE_MAX);

	//D3D12_DESCRIPTOR_HEAP_DESC desc{};
	//desc.NodeMask = 1;
	//desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	//desc.NumDescriptors = HANDLE_MAX;
	//desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	//auto& device = DIRECT3D.Device();

	//// ディスクリプタヒープを生成
	//auto hr = device.CreateDescriptorHeap(
	//	&desc,
	//	IID_PPV_ARGS(_heap.ReleaseAndGetAddressOf()));

	//if (FAILED(hr))
	//{
	//	_isValid = false;
	//	return;
	//}

	//_incrementSize = device.GetDescriptorHandleIncrementSize(desc.Type); // ディスクリプタヒープ1個のメモリサイズを返す
	//_isValid = true;
}

//ID3D12DescriptorHeap* DescriptorHeap::GetHeap()
//{
//	return _heap.Get();
//}
//
//void DescriptorHeap::Register(Texture2D* texture)
//{
//	//DescriptorHandle* pHandle = new DescriptorHandle();
//
//	auto handleCPU = _heap->GetCPUDescriptorHandleForHeapStart();
//	handleCPU.ptr += DIRECT3D.Device().GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
//
//	//auto handleGPU = _heap->GetGPUDescriptorHandleForHeapStart(); // ディスクリプタヒープの最初のアドレス
//	//handleGPU.ptr += _incrementSize * count; // 最初のアドレスからcount番目が今回追加されたリソースのハンドル
//
//	//pHandle->HandleCPU =/ handleCPU;
//	//pHandle->HandleGPU = handleGPU;
//
//	// auto resource = texture->Resource();
//	// auto desc = texture->ViewDesc();
//
//	DIRECT3D.Device().CreateShaderResourceView(
//		texture->GetTexture(),
//		&texture->GetViewDesc(),
//		handleCPU
//	); // シェーダーリソースビュー作成
//
//}