#include "Renderer/RendererManager.h"
#include "DirectX/Direct3D.h"
#include "DirectX/ConstantBuffer.h"
#include "DirectX/RootSignature.h"
#include "DirectX/PipelineState.h"
#include "DirectX/DescriptorHeap.h"
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

    if (renderFunc) {
        renderFunc();  // ← ここで各ModelComponent::Render()が呼ばれる
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

