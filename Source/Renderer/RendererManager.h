#pragma once
#include <SDL3/SDL.h>
#include "DirectX/DirectX.h"
#include "DirectX/ShaderStruct.h"
#include <Windows.h>
#include <vector>

class RendererManager {
public:
	RendererManager(class Game& game);
	~RendererManager();

	bool Initialize();
	void Update();
	void Render();
	
	// カメラとライティング設定
	void SetCamera(const Vector3& eyePos, const Vector3& targetPos, const Vector3& upward);
	void SetProjection(float fov, float aspect, float nearPlane, float farPlane);

private:
	class Game& _game;
	class Direct3D* _direct3D;
	class Shader* _shader;
	class Camera* _camera;
	
	// レンダリング関連
	class ConstantBuffer* _constantBuffer[3]; // FRAME_BUFFER_COUNT分
	class RootSignature* _rootSignature;
	class PipelineState* _pipelineState;
	
	// 変換行列
	float _rotationY = 0.0f;
	
	bool CreateRenderingResources();
	bool CreateRootSignature();
	bool CreatePipelineState();
};