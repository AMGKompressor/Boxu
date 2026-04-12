// COMP710 GP Framework 2024

#include "logmanager.h"

#include <cstdio>
#if defined(_WIN32)
#include <Windows.h>
#endif

LogManager* LogManager::sInstance = 0;

LogManager& LogManager::getInstance()
{
	if (sInstance == 0)
	{
		sInstance = new LogManager();
	}
	return (*sInstance);
}

void LogManager::destroyInstance()
{
	delete sInstance;
	sInstance = 0;
}

LogManager::LogManager()
{

}

LogManager::~LogManager()
{

}

void LogManager::log(const char* message)
{
#if defined(_WIN32)
	OutputDebugStringA(message);
	OutputDebugStringA("\n");
#else
	std::fprintf(stderr, "%s\n", message);
#endif
}
