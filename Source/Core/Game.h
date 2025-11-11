#pragma once
#include <SDL3/SDL.h>

class Game {
public:
	Game();
	~Game();

	bool Initalize();

	void Input();

	void Update();

	void Render();

	bool IsRunning() const { return _isRunning; }

	class RendererManager& GetRendererManager() { return *_rendererManager; }

private:

	bool _isRunning = true;

	class RendererManager* _rendererManager;
	class InputManager* _inputManager;
	class InputAction* _inputAction;
	class SceneManager* _sceneManager;
	
	static Game* s_instance;

	void SetAction();


public:

	static void CreateInstance() {
		DestroyInstance();
		s_instance = new Game();
	}

	static void DestroyInstance() {
		if (s_instance) {
			delete s_instance;
			s_instance = nullptr;
		}
	}

	static Game& Instance() {
		return *s_instance;
	}

};

#define GAME Game::Instance()