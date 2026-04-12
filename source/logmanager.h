// COMP710 GP Framework (Shadow Box) – cross-platform logging
#ifndef LOGMANAGER_H
#define LOGMANAGER_H

class LogManager
{
public:
    static LogManager& GetInstance();
    static void DestroyInstance();
    void Log(const char* pcMessage);

protected:
private:
    LogManager();
    ~LogManager();
    LogManager(const LogManager& logManager);
    LogManager& operator=(const LogManager& logManager);

public:
protected:
    static LogManager* sm_pInstance;

private:
};

#endif // LOGMANAGER_H
