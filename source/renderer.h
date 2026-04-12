// COMP710 GP Framework (Shadow Box) – minimal renderer (Phase 0)
#ifndef RENDERER_H
#define RENDERER_H

#include <SDL_video.h>

class Renderer
{
public:
    Renderer();
    ~Renderer();

    Renderer(const Renderer& renderer) = delete;
    Renderer& operator=(const Renderer& renderer) = delete;

    bool Initialise(bool windowed, int width = 0, int height = 0);
    void Clear();
    void Present();
    void SetClearColour(unsigned char r, unsigned char g, unsigned char b);
    void GetClearColour(unsigned char& r, unsigned char& g, unsigned char& b) const;

    int GetWidth() const;
    int GetHeight() const;

protected:
    bool InitialiseOpenGL(int screenWidth, int screenHeight);
    void SetFullscreen(bool fullscreen);
    void LogSdlError();

private:
    bool m_sdlVideoInitialised;
    bool m_imgInitialised;
    SDL_Window* m_pWindow;
    SDL_GLContext m_glContext;
    int m_iWidth;
    int m_iHeight;
    float m_fClearRed;
    float m_fClearGreen;
    float m_fClearBlue;
};

#endif // RENDERER_H
