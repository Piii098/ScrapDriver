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

private:

	class RendererManager* _rendererManager;
	class InputManager* _inputManager;
	class InputAction* _inputAction;

};