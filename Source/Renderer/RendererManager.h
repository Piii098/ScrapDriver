#pragma once
#include <SDL3/SDL.h>
#include "DirectX/DirectX.h"
#include "DirectX/ShaderStruct.h"
#include <Windows.h>
#include <vector>
#include <functional>
#include "Renderer/RaytracingManager.h"

class RendererManager {
public:
	RendererManager(class Game& game);
	~RendererManager();

	bool Initialize();
	void Update();
	void Render(std::function<void()> renderFunc);

	// レイトレーシング関連
	bool InitializeRaytracing();
	void RenderRaytracing();
	RaytracingManager* GetRaytracingManager() { return _raytracingManager.get(); }

	// カメラとライティング設定
	//void SetCamera(const Vector3& eyePos, const Vector3& targetPos, const Vector3& upward);
	//void SetProjection(float fov, float aspect, float nearPlane, float farPlane);

private:
	class Game& _game;
	class Direct3D* _direct3D;
	class Shader* _shader;
	class Camera* _camera;
	
	// レンダリング関連
	class ConstantBuffer* _constantBuffer; 
	class RootSignature* _rootSignature;
	class PipelineState* _pipelineState;

	// レイトレーシング関連
	std::unique_ptr<RaytracingManager> _raytracingManager;
	bool _useRaytracing = false;

	float _angle;
	Matrix4x4 _worldMatrix;
	Matrix4x4 _viewMatrix;
	Matrix4x4 _projectionMatrix;
	Transform* _mapPtr;
	UINT _cbvIndex = 0;
	
	bool CreateRenderingResources();
	bool CreateRootSignature();
	bool CreatePipelineState();
};

