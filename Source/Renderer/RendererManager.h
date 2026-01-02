#pragma once
#include <SDL3/SDL.h>
#include "DirectX/DirectX.h"
#include "DirectX/ShaderStruct.h"
#include <Windows.h>
#include <vector>
#include <functional>
#include "Renderer/RaytracingManager.h"

enum class RenderingMode {
	RASTERIZATION,  // ラスタライズ
	RAYTRACING      // レイトレーシング
};

class RendererManager {
public:
	RendererManager(class Game& game);
	~RendererManager();

	bool Initialize();
	void Update();
	void Render(std::function<void()> renderFunc);

	// レンダリングモード切り替え
	void SetRenderingMode(RenderingMode mode);
	RenderingMode GetRenderingMode() const { return _renderingMode; }
	bool IsRaytracing() const { return _renderingMode == RenderingMode::RAYTRACING; }

	// レイトレーシング関連
	bool InitializeRaytracing();
	void RenderRaytracing();
	RaytracingManager* GetRaytracingManager() { return _raytracingManager.get(); }
	
	// ModelComponent統合レンダリング
	void RegisterModel(class ModelComponent* model);
	void UnregisterModel(class ModelComponent* model);
	void UpdateModelsForRaytracing();
	
	// ライト管理
	void UpdateLights();
	
	// ワールド行列設定（ラスタライズ用）
	void SetWorldMatrix(const Matrix4x4& world);

	UINT GetConstantBufferIndex() const { return _cbvIndex; }
	
	// シャドウマップ取得
	class ShadowMap* GetShadowMap() { return _shadowMap; }

	// カメラはCameraComponent::GetActiveCamera()から自動取得

private:
	class Game& _game;
	class Direct3D* _direct3D;
	class Shader* _shader;
	class Camera* _camera;
	
	// レンダリング関連
	class ConstantBuffer* _constantBuffer; 
	class RootSignature* _rootSignature;
	class PipelineState* _pipelineState;
	
	// シャドウマップ関連
	class ShadowMap* _shadowMap = nullptr;
	class ShadowRootSignature* _shadowRootSignature = nullptr;
	class ShadowPipelineState* _shadowPipelineState = nullptr;
	GenericConstantBuffer<Matrix4x4>* _shadowLightConstantBuffer = nullptr;
	GenericConstantBuffer<ShadowLightsData>* _lightsConstantBuffer = nullptr;
	UINT _shadowLightCBVIndex = 0;
	UINT _lightsCBVIndex = 0;
	
	// Shadow map constants
	static constexpr UINT SHADOW_MAP_SIZE = 2048;

	// レイトレーシング関連
	std::unique_ptr<RaytracingManager> _raytracingManager;
	RenderingMode _renderingMode = RenderingMode::RAYTRACING;
	
	// ModelComponent管理
	std::vector<class ModelComponent*> _registeredModels;

	float _angle;
	Matrix4x4 _worldMatrix;
	Matrix4x4 _viewMatrix;
	Matrix4x4 _projectionMatrix;
	Transform* _mapPtr;
	UINT _cbvIndex = 0;
	
	bool CreateRenderingResources();
	bool CreateRootSignature();
	bool CreatePipelineState();
	bool CreateShadowResources();
	void RenderShadowMap(std::function<void()> renderFunc);
};