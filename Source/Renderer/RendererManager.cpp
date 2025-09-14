#include "Renderer/RendererManager.h"
#include "DirectX/Direct3D.h"
#include "DirectX/ConstantBuffer.h"
#include "DirectX/RootSignature.h"
#include "DirectX/PipelineState.h"
#include "Utility/Math.h"
#include <d3dcompiler.h>
#include "Camera.h"

RendererManager::RendererManager(class Game& game)
	: _game(game), _direct3D(nullptr), _shader(nullptr), _camera(nullptr),
	  _rootSignature(nullptr), _pipelineState(nullptr), _rotationY(0.0f) {
	
	// 定数バッファの初期化
	for (int i = 0; i < 3; i++) {
		_constantBuffer[i] = nullptr;
	}
}

RendererManager::~RendererManager(){
	// リソースの解放
	for (int i = 0; i < 3; i++) {
		if (_constantBuffer[i]) {
			delete _constantBuffer[i];
			_constantBuffer[i] = nullptr;
		}
	}
	
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
	if (!DIRECT3D.Initialize()) {
		return false;
	}

	DIRECT3D.SetProjectionMatrix(800, 600, 0.1f, 100.0f);

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

	// デフォルトのカメラ設定
	SetCamera(Vector3(0.0f, 120.0f, 75.0f), Vector3(0.0f, 120.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f));
	auto aspect = static_cast<float>(DIRECT3D.Width()) / static_cast<float>(DIRECT3D.Height());
	SetProjection(XMConvertToRadians(60), aspect, 0.3f, 1000.0f);

	return true;
}


void RendererManager::Update() {
	_rotationY += 0.01f;
	auto currentIndex = DIRECT3D.CurrentBackBufferIndex();
	auto currentTransform = _constantBuffer[currentIndex]->GetPtr<Transform>();
	currentTransform->World = XMMatrixRotationY(_rotationY);
}

void RendererManager::Render() {
	DIRECT3D.BeginRender();

	// ここで実際のレンダリング処理を行う
	// モデル関連は別途実装予定

	DIRECT3D.EndRender();
}

void RendererManager::SetCamera(const Vector3& eyePos, const Vector3& targetPos, const Vector3& upward) {
	auto eyePosVec = XMVectorSet(eyePos.x, eyePos.y, eyePos.z, 0.0f);
	auto targetPosVec = XMVectorSet(targetPos.x, targetPos.y, targetPos.z, 0.0f);
	auto upwardVec = XMVectorSet(upward.x, upward.y, upward.z, 0.0f);
	
	for (size_t i = 0; i < DIRECT3D.FRAME_BUFFER_COUNT; i++) {
		if (_constantBuffer[i] && _constantBuffer[i]->IsValid()) {
			auto ptr = _constantBuffer[i]->GetPtr<Transform>();
			ptr->View = XMMatrixLookAtRH(eyePosVec, targetPosVec, upwardVec);
		}
	}
}

void RendererManager::SetProjection(float fov, float aspect, float nearPlane, float farPlane) {
	for (size_t i = 0; i < DIRECT3D.FRAME_BUFFER_COUNT; i++) {
		if (_constantBuffer[i] && _constantBuffer[i]->IsValid()) {
			auto ptr = _constantBuffer[i]->GetPtr<Transform>();
			ptr->Proj = XMMatrixPerspectiveFovRH(fov, aspect, nearPlane, farPlane);
		}
	}
}

bool RendererManager::CreateRenderingResources() {
	// 定数バッファの作成
	for (size_t i = 0; i < DIRECT3D.FRAME_BUFFER_COUNT; i++) {
		_constantBuffer[i] = new ConstantBuffer(sizeof(Transform));
		if (!_constantBuffer[i]->IsValid()) {
			printf("変換行列用定数バッファの生成に失敗\n");
			return false;
		}

		// 変換行列の初期化
		auto ptr = _constantBuffer[i]->GetPtr<Transform>();
		ptr->World = XMMatrixIdentity();
		ptr->View = XMMatrixIdentity();
		ptr->Proj = XMMatrixIdentity();
	}
	
	return true;
}

bool RendererManager::CreateRootSignature() {
	_rootSignature = new RootSignature();
	if (!_rootSignature->IsValid()) {
		SDL_Log("Failed to create root signature");
		return false;
	}
	return true;
}

bool RendererManager::CreatePipelineState() {
	_pipelineState = new PipelineState();
	_pipelineState->SetInputLayout(Vertex::InputLayout);
	_pipelineState->SetRootSignature(_rootSignature->Get());
	_pipelineState->SetVS(L"bin/x64/Debug/SimpleVS.cso");
	_pipelineState->SetPS(L"bin/x64/Debug/SimplePS.cso");
	_pipelineState->Create();
	if (!_pipelineState->IsValid()) {
		printf("パイプラインステートの生成に失敗\n");
		return false;
	}
	return true;
}

