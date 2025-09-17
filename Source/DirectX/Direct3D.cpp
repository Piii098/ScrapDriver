#include "DirectX/Direct3D.h"

Direct3D* Direct3D::s_instance = nullptr;

Direct3D::Direct3D()
	: _window(nullptr) {
}

Direct3D::~Direct3D() {
}

bool Direct3D::Initialize() {

	SDL_WindowFlags flags = SDL_WINDOW_RESIZABLE;
	_window = SDL_CreateWindow("test", 640, 480, flags);

	_hWnd = (HWND)SDL_GetPointerProperty(SDL_GetWindowProperties(_window), SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);

	SDL_GetWindowSize(_window, &_width, &_height);

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

	if (!CreateCommandAllocator()) {
		return false;
	}

	if (!CreateFence()) {
		return false;
	}

	return true;
}

bool Direct3D::CreateDevice() {

	if (FAILED(D3D12CreateDevice(
				nullptr,
				D3D_FEATURE_LEVEL_11_0,
				IID_PPV_ARGS(_device.ReleaseAndGetAddressOf())))) {
		return false;
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
	IDXGIFactory4* factory = nullptr;
	HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&factory));
	if (FAILED(hr)) {
		return false;
	}

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
	desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	IDXGISwapChain* swapChain = nullptr;
	if (FAILED(factory->CreateSwapChain(
		_queue.Get(),
		&desc,
		&swapChain))) {
		return false;
	}
	if (FAILED(swapChain->QueryInterface(IID_PPV_ARGS(&_swapChain)))) {
		factory->Release();
		swapChain->Release();
		return false;
	}

	_currentBackBufferIndex = _swapChain->GetCurrentBackBufferIndex();

	factory->Release();
	swapChain->Release();

	return true;
}

bool Direct3D::CreateCommandAllocator() {

	// ・ｽR・ｽ}・ｽ・ｽ・ｽh・ｽA・ｽ・ｽ・ｽP・ｽ[・ｽ^・ｽ[・ｽﾌ作成
	for (int i = 0; i < FRAME_BUFFER_COUNT; i++) {
		if (FAILED(_device->CreateCommandAllocator(
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			IID_PPV_ARGS(&_allocator[i])))) {
			return false;
		}
	}

	// ・ｽR・ｽ}・ｽ・ｽ・ｽh・ｽ・ｽ・ｽX・ｽg・ｽﾌ作成
	if(FAILED(_device->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		_allocator[0].Get(),
		nullptr,
		IID_PPV_ARGS(&_commandList)))) {
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

	// 
	_rtvDescriptorSize = _device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = _rtvHeap->GetCPUDescriptorHandleForHeapStart();

	for (int i = 0; i < FRAME_BUFFER_COUNT; i++) {
		_swapChain->GetBuffer(i, IID_PPV_ARGS(_renderTargets[i].ReleaseAndGetAddressOf()));
		_device->CreateRenderTargetView(_renderTargets[i].Get(), nullptr, rtvHandle);
		rtvHandle.ptr += _rtvDescriptorSize;
	}
	
	return true;
}

bool Direct3D::CreateDepthStencil() {
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.NumDescriptors = 1;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	if (FAILED(_device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&_dsvHeap)))) {
		return false;
	}

	_dsvDescriptorSize = _device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	D3D12_CLEAR_VALUE clearValue = {};
	clearValue.Format = DXGI_FORMAT_D32_FLOAT;
	clearValue.DepthStencil.Depth = 1.0f;
	clearValue.DepthStencil.Stencil = 0;

	auto heapDesc = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	CD3DX12_RESOURCE_DESC resourceDesc(
		D3D12_RESOURCE_DIMENSION_TEXTURE2D,
		0,
		_width,
		_height,
		1,
		1,
		DXGI_FORMAT_D32_FLOAT,
		1,
		0,
		D3D12_TEXTURE_LAYOUT_UNKNOWN,
		D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL | D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE);

	if (FAILED(_device->CreateCommittedResource(
		&heapDesc,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&clearValue,
		IID_PPV_ARGS(_depthStencilBuffer.ReleaseAndGetAddressOf())))) {
		return false;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = _dsvHeap->GetCPUDescriptorHandleForHeapStart();

	_device->CreateDepthStencilView(_depthStencilBuffer.Get(), nullptr, dsvHandle);

	return true;
}

void Direct3D::BeginRender() {
	
	_currentRenderTarget = _renderTargets[_currentBackBufferIndex].Get();

	_allocator[_currentBackBufferIndex]->Reset();
	_commandList->Reset(_allocator[_currentBackBufferIndex].Get(), nullptr);

	_commandList->RSSetViewports(1, &_viewport);
	_commandList->RSSetScissorRects(1, &_scissorRect);

	auto currentRTVHandle = _rtvHeap->GetCPUDescriptorHandleForHeapStart();
	currentRTVHandle.ptr += _currentBackBufferIndex * _rtvDescriptorSize;

	auto currentDSVHandle = _dsvHeap->GetCPUDescriptorHandleForHeapStart();

	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		_currentRenderTarget,
		D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RENDER_TARGET);
	_commandList->ResourceBarrier(1, &barrier);

	_commandList->OMSetRenderTargets(1, &currentRTVHandle, FALSE, &currentDSVHandle);

	const float clearColor[] = { 1.0f, 1.0f, 0.0f, 1.0f };
	_commandList->ClearRenderTargetView(currentRTVHandle, clearColor, 0, nullptr);

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

	WaitRender();

	_currentBackBufferIndex = _swapChain->GetCurrentBackBufferIndex();
}
