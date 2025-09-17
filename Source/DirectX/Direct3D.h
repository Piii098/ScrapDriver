#pragma once
#include <SDL3/SDL.h>
#include <Windows.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "DirectXTex.lib")
#pragma comment(lib, "ole32.lib")      
#pragma comment(lib, "oleaut32.lib")  


#include <d3d12.h>
#include <d3dx12.h>
#include <d3dcompiler.h>
#include <dxgi1_4.h>
#include <dxgi.h>

#include <DirectXTex.h>
#include "ShaderStruct.h"

#include <wrl/client.h>
using Microsoft::WRL::ComPtr;

#include "Framework.h"

class Direct3D {
public:
	static const int FRAME_BUFFER_COUNT = 2;

	bool Initialize();

	void BeginRender();
	void WaitRender();
	void EndRender();

	void SetProjectionMatrix(const Matrix4x4& matrix) { _projectionMatrix = matrix; }
	void SetProjectionMatrix(float width, float height, float nearZ, float farZ) {
		_projectionMatrix = XMMatrixPerspectiveLH(width, height, nearZ, farZ);
	}

	ID3D12Device6& Device()  { return *_device.Get(); }
	ID3D12GraphicsCommandList& CommandList() { return *_commandList.Get(); }
	UINT CurrentBackBufferIndex() const { return _currentBackBufferIndex; }

	int Width() const { return _width; }
	int Height() const { return _height; }
	
	const Matrix4x4& ProjectionMatrix() const { return _projectionMatrix; }

private:
	Direct3D();
	~Direct3D();

	bool CreateDevice();
	bool CreateCommandQueue();
	bool CreateSwapChain();
	bool CreateCommandAllocator();
	bool CreateFence();
	bool CreateViewPort();
	bool CreateScissorRect();
	bool CreateRenderTarget(); // レンダーターゲットを生成
	bool CreateDepthStencil(); // 深度ステンシルバッファを生成


	SDL_Window* _window;

	ComPtr<ID3D12Device6> _device = nullptr; // デバイス
	ComPtr<ID3D12CommandQueue> _queue = nullptr; // コマンドキュー
	ComPtr<IDXGISwapChain3> _swapChain = nullptr; // スワップチェイン
	ComPtr<ID3D12CommandAllocator> _allocator[FRAME_BUFFER_COUNT] = { nullptr }; // コマンドアロケーたー
	ComPtr<ID3D12GraphicsCommandList> _commandList = nullptr; // コマンドリス
	HANDLE _fenceEvent = nullptr; // フェンスイベントハンドル
	ComPtr<ID3D12Fence> _fence = nullptr; // フェンス
	UINT64 _fenceValue[FRAME_BUFFER_COUNT] = { 0 }; // フェンス値
	D3D12_VIEWPORT _viewport; // ビューポート
	D3D12_RECT _scissorRect; // シザー矩形

	UINT _rtvDescriptorSize = 0; // レンダーターゲットビューのディスクリプタサイズ
	ComPtr<ID3D12DescriptorHeap> _rtvHeap = nullptr; // レンダーターゲットのディスクリプタヒープ
	ComPtr<ID3D12Resource> _renderTargets[FRAME_BUFFER_COUNT] = { nullptr }; // レンダーターゲット（ダブルバッファリングするので2個）

	UINT _dsvDescriptorSize = 0; // 深度ステンシルのディスクリプターサイズ
	ComPtr<ID3D12DescriptorHeap> _dsvHeap = nullptr; // 深度ステンシルのディスクリプタヒープ
	ComPtr<ID3D12Resource> _depthStencilBuffer = nullptr; // 深度ステンシルバッファ（こっちは1つでいい）
	ID3D12Resource* _currentRenderTarget = nullptr; // 現在のレンダーターゲット

	Matrix4x4 _projectionMatrix;

	HWND _hWnd;
	int _width;
	int _height;

	int _currentBackBufferIndex = 0;
	
	static Direct3D* s_instance;

public:

	static void CreateInstance() {
		DestroyInstance();
		s_instance = new Direct3D();
	}

	static void DestroyInstance() {
		if (s_instance) {
			delete s_instance;
			s_instance = nullptr;
		}
	}

	static Direct3D& Instance() {
		return *s_instance;
	}
};

#define DIRECT3D Direct3D::Instance()