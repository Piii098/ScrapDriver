#include "Renderer/RendererManager.h"
#include "DirectX/Direct3D.h"
#include "DirectX/ConstantBuffer.h"
#include "DirectX/GenericConstantBuffer.h"
#include "DirectX/RootSignature.h"
#include "DirectX/PipelineState.h"
#include "DirectX/DescriptorHeap.h"
#include "DirectX/ShadowMap.h"
#include "DirectX/ShadowRootSignature.h"
#include "DirectX/ShadowPipelineState.h"
#include "Utility/Math.h"
#include <d3dcompiler.h>
#include "Component/CameraComponent.h"
#include "Component/ModelComponent.h"

RendererManager::RendererManager(class Game& game)
	: _game(game), _direct3D(nullptr), _shader(nullptr), _camera(nullptr),
	  _rootSignature(nullptr), _pipelineState(nullptr), _angle(0.7f){
	
}

RendererManager::~RendererManager(){

	if (_rootSignature) {
		delete _rootSignature;
		_rootSignature = nullptr;
	}
	
	if (_pipelineState) {
		delete _pipelineState;
		_pipelineState = nullptr;
	}
	
	if (_shadowMap) {
		delete _shadowMap;
		_shadowMap = nullptr;
	}
	
	if (_shadowRootSignature) {
		delete _shadowRootSignature;
		_shadowRootSignature = nullptr;
	}
	
	if (_shadowPipelineState) {
		delete _shadowPipelineState;
		_shadowPipelineState = nullptr;
	}
}

bool RendererManager::Initialize() {
	Direct3D::CreateInstance();
	DIRECT3D.Width(1000);
	DIRECT3D.Height(800);
	if (!DIRECT3D.Initialize()) {
		return false;
	}

	DIRECT3D.SetProjectionMatrix(1000, 800, 0.1f, 100.0f);

	DescriptorManager::CreateInstance();

	if (!DESCRIPTORMANAGER.Initialize()) {
		SDL_Log("Failed to initialize DescriptorManager");
		return false; 
	}	

	// レンダリングリソースの作成
	if (!CreateRenderingResources()) {
		return false;
	}
	
	if (!CreateRootSignature()) {
		return false;
	}
	
	if (!CreatePipelineState()) {
		return false;
	}
	
	// シャドウマップリソースの作成
	if (!CreateShadowResources()) {
		SDL_Log("Warning: Shadow resources creation failed, shadows will be disabled");
		// シャドウが無くても動作は可能なので、失敗してもエラーにしない
	}

	// レイトレーシング初期化（オプション）
	InitializeRaytracing();

	// デフォルトのカメラ設定
	//SetCamera(Vector3(0.0f, 120.0f, 75.0f), Vector3(0.0f, 120.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f));
	//auto aspect = static_cast<float>(DIRECT3D.Width()) / static_cast<float>(DIRECT3D.Height());
	//SetProjection(XMConvertToRadians(60), aspect, 0.3f, 1000.0f);

	return true;
}

bool RendererManager::InitializeRaytracing() {
    _raytracingManager = std::make_unique<RaytracingManager>();

    if (!_raytracingManager->Initialize()) {
        SDL_Log("Raytracing initialization failed, using rasterization only");
        _raytracingManager.reset();
        _renderingMode = RenderingMode::RASTERIZATION;
        return false;
    }
    
    // デフォルトはラスタライズモードのまま
    // 切り替えはSetRenderingMode()で行う
    SDL_Log("Raytracing initialized (use SetRenderingMode to enable)");
    
    return true;
}

void RendererManager::SetRenderingMode(RenderingMode mode) {
    if (mode == RenderingMode::RAYTRACING && !_raytracingManager) {
        SDL_Log("Cannot switch to raytracing mode: not initialized");
        return;
    }
    
    _renderingMode = mode;
    SDL_Log("Rendering mode switched to: %s", 
        mode == RenderingMode::RAYTRACING ? "RAYTRACING" : "RASTERIZATION");
    
    // モード切り替え時は全モデルを再登録
    if (mode == RenderingMode::RAYTRACING) {
        UpdateModelsForRaytracing();
    }
}

void RendererManager::RegisterModel(ModelComponent* model) {
    if (!model) return;
    
    auto it = std::find(_registeredModels.begin(), _registeredModels.end(), model);
    if (it == _registeredModels.end()) {
        _registeredModels.push_back(model);
        
        // レイトレーシングモードの場合は即座に登録
        if (_renderingMode == RenderingMode::RAYTRACING) {
            // モデルの更新をトリガー
            model->OnUpdateWorldTransform();
        }
    }
}

void RendererManager::UnregisterModel(ModelComponent* model) {
    auto it = std::find(_registeredModels.begin(), _registeredModels.end(), model);
    if (it != _registeredModels.end()) {
        _registeredModels.erase(it);
    }
}

void RendererManager::UpdateModelsForRaytracing() {
    if (_renderingMode != RenderingMode::RAYTRACING || !_raytracingManager) {
        return;
    }
    
    // 全ModelComponentのレイトレーシングメッシュを更新
    for (auto* model : _registeredModels) {
        model->OnUpdateWorldTransform();
    }
}

void RendererManager::SetWorldMatrix(const Matrix4x4& world) {
	// 互換性維持用（現在はRootConstantsで直接送るため何もしない）
	_worldMatrix = world;
}

void RendererManager::RenderRaytracing() {
	if (_renderingMode != RenderingMode::RAYTRACING || !_raytracingManager) {
		return;
	}

	_raytracingManager->Render();
}

void RendererManager::Update() {

}

void RendererManager::Render(std::function<void()> renderFunc) {
    // レイトレーシングモード
    if (_renderingMode == RenderingMode::RAYTRACING && _raytracingManager) {
        DIRECT3D.BeginRender(nullptr);
        
        for (auto* model : _registeredModels) {
            model->UpdateRaytracingMesh();
        }
        
        RenderRaytracing();
        DIRECT3D.EndRender();
        return;
    }

    // ラスタライズモード
    
    // シャドウマップ生成
    if (_shadowMap) {
        RenderShadowMap(renderFunc);
    }
    
    // メインレンダリングパス
    DIRECT3D.BeginRender(_pipelineState);
    DIRECT3D.CommandList().SetGraphicsRootSignature(_rootSignature->Get());
    DESCRIPTORMANAGER.BindDescriptorHeaps();

    CameraComponent* camera = CameraComponent::GetActiveCamera();
    if (camera) {
        float aspect = static_cast<float>(DIRECT3D.Width()) / static_cast<float>(DIRECT3D.Height());
        _viewMatrix = camera->GetViewMatrix();
        _projectionMatrix = camera->GetProjectionMatrix(aspect);
    }

	// ビュー・プロジェクション行列を更新（1回のみ）
	_mapPtr->View = _viewMatrix;
	_mapPtr->Proj = _projectionMatrix;

	// シーン定数バインド（b0）
	D3D12_GPU_DESCRIPTOR_HANDLE cbvHandle = DESCRIPTORMANAGER.GetGPUHandleAt(_cbvIndex);
	DIRECT3D.CommandList().SetGraphicsRootDescriptorTable(1, cbvHandle);
	
	// シャドウマップをバインド（t1）
	if (_shadowMap) {
		D3D12_GPU_DESCRIPTOR_HANDLE shadowMapHandle = DESCRIPTORMANAGER.GetGPUHandleAt(_shadowMap->GetSRVIndex());
		DIRECT3D.CommandList().SetGraphicsRootDescriptorTable(4, shadowMapHandle);
	}
	
	// ライト定数バッファをバインド（b3）
	if (_lightsConstantBuffer) {
		D3D12_GPU_DESCRIPTOR_HANDLE lightsHandle = DESCRIPTORMANAGER.GetGPUHandleAt(_lightsCBVIndex);
		DIRECT3D.CommandList().SetGraphicsRootDescriptorTable(5, lightsHandle);
	}

    if (renderFunc) {
        renderFunc();  // ← ここで各ModelComponent::Render()が呼ばれる
    }
    
    // シャドウマップを次のフレームのためにDepth Writeステートに戻す
    if (_shadowMap) {
        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource = _shadowMap->GetResource();
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_DEPTH_WRITE;
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        DIRECT3D.CommandList().ResourceBarrier(1, &barrier);
    }

    DIRECT3D.EndRender();
}

bool RendererManager::CreateRenderingResources() {

	_constantBuffer = new ConstantBuffer();\

	if(!_constantBuffer->IsValid()) {
		SDL_Log("Failed to create constant buffer");
		return false;
	}

	_worldMatrix = XMMatrixIdentity();
	Vector3 eye(0,0, -5);
	Vector3 target(0, 0, 0);
	Vector3 up(0, 1, 0);
	_viewMatrix = XMMatrixLookAtLH(eye, target, up);
	_projectionMatrix = XMMatrixPerspectiveFovLH(XM_PIDIV2,
		static_cast<float>(DIRECT3D.Width()) / static_cast<float>(DIRECT3D.Height()),
		1.0f,
		200.0f
	);

	_mapPtr = _constantBuffer->GetPtr();

	if (_mapPtr) {
		_mapPtr->View = _viewMatrix;
		_mapPtr->Proj = _projectionMatrix;
	}

	// 定数バッファを先に登録（インデックス0になる）
	_cbvIndex = DESCRIPTORMANAGER.ResisterConstantBuffer(_constantBuffer->GetResource());
	if (_cbvIndex == static_cast<UINT>(-1)) {
		SDL_Log("Failed to register constant buffer");
		return false;
	}
	SDL_Log("Constant buffer registered at index: %d", _cbvIndex);

	return true;
}

bool RendererManager::CreateRootSignature() {
	_rootSignature = new RootSignature();

	if (!_rootSignature->Initialize()) {
		SDL_Log("Failed to create root signature");
		return false;
	}
	DIRECT3D.CommandList().SetGraphicsRootSignature(_rootSignature->Get());
	return true;
}

bool RendererManager::CreatePipelineState() {
	_pipelineState = new PipelineState();
	_pipelineState->SetInputLayout();
	_pipelineState->SetRootSignature(_rootSignature->Get());
	_pipelineState->SetVS(L"Assets/Shader/TestVS.hlsl");
	_pipelineState->SetPS(L"Assets/Shader/TestPS.hlsl");
	_pipelineState->Create();
	if (!_pipelineState->IsValid()) {
		printf("パイプラインステートの生成に失敗\n");
		return false;
	}
	DIRECT3D.CommandList().SetPipelineState(_pipelineState->Get());
	return true;
}

bool RendererManager::CreateShadowResources() {
	// シャドウマップの作成
	_shadowMap = new ShadowMap();
	if (!_shadowMap->Initialize(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE)) {
		SDL_Log("Failed to create shadow map");
		delete _shadowMap;
		_shadowMap = nullptr;
		return false;
	}
	
	// シャドウパス用ルートシグネチャの作成
	_shadowRootSignature = new ShadowRootSignature();
	if (!_shadowRootSignature->Initialize()) {
		SDL_Log("Failed to create shadow root signature");
		return false;
	}
	
	// シャドウパス用パイプラインステートの作成
	_shadowPipelineState = new ShadowPipelineState();
	if (!_shadowPipelineState->Initialize(_shadowRootSignature->Get())) {
		SDL_Log("Failed to create shadow pipeline state");
		return false;
	}
	
	// シャドウパス用ライト定数バッファの作成（lightViewProj用）
	_shadowLightConstantBuffer = new GenericConstantBuffer<Matrix4x4>();
	if (!_shadowLightConstantBuffer->IsValid()) {
		SDL_Log("Failed to create shadow light constant buffer");
		return false;
	}
	_shadowLightCBVIndex = DESCRIPTORMANAGER.ResisterConstantBuffer(_shadowLightConstantBuffer->GetResource());
	
	// メインパス用ライト定数バッファの作成（ShadowLightsData用）
	_lightsConstantBuffer = new GenericConstantBuffer<ShadowLightsData>();
	if (!_lightsConstantBuffer->IsValid()) {
		SDL_Log("Failed to create lights constant buffer");
		return false;
	}
	_lightsCBVIndex = DESCRIPTORMANAGER.ResisterConstantBuffer(_lightsConstantBuffer->GetResource());
	
	SDL_Log("Shadow resources created successfully");
	return true;
}

void RendererManager::RenderShadowMap(std::function<void()> renderFunc) {
	if (!_shadowMap || !_shadowRootSignature || !_shadowPipelineState) {
		return;
	}
	
	// ライト空間のビュー・プロジェクション行列を計算
	// ここでは簡単のため、固定のディレクショナルライトを想定
	Vector3 lightPos(50.0f, 100.0f, 50.0f);
	Vector3 lightTarget(0.0f, 0.0f, 0.0f);
	Vector3 lightUp(0.0f, 1.0f, 0.0f);
	
	Matrix4x4 lightView = XMMatrixLookAtLH(lightPos, lightTarget, lightUp);
	
	// 正射影（シャドウマップ用）
	float shadowMapSize = 200.0f;
	Matrix4x4 lightProj = XMMatrixOrthographicLH(shadowMapSize, shadowMapSize, 0.1f, 500.0f);
	Matrix4x4 lightViewProj = lightView * lightProj;
	
	// シャドウパス用定数バッファを更新
	Matrix4x4* shadowLightMapPtr = (Matrix4x4*)_shadowLightConstantBuffer->GetPtr();
	if (shadowLightMapPtr) {
		*shadowLightMapPtr = lightViewProj;
	}
	
	// メインパス用のライト定数バッファを更新
	ShadowLightsData* lightsMapPtr = (ShadowLightsData*)_lightsConstantBuffer->GetPtr();
	if (lightsMapPtr) {
		// 簡単のため、1つのディレクショナルライトを設定
		lightsMapPtr->lightCount = 1;
		lightsMapPtr->lights[0].type = 0; // Directional
		lightsMapPtr->lights[0].direction = Vector3(lightPos - lightTarget).Normalize();
		lightsMapPtr->lights[0].color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		lightsMapPtr->lights[0].intensity = 1.0f;
		lightsMapPtr->lightViewProj = lightViewProj;
	}
	
	// シャドウマップをクリア
	_shadowMap->Clear();
	
	// ビューポートとシザー矩形を設定
	D3D12_VIEWPORT viewport = {};
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = static_cast<FLOAT>(_shadowMap->GetWidth());
	viewport.Height = static_cast<FLOAT>(_shadowMap->GetHeight());
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	DIRECT3D.CommandList().RSSetViewports(1, &viewport);
	
	D3D12_RECT scissorRect = {};
	scissorRect.left = 0;
	scissorRect.top = 0;
	scissorRect.right = _shadowMap->GetWidth();
	scissorRect.bottom = _shadowMap->GetHeight();
	DIRECT3D.CommandList().RSSetScissorRects(1, &scissorRect);
	
	// レンダーターゲットなし、深度バッファのみ
	DIRECT3D.CommandList().OMSetRenderTargets(0, nullptr, FALSE, &_shadowMap->GetDSV());
	
	// シャドウパス用パイプラインをセット
	DIRECT3D.CommandList().SetPipelineState(_shadowPipelineState->Get());
	DIRECT3D.CommandList().SetGraphicsRootSignature(_shadowRootSignature->Get());
	
	// ディスクリプタヒープをバインド
	DESCRIPTORMANAGER.BindDescriptorHeaps();
	
	// ライト定数バッファをバインド（b0）
	D3D12_GPU_DESCRIPTOR_HANDLE shadowLightHandle = DESCRIPTORMANAGER.GetGPUHandleAt(_shadowLightCBVIndex);
	DIRECT3D.CommandList().SetGraphicsRootDescriptorTable(0, shadowLightHandle);
	
	// 各モデルをシャドウマップに描画
	// Note: 現在の実装では、ModelComponent側でシャドウパス用の描画を実装する必要があります。
	// 簡易実装として、ここでは空のシャドウマップを生成します。
	// 実際のプロジェクトでは、各ModelComponentがシャドウパス用の描画コールを持つ必要があります。
	
	// リソースバリア：シャドウマップを深度書き込みからシェーダーリソースに変更
	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = _shadowMap->GetResource();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_DEPTH_WRITE;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	DIRECT3D.CommandList().ResourceBarrier(1, &barrier);
}

