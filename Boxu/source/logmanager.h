// COMP710 GP Framework 2024
#ifndef __LOGMANAGER_H_
#define __LOGMANAGER_H_

class LogManager
{
public:
	static LogManager& getInstance();
	static void destroyInstance();
	void log(const char* message);

private:
	LogManager();
	~LogManager();
	LogManager(const LogManager& logManager);
	LogManager& operator=(const LogManager& logManager);

	static LogManager* sInstance;
};

#endif // __LOGMANAGER_H_
