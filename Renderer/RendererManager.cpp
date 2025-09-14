#include "Renderer/RendererManager.h"
#include "Direct3D.h"
#include <d3dcompiler.h>
#include "Camera.h"

RendererManager::RendererManager(class Game& game)
	: _game(game){
}

RendererManager::~RendererManager(){

}

bool RendererManager::Initialize() {
	Direct3D::CreateInstance();
	if (!DIRECT3D.Initialize()) {
		return false;
	}

	_camera = new Camera();
	_camera->Position(Vector3(0.0f, 0.0f, -5.0f));
	DIRECT3D.SetProjectionMatrix(800, 600, 0.1f, 100.0f);

	return true;
}


void RendererManager::Render() {

	DIRECT3D.BeginRender();

	DIRECT3D.EndRender();
	
}

