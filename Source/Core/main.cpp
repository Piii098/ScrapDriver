#include "Core/Game.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

int main(int argc, char* argv[]) {
	GAME.CreateInstance();

	Game& game = GAME;

	if (!game.Initalize()) {
		return -1;
	}

	while (game.IsRunning()) {
		game.Update();
		game.Render();
	}

	return 0;
}