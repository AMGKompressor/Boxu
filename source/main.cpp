// COMP710 GP Framework (Shadow Box)
#include <SDL.h>

#include "game.h"
#include "logmanager.h"

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    Game& gameInstance = Game::GetInstance();
    if (!gameInstance.Initialise())
    {
        LogManager::GetInstance().Log("Game initialise failed!");
        LogManager::DestroyInstance();
        return 1;
    }

    while (gameInstance.DoGameLoop())
    {
        // Loop body intentionally empty; work happens in DoGameLoop.
    }

    Game::DestroyInstance();
    LogManager::DestroyInstance();
    return 0;
}
