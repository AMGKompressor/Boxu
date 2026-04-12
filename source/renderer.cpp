// COMP710 GP Framework (Shadow Box)
#include "renderer.h"

#include "logmanager.h"

#include <SDL.h>
#include <SDL_image.h>
#include <glew.h>

#include <vector>

Renderer::Renderer()
    : m_sdlVideoInitialised(false)
    , m_imgInitialised(false)
    , m_pWindow(nullptr)
    , m_glContext(nullptr)
    , m_iWidth(0)
    , m_iHeight(0)
    , m_fClearRed(0.0f)
    , m_fClearGreen(0.0f)
    , m_fClearBlue(0.0f)
{
}

Renderer::~Renderer()
{
    if (m_glContext != nullptr)
    {
        SDL_GL_DeleteContext(m_glContext);
        m_glContext = nullptr;
    }
    if (m_pWindow != nullptr)
    {
        SDL_DestroyWindow(m_pWindow);
        m_pWindow = nullptr;
    }
    if (m_imgInitialised)
    {
        IMG_Quit();
        m_imgInitialised = false;
    }
    if (m_sdlVideoInitialised)
    {
        SDL_Quit();
        m_sdlVideoInitialised = false;
    }
}

void Renderer::LogSdlError()
{
    LogManager::GetInstance().Log(SDL_GetError());
}

bool Renderer::Initialise(bool windowed, int width, int height)
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        LogSdlError();
        return false;
    }
    m_sdlVideoInitialised = true;

    const int imgFlags = IMG_INIT_PNG;
    const int imgInitResult = IMG_Init(imgFlags);
    if (imgInitResult == 0 || (imgInitResult & imgFlags) != imgFlags)
    {
        LogManager::GetInstance().Log("SDL_image failed to initialise (PNG).");
        LogSdlError();
        SDL_Quit();
        m_sdlVideoInitialised = false;
        return false;
    }
    m_imgInitialised = true;

    if (!windowed)
    {
        const int numDisplays = SDL_GetNumVideoDisplays();
        if (numDisplays < 1)
        {
            LogManager::GetInstance().Log("No video displays reported by SDL.");
            return false;
        }

        std::vector<SDL_DisplayMode> currentDisplayMode(static_cast<std::size_t>(numDisplays));
        for (int k = 0; k < numDisplays; ++k)
        {
            SDL_GetCurrentDisplayMode(k, &currentDisplayMode[static_cast<std::size_t>(k)]);
        }

        int widest = 0;
        int andItsHeight = 0;
        for (int k = 0; k < numDisplays; ++k)
        {
            if (currentDisplayMode[static_cast<std::size_t>(k)].w > widest)
            {
                widest = currentDisplayMode[static_cast<std::size_t>(k)].w;
                andItsHeight = currentDisplayMode[static_cast<std::size_t>(k)].h;
            }
        }
        width = widest;
        height = andItsHeight;
    }

    const bool initialised = InitialiseOpenGL(width, height);
    SetFullscreen(!windowed);
    return initialised;
}

bool Renderer::InitialiseOpenGL(int screenWidth, int screenHeight)
{
    m_iWidth = screenWidth;
    m_iHeight = screenHeight;

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

    m_pWindow = SDL_CreateWindow(
        "Shadow Box – COMP710 GP Framework",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        screenWidth,
        screenHeight,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

    if (m_pWindow == nullptr)
    {
        LogSdlError();
        return false;
    }

    m_glContext = SDL_GL_CreateContext(m_pWindow);
    if (m_glContext == nullptr)
    {
        LogSdlError();
        return false;
    }

    glewExperimental = GL_TRUE;
    const GLenum glewResult = glewInit();
    if (glewResult != GLEW_OK)
    {
        LogManager::GetInstance().Log("glewInit failed.");
        return false;
    }

    SDL_GL_SetSwapInterval(0);
    return true;
}

void Renderer::Clear()
{
    glClearColor(m_fClearRed, m_fClearGreen, m_fClearBlue, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void Renderer::Present()
{
    SDL_GL_SwapWindow(m_pWindow);
}

void Renderer::SetFullscreen(bool fullscreen)
{
    if (m_pWindow == nullptr)
    {
        return;
    }

    if (fullscreen)
    {
        SDL_SetWindowFullscreen(
            m_pWindow,
            SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_ALWAYS_ON_TOP);
        SDL_SetHint(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "0");
        SDL_SetWindowSize(m_pWindow, m_iWidth, m_iHeight);
    }
    else
    {
        SDL_SetWindowFullscreen(m_pWindow, 0);
    }
}

void Renderer::SetClearColour(unsigned char r, unsigned char g, unsigned char b)
{
    m_fClearRed = static_cast<float>(r) / 255.0f;
    m_fClearGreen = static_cast<float>(g) / 255.0f;
    m_fClearBlue = static_cast<float>(b) / 255.0f;
}

void Renderer::GetClearColour(unsigned char& r, unsigned char& g, unsigned char& b) const
{
    r = static_cast<unsigned char>(m_fClearRed * 255.0f);
    g = static_cast<unsigned char>(m_fClearGreen * 255.0f);
    b = static_cast<unsigned char>(m_fClearBlue * 255.0f);
}

int Renderer::GetWidth() const
{
    return m_iWidth;
}

int Renderer::GetHeight() const
{
    return m_iHeight;
}
