// COMP710 GP Framework 2025

#include <SDL.h>

#include "game.h"
#include "logmanager.h"

int main(int argc, char* argv[])
{
	(void)argc;
	(void)argv;

	Game& gameInstance = Game::getInstance();
	if (!gameInstance.initialize())
	{
		LogManager::getInstance().log("Game initialize failed!");

		return 1;
	}

	while (gameInstance.doGameLoop())
	{
	}

	Game::destroyInstance();
	LogManager::destroyInstance();

	return 0;
}
