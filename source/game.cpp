// COMP710 GP Framework (Shadow Box)
#include "game.h"

#include "logmanager.h"
#include "renderer.h"

#include <SDL.h>

Game* Game::sm_pInstance = nullptr;

Game& Game::GetInstance()
{
    if (sm_pInstance == nullptr)
    {
        sm_pInstance = new Game();
    }
    return *sm_pInstance;
}

void Game::DestroyInstance()
{
    delete sm_pInstance;
    sm_pInstance = nullptr;
}

Game::Game()
    : m_pRenderer(nullptr)
    , m_iLastTime(0)
    , m_fExecutionTime(0.0f)
    , m_fElapsedSeconds(0.0f)
    , m_iFrameCount(0)
    , m_iFPS(0)
    , m_bLooping(true)
{
}

Game::~Game()
{
    delete m_pRenderer;
    m_pRenderer = nullptr;
}

void Game::Quit()
{
    m_bLooping = false;
}

bool Game::Initialise()
{
    const int bbWidth = 1024;
    const int bbHeight = 768;

    m_pRenderer = new Renderer();
    if (!m_pRenderer->Initialise(true, bbWidth, bbHeight))
    {
        LogManager::GetInstance().Log("Renderer failed to initialise!");
        delete m_pRenderer;
        m_pRenderer = nullptr;
        return false;
    }

    m_iLastTime = SDL_GetPerformanceCounter();
    m_pRenderer->SetClearColour(0, 255, 255);
    LogManager::GetInstance().Log("Shadow Box: Game initialised.");
    return true;
}

bool Game::DoGameLoop()
{
    SDL_Event event;
    while (SDL_PollEvent(&event) != 0)
    {
        if (event.type == SDL_QUIT)
        {
            m_bLooping = false;
        }
        else if (event.type == SDL_KEYDOWN && event.key.repeat == 0)
        {
            if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
            {
                Quit();
            }
        }
    }

    if (m_bLooping)
    {
        const std::uint64_t current = SDL_GetPerformanceCounter();
        const float deltaTime =
            (current - m_iLastTime) / static_cast<float>(SDL_GetPerformanceFrequency());
        m_iLastTime = current;
        m_fExecutionTime += deltaTime;

        Process(deltaTime);
        Draw(*m_pRenderer);
    }

    return m_bLooping;
}

void Game::Process(float deltaTime)
{
    ProcessFrameCounting(deltaTime);
}

void Game::Draw(Renderer& renderer)
{
    ++m_iFrameCount;
    renderer.Clear();
    renderer.Present();
}

void Game::ProcessFrameCounting(float deltaTime)
{
    m_fElapsedSeconds += deltaTime;
    if (m_fElapsedSeconds > 1.0f)
    {
        m_fElapsedSeconds -= 1.0f;
        m_iFPS = m_iFrameCount;
        m_iFrameCount = 0;
    }
}
