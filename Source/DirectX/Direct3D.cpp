#include "DirectX/Direct3D.h"
#include "Core/SceneManager.h"
#include "DirectX/PipelineState.h"

Direct3D* Direct3D::s_instance = nullptr;

Direct3D::Direct3D()
	: _window(nullptr) {
}

Direct3D::~Direct3D() {
}

bool Direct3D::Initialize() {

	SDL_WindowFlags flags = SDL_WINDOW_RESIZABLE;
	_window = SDL_CreateWindow("test", _width, _height, flags);

	_hWnd = (HWND)SDL_GetPointerProperty(SDL_GetWindowProperties(_window), SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);

	SDL_GetWindowSize(_window, &_width, &_height);

#ifdef _DEBUG

	EnableDebugLayer();

#endif

	if(!CreateDXGIFactory()) {
		return false;
	}

	if (!EnumerateAdapters()) {
		return false;
	}

	if (!CreateDevice()) {
		return false;
	}

	if (!CreateCommandQueue()) {
		return false;
	}
	
	if(!CreateSwapChain()) {
		return false;
	}

	if (!CreateViewPort()) {
		return false;
	}

	if (!CreateScissorRect()) {
		return false;
	}

	if (!CreateRenderTarget()) {
		return false;
	}

	if (!CreateDepthStencil()) {
		return false;
	}	

	if (!CreateCommandAllocatorAndList()) {
		return false;
	}

	if (!CreateFence()) {
		return false;
	}

	if(!InitializeRayTracing()) {
		return false;
	}

	return true;
}

void Direct3D::EnableDebugLayer() {
	ID3D12Debug* debugController;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
		debugController->EnableDebugLayer();
	}
	debugController->Release();
}	

bool Direct3D::CreateDevice() {

	if (FAILED(D3D12CreateDevice(
				_selectedAdapter.Get(),
				D3D_FEATURE_LEVEL_12_0,
				IID_PPV_ARGS(&_device)))) {
		return false;
	}

	return true;
}

bool Direct3D::CreateDXGIFactory() {
#ifdef _DEBUG
	if(FAILED(CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, 
								 IID_PPV_ARGS(&_dxgiFactory)))) {
		return false;
#else
	if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&_dxgiFactory)))) {
#endif
		return false;
	}
	return true;
}

bool Direct3D::EnumerateAdapters() {
	IDXGIAdapter* adapter = nullptr;
	for (UINT i = 0; _dxgiFactory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND; i++) {
		_adapters.push_back(adapter);
	}

	for (auto adpt : _adapters) {
		DXGI_ADAPTER_DESC desc = {};
		adpt->GetDesc(&desc);
		std::wstring wstrDesc = desc.Description;
		std::string strDesc(wstrDesc.begin(), wstrDesc.end());

		SDL_Log("Adapter %d: %s", _adapters.size(), strDesc.c_str());

		if (wstrDesc.find(L"NVIDIA") != std::string::npos) {
			SDL_Log("Selected Adapter: %s", strDesc.c_str());
			_selectedAdapter = adpt;
		}
	}
	return true;
}
                                        
bool Direct3D::CreateCommandQueue() {
	D3D12_COMMAND_QUEUE_DESC  desc = {};
	desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;                                                               
	desc.NodeMask = 0;

	if (FAILED(_device->CreateCommandQueue(
						&desc, 
						IID_PPV_ARGS(_queue.ReleaseAndGetAddressOf())))) {
		return false;
	}

	return true;
}

bool Direct3D::CreateSwapChain() {

	DXGI_SWAP_CHAIN_DESC desc = {};
	desc.BufferDesc.Width = _width;
	desc.BufferDesc.Height = _height;
	desc.BufferDesc.RefreshRate.Numerator = 60;
	desc.BufferDesc.RefreshRate.Denominator = 1;
	desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	desc.BufferCount = FRAME_BUFFER_COUNT;
	desc.OutputWindow = _hWnd;
	desc.Windowed = TRUE;
	desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;		// フリップ後に破棄
	desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;	// フルスクリーン切り替えを許可

	IDXGISwapChain* swapChain = nullptr;
	if (FAILED(_dxgiFactory->CreateSwapChain(
		_queue.Get(),
		&desc,
		&swapChain))) {
		return false;
	}
	if (FAILED(swapChain->QueryInterface(IID_PPV_ARGS(&_swapChain)))) {
		swapChain->Release();
		return false;
	}

	_currentBackBufferIndex = _swapChain->GetCurrentBackBufferIndex();

	swapChain->Release();

	return true;
}

bool Direct3D::CreateCommandAllocatorAndList() {
	for (int i = 0; i < FRAME_BUFFER_COUNT; i++) {
		if (FAILED(_device->CreateCommandAllocator(
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			IID_PPV_ARGS(&_allocator[i])))) {
			return false;
		}
	}

	// 通常のコマンドリストを作成
	ComPtr<ID3D12GraphicsCommandList> tempCommandList;
	if (FAILED(_device->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		_allocator[0].Get(),
		nullptr,
		IID_PPV_ARGS(&tempCommandList)))) {
		SDL_Log("Failed to create command list.");
		return false;
	}

	// ID3D12GraphicsCommandList4にアップグレード
	if (FAILED(tempCommandList->QueryInterface(IID_PPV_ARGS(&_commandList)))) {
		SDL_Log("Failed to get ID3D12GraphicsCommandList4 interface.");
		return false;
	}

	_commandList->Close();

	return true;
}

bool Direct3D::CreateFence() {
	
	for (int i = 0; i < FRAME_BUFFER_COUNT; i++) {
		_fenceValue[i] = 0;
	}

	if (FAILED(_device->CreateFence(
		0,
		D3D12_FENCE_FLAG_NONE,
		IID_PPV_ARGS(&_fence)))) {
		return false;
	}

	_fenceValue[_currentBackBufferIndex]++;

	_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

	return _fenceEvent != nullptr;
}

bool Direct3D::CreateViewPort() {
	_viewport.TopLeftX = 0;
	_viewport.TopLeftY = 0;
	_viewport.Width = static_cast<float>(_width);
	_viewport.Height = static_cast<float>(_height);
	_viewport.MinDepth = 0.0f;
	_viewport.MaxDepth = 1.0f;
	return true;
}

bool Direct3D::CreateScissorRect() {
	_scissorRect.left = 0;
	_scissorRect.top = 0;
	_scissorRect.right = static_cast<LONG>(_width);
	_scissorRect.bottom = static_cast<LONG>(_height);
	return true;
}

bool Direct3D::CreateRenderTarget() {

	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.NumDescriptors = FRAME_BUFFER_COUNT;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	if (FAILED(_device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&_rtvHeap)))) {
		return false;
	}

	_rtvDescriptorSize = _device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = _rtvHeap->GetCPUDescriptorHandleForHeapStart();

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; // ガンマ補正
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	for (int i = 0; i < FRAME_BUFFER_COUNT; i++) {
		_swapChain->GetBuffer(i, IID_PPV_ARGS(_renderTargets[i].ReleaseAndGetAddressOf()));
		_device->CreateRenderTargetView(_renderTargets[i].Get(), &rtvDesc, rtvHandle);
		rtvHandle.ptr += _rtvDescriptorSize;
	}
	
	return true;
}

bool Direct3D::CreateDepthStencil() {
	D3D12_RESOURCE_DESC desc = {};
	desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	desc.Alignment = 0;
	desc.Width = _width;
	desc.Height = _height;
	desc.DepthOrArraySize = 1;
	desc.MipLevels = 1;
	desc.Format = DXGI_FORMAT_D32_FLOAT;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_HEAP_PROPERTIES heapProps = {};
	heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
	heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	D3D12_CLEAR_VALUE clearValue = {};
	clearValue.DepthStencil.Depth = 1.0f;
	clearValue.Format = DXGI_FORMAT_D32_FLOAT;
	
	if(FAILED(_device->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&clearValue,
		IID_PPV_ARGS(&_depthStencilBuffer)))) {
		SDL_Log("Failed to create depth stencil buffer.");
		return false;
	}	

	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;

	if(FAILED(_device->CreateDescriptorHeap(
		&dsvHeapDesc,
		IID_PPV_ARGS(&_dsvHeap)))) {
		SDL_Log("Failed to create DSV descriptor heap.");
		return false;
	}

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

	_device->CreateDepthStencilView(
		_depthStencilBuffer.Get(),
		&dsvDesc,
		_dsvHeap->GetCPUDescriptorHandleForHeapStart());

	return true;
}

bool Direct3D::InitializeRayTracing(){

	if(!CheckRayTracingSupport()){
		return false;
	}

	if (FAILED(_device->QueryInterface(IID_PPV_ARGS(&_device)))) {
		SDL_Log("Failed to get ID3D12Device5 interface.");
		return false;
	}
	SDL_Log("Raytracing interfaces initialized successfully");
	return true;
}

bool Direct3D::CheckRayTracingSupport(){
	D3D12_FEATURE_DATA_D3D12_OPTIONS5 options5 = {};

	HRESULT hr = _device->CheckFeatureSupport(
		D3D12_FEATURE_D3D12_OPTIONS5,
		&options5,
		sizeof(options5)
	);

	if(FAILED(hr) || options5.RaytracingTier == D3D12_RAYTRACING_TIER_NOT_SUPPORTED) {
		SDL_Log("Ray Tracing not supported.");
		return false;
	}

	SDL_Log("Raytracing Tier: %d", options5.RaytracingTier);

	return true;

}

void Direct3D::BeginRender(class PipelineState* pipelineSate) {
    
    _currentRenderTarget = _renderTargets[_currentBackBufferIndex].Get();

    _allocator[_currentBackBufferIndex]->Reset();
    
    // パイプラインステートがnullptrの場合（レイトレーシング時）はnullptrを渡す
    ID3D12PipelineState* pipelineStatePtr = pipelineSate ? pipelineSate->Get() : nullptr;
    _commandList->Reset(_allocator[_currentBackBufferIndex].Get(), pipelineStatePtr);

    _commandList->RSSetViewports(1, &_viewport);
    _commandList->RSSetScissorRects(1, &_scissorRect);

    auto currentRTVHandle = _rtvHeap->GetCPUDescriptorHandleForHeapStart();
    currentRTVHandle.ptr += _currentBackBufferIndex * _rtvDescriptorSize;

    auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        _currentRenderTarget,
        D3D12_RESOURCE_STATE_PRESENT,
        D3D12_RESOURCE_STATE_RENDER_TARGET);
    _commandList->ResourceBarrier(1, &barrier);

    auto currentDSVHandle = _dsvHeap->GetCPUDescriptorHandleForHeapStart();

    _commandList->OMSetRenderTargets(1, &currentRTVHandle, FALSE, &currentDSVHandle);

    const float clearColor[] = { 1.0f, 1.0f, 0.0f, 1.0f };
    _commandList->ClearRenderTargetView(currentRTVHandle, clearColor, 0, nullptr);
    _commandList->RSSetViewports(1, &_viewport);
    _commandList->RSSetScissorRects(1, &_scissorRect);

    _commandList->ClearDepthStencilView(currentDSVHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
}

void Direct3D::WaitRender() {
	const UINT64 fenceValue = _fenceValue[_currentBackBufferIndex]; 
	_queue->Signal(_fence.Get(), fenceValue);
	_fenceValue[_currentBackBufferIndex]++;

	if(_fence->GetCompletedValue() < fenceValue) {
		if(FAILED(_fence->SetEventOnCompletion(fenceValue, _fenceEvent))) {
			return;
		}

		if(WAIT_OBJECT_0 != WaitForSingleObjectEx(_fenceEvent, INFINITE, FALSE)) {
			return;
		}
	}
}

void Direct3D::EndRender() {
	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		_currentRenderTarget,
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PRESENT);
	_commandList->ResourceBarrier(1, &barrier);

	_commandList->Close();

	ID3D12CommandList* commandLists[] = { _commandList.Get() };
	_queue->ExecuteCommandLists(1, commandLists);

	_swapChain->Present(1, 0);

	WaitRender(); // GPUの処理が終わるまで待つ

	_currentBackBufferIndex = _swapChain->GetCurrentBackBufferIndex();
}
