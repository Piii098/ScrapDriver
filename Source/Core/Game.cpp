#include "Core/Game.h"
#include <SDL3_ttf/SDL_ttf.h>
#include "Input/InputManager.h"
#include "Renderer/RendererManager.h"
#include "DirectX/Direct3D.h"
#include "Core/SceneManager.h"
#include "Resource/AssetManager.h"

Game* Game::s_instance = nullptr;

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

	_inputAction = new InputAction(*_inputManager);

	SetAction();

	AssetManagers::CreateInstance(*this);

	SceneManager::CreateInstance();
	if (!SCENEMANAGER.Initialize()) {
		SDL_Log("Failed to initialize SceneManager : %s", SDL_GetError());
	}
	

	return true;
}

void Game::Input() {
	_inputManager->PreUpdate();
	SDL_Event event;
	while(SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_EVENT_QUIT:
			SDL_Log("Quit Event");
			// ウィンドウ閉じる
			_isRunning = false;
			break;
		case SDL_EVENT_MOUSE_WHEEL:     // マウスホイールイベント
		case SDL_EVENT_GAMEPAD_ADDED:   // ゲームパッド接続イベント
		case SDL_EVENT_GAMEPAD_REMOVED: // ゲームパッド切断イベント
			_inputManager->ProcessInput(event);
			break;
		case SDL_EVENT_WINDOW_RESIZED:
			SDL_Log("Window Resized Event");
			break;
		default:
			break;
		}
	}
	_inputManager->Update(0.1f);

	SCENEMANAGER.Input(*_inputAction);
}

void Game::SetAction() {

	_inputAction->MapAction("Left", SDL_SCANCODE_A);
	_inputAction->MapAction("Right", SDL_SCANCODE_D);
	_inputAction->MapAction("Up", SDL_SCANCODE_W);
	_inputAction->MapAction("Down", SDL_SCANCODE_S);
	_inputAction->MapAction("Select", SDL_SCANCODE_SPACE);
	_inputAction->MapAction("Jump", SDL_SCANCODE_SPACE);
	_inputAction->MapAction("Menu", SDL_SCANCODE_TAB);

	_inputAction->MapAction("Left", SDL_GAMEPAD_BUTTON_DPAD_LEFT);
	_inputAction->MapAction("Right", SDL_GAMEPAD_BUTTON_DPAD_RIGHT);
	_inputAction->MapAction("Up", SDL_GAMEPAD_BUTTON_DPAD_UP);
	_inputAction->MapAction("Down", SDL_GAMEPAD_BUTTON_DPAD_DOWN);
	_inputAction->MapAction("Select", SDL_GAMEPAD_BUTTON_SOUTH);
	_inputAction->MapAction("Jump", SDL_GAMEPAD_BUTTON_SOUTH);
	_inputAction->MapAction("Menu", SDL_GAMEPAD_BUTTON_BACK);

	_inputAction->MapActionAxis("Horizontal", SDL_SCANCODE_A, SDL_SCANCODE_D);
	_inputAction->MapActionAxis("Vertical", SDL_SCANCODE_S, SDL_SCANCODE_W);

	_inputAction->MapActionAxis("Horizontal", SDL_GAMEPAD_AXIS_LEFTX);
	_inputAction->MapActionAxis("Vertical", SDL_GAMEPAD_AXIS_LEFTY);
}

void Game::Update() {
	Input();

	SCENEMANAGER.Update(0.1);
}

void Game::Render() {
	_rendererManager->Render([this]() {
		SCENEMANAGER.Render();
		});
}