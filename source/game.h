// COMP710 GP Framework (Shadow Box)
#ifndef GAME_H
#define GAME_H

#include <cstdint>

class Renderer;

class Game
{
public:
    static Game& GetInstance();
    static void DestroyInstance();
    bool Initialise();
    bool DoGameLoop();
    void Quit();

protected:
    void Process(float deltaTime);
    void Draw(Renderer& renderer);
    void ProcessFrameCounting(float deltaTime);

private:
    Game();
    ~Game();
    Game(const Game& game);
    Game& operator=(const Game& game);

public:
protected:
    static Game* sm_pInstance;
    Renderer* m_pRenderer;
    std::uint64_t m_iLastTime;
    float m_fExecutionTime;
    float m_fElapsedSeconds;
    int m_iFrameCount;
    int m_iFPS;
    bool m_bLooping;

private:
};

#endif // GAME_H
