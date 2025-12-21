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

    // 固定連続領域（レイトレ用テクスチャ配列/頂点位置配列/頂点法線配列）を避けて、通常割り当ての開始位置を後ろにずらす
    _currentSVRIndex = 800;
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

UINT DescriptorManager::RegisterRaytracingTexture(ID3D12Resource* texture, DXGI_FORMAT format) {
    // レイトレーシング用テクスチャ配列の範囲をチェック
    if (_raytracingTextureCount >= RAYTRACING_TEXTURE_ARRAY_SIZE) {
        SDL_Log("Error: Raytracing texture array is full (max: %d)", RAYTRACING_TEXTURE_ARRAY_SIZE);
        return static_cast<UINT>(-1);
    }
    
    // 連続した領域に配置
    UINT index = RAYTRACING_TEXTURE_ARRAY_START + _raytracingTextureCount;
    
    D3D12_CPU_DESCRIPTOR_HANDLE handle = _heap->GetCPUDescriptorHandleForHeapStart();
    handle.ptr += _descriptorSize * index;
    
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = format;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;
    
    DIRECT3D.Device().CreateShaderResourceView(
        texture,
        &srvDesc,
        handle);
    
    UINT relativeIndex = _raytracingTextureCount;
    _raytracingTextureCount++;
    
    SDL_Log("[RT_TEXTURE] Registered at absolute=%d, relative=%d, ptr=%p", 
        index, relativeIndex, (void*)texture);
    return relativeIndex;  // 配列内での相対インデックスを返す
}

UINT DescriptorManager::RegisterRaytracingVertexPositionBuffer(ID3D12Resource* vertexPositionBuffer, UINT vertexCount) {
    if (_raytracingVertexPositionCount >= RAYTRACING_VERTEX_POSITION_ARRAY_SIZE) {
        SDL_Log("Error: Raytracing vertex position array is full (max: %d)", RAYTRACING_VERTEX_POSITION_ARRAY_SIZE);
        return static_cast<UINT>(-1);
    }

    UINT index = RAYTRACING_VERTEX_POSITION_ARRAY_START + _raytracingVertexPositionCount;

    D3D12_CPU_DESCRIPTOR_HANDLE handle = _heap->GetCPUDescriptorHandleForHeapStart();
    handle.ptr += _descriptorSize * index;

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Buffer.FirstElement = 0;
    srvDesc.Buffer.NumElements = vertexCount;
    srvDesc.Buffer.StructureByteStride = sizeof(float) * 3;
    srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

    DIRECT3D.Device().CreateShaderResourceView(
        vertexPositionBuffer,
        &srvDesc,
        handle);

    UINT relativeIndex = _raytracingVertexPositionCount;
    _raytracingVertexPositionCount++;

    SDL_Log("[RT_VB] Registered vertex position buffer at absolute=%d, relative=%d, vertices=%u, ptr=%p",
        index, relativeIndex, vertexCount, (void*)vertexPositionBuffer);
    return relativeIndex;
}

UINT DescriptorManager::RegisterRaytracingVertexNormalBuffer(ID3D12Resource* vertexNormalBuffer, UINT vertexCount) {
    if (_raytracingVertexNormalCount >= RAYTRACING_VERTEX_NORMAL_ARRAY_SIZE) {
        SDL_Log("Error: Raytracing vertex normal array is full (max: %d)", RAYTRACING_VERTEX_NORMAL_ARRAY_SIZE);
        return static_cast<UINT>(-1);
    }

    UINT index = RAYTRACING_VERTEX_NORMAL_ARRAY_START + _raytracingVertexNormalCount;

    D3D12_CPU_DESCRIPTOR_HANDLE handle = _heap->GetCPUDescriptorHandleForHeapStart();
    handle.ptr += _descriptorSize * index;

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Buffer.FirstElement = 0;
    srvDesc.Buffer.NumElements = vertexCount;
    srvDesc.Buffer.StructureByteStride = sizeof(float) * 3;
    srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

    DIRECT3D.Device().CreateShaderResourceView(
        vertexNormalBuffer,
        &srvDesc,
        handle);

    UINT relativeIndex = _raytracingVertexNormalCount;
    _raytracingVertexNormalCount++;

    SDL_Log("[RT_NB] Registered vertex normal buffer at absolute=%d, relative=%d, vertices=%u, ptr=%p",
        index, relativeIndex, vertexCount, (void*)vertexNormalBuffer);
    return relativeIndex;
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