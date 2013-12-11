#pragma once

#include "PCH.h"
#include "assimp.hpp"
#include "Logger.h"
#include "LogStream.h"
#include "DefaultLogger.h"

class AssimpLogger : public Assimp::LogStream
{
private:
    Logger* _logger;

    static const UINT MAX_MESSAGE_LEN = 256;

    static const UINT LOG_SEVERITY = Assimp::Logger::DEBUGGING | Assimp::Logger::INFO |
        Assimp::Logger::ERR | Assimp::Logger::WARN;
    static AssimpLogger _instance;
    static bool _registered;

    AssimpLogger(Logger* logger);

public:
    ~AssimpLogger();

    void write(const char* message);

    static void Register();
    static void Unregister();
};