// COMP710 GP Framework (Shadow Box)
#include "logmanager.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <cstdio>
#endif

LogManager* LogManager::sm_pInstance = nullptr;

LogManager& LogManager::GetInstance()
{
    if (sm_pInstance == nullptr)
    {
        sm_pInstance = new LogManager();
    }
    return *sm_pInstance;
}

void LogManager::DestroyInstance()
{
    delete sm_pInstance;
    sm_pInstance = nullptr;
}

LogManager::LogManager() = default;

LogManager::~LogManager() = default;

void LogManager::Log(const char* pcMessage)
{
#ifdef _WIN32
    OutputDebugStringA(pcMessage);
    OutputDebugStringA("\n");
#else
    std::fprintf(stderr, "%s\n", pcMessage);
    std::fflush(stderr);
#endif
}
