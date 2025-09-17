#include "Core/Game.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

int main(int argc, char* argv[]) {
	Game game;

	if (!game.Initalize()) {
		return -1;
	}

	while (true) {
		game.Update();
		game.Render();
	}

	return 0;
}