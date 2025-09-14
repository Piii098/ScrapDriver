#include "Core/Game.h"
#include <SDL3_ttf/SDL_ttf.h>
#include "Input/InputManager.h"
#include "Renderer/RendererManager.h"
#include "DirectX/Direct3D.h"
#include "Core/Scene.h"

Game::Game()
	:_rendererManager(nullptr)
	, _inputManager(nullptr){
}

Game::~Game() {
	delete _rendererManager;
	_rendererManager = nullptr;
	delete _inputManager;
	_inputManager = nullptr;
}

bool Game::Initalize() {

	// SDL
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD) == 0) {
		SDL_Log("Failed to intialized SDL : %s", SDL_GetError());
		return false;
	}
	// TTF
	if (!TTF_Init()) {
		SDL_Log("Failed to intialized TTF : %s", SDL_GetError());
		return false;
	}

	_rendererManager = new RendererManager(*this);
	if( !_rendererManager->Initialize()) {
		SDL_Log("Failed to initialize RendererManager : %s", SDL_GetError());
		return false;
	}

	_inputManager = new InputManager(*this);
	if (!_inputManager->Initialize()) {
		SDL_Log("Failed to initialize InputManager : %s", SDL_GetError());
		return false;
	}

	_scene = new Scene();
	if (!_scene->Initialize()) {
		SDL_Log("Failed to initialize Scene : %s", SDL_GetError());
		return false;
	}

	return true;
}

void Game::Input() {
	SDL_Event event;
	while(SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_EVENT_QUIT:
			SDL_Log("Quit Event");
			break;
		case SDL_EVENT_WINDOW_RESIZED:
			SDL_Log("Window Resized Event");
			break;
		default:
			break;
		}
	}
}

void Game::Update() {
	Input();
	_scene->Update();
}

void Game::Render() {
	DIRECT3D.BeginRender();
	_scene->Render();
	DIRECT3D.EndRender();
}