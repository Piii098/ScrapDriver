#pragma once
#include <SDL3/SDL.h>
#include "DirectX.h"

#include <Windows.h>

class RendererManager {
public:
	RendererManager(class Game& game);
	~RendererManager();

	bool Initialize();

	void Render();

private:
	class Game& _game;
	class Direct3D* _direct3D;
	class Shader* _shader;
	class Camera* _camera;

};