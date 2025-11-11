#include "Renderer/RendererManager.h"
#include "DirectX/Direct3D.h"
#include "DirectX/ConstantBuffer.h"
#include "DirectX/RootSignature.h"
#include "DirectX/PipelineState.h"
#include "DirectX/DescriptorHeap.h"
#include "Utility/Math.h"
#include <d3dcompiler.h>
#include "Component/CameraComponent.h"

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
        _useRaytracing = false;
        return false;
    }

    _useRaytracing = true;
    
    // GPU タイムアウト対策: 解像度を下げる
    // または、フレームレートを制限する
    SDL_Log("Raytracing enabled with recursion depth limit");
    
    return true;
}

void RendererManager::RenderRaytracing() {
	if (!_useRaytracing || !_raytracingManager) {
		return;
	}

	_raytracingManager->Render();
}

void RendererManager::Update() {

}

void RendererManager::Render(std::function<void()> renderFunc) {
    // レイトレーシングレンダリング
    if (_useRaytracing) {
        DIRECT3D.BeginRender(nullptr);
        
        //// 最初のフレームでAcceleration Structureを構築
        //static bool firstFrame = true;
        //if (firstFrame) {
        //    float vertices[] = {
        //        0.0f,  1.0f, 0.0f,
        //        1.0f, -1.0f, 0.0f,
        //       -1.0f, -1.0f, 0.0f
        //    };
        //    
        //    if (_raytracingManager->AddTriangle(vertices, 3)) {
        //        SDL_Log("Triangle added successfully on first frame");
        //    } else {
        //        SDL_Log("Failed to add triangle on first frame");
        //    }
        //    firstFrame = false;
        //}
        //
        RenderRaytracing();
        DIRECT3D.EndRender();
        return;
    }

    // 通常のラスタライズレンダリング
    DIRECT3D.BeginRender(_pipelineState);
    DIRECT3D.CommandList().SetGraphicsRootSignature(_rootSignature->Get());
    DESCRIPTORMANAGER.BindDescriptorHeaps();

    _angle += 0.01f;
    _worldMatrix = XMMatrixRotationY(_angle);
    _mapPtr->World = _worldMatrix;
    _mapPtr->View = _viewMatrix;
    _mapPtr->Proj = _projectionMatrix;

    D3D12_GPU_DESCRIPTOR_HANDLE cbvHandle = DESCRIPTORMANAGER.GetGPUHandleAt(_cbvIndex);
    DIRECT3D.CommandList().SetGraphicsRootDescriptorTable(1, cbvHandle);

    if (renderFunc) {
        renderFunc();
    }

    DIRECT3D.EndRender();
}

//void RendererManager::SetCamera(const Vector3& eyePos, const Vector3& targetPos, const Vector3& upward) {
//	
//}
//
//void RendererManager::SetProjection(float fov, float aspect, float nearPlane, float farPlane) {
//	
//}

bool RendererManager::CreateRenderingResources() {

	_constantBuffer = new ConstantBuffer();\

	if(!_constantBuffer->IsValid()) {
		SDL_Log("Failed to create constant buffer");
		return false;
	}

	_worldMatrix = XMMatrixRotationY(_angle);
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
		_mapPtr->World = _worldMatrix;
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

