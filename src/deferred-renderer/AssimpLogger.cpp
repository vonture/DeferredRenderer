#include "PCH.h"
#include "AssimpLogger.h"

AssimpLogger::AssimpLogger(Logger* logger)
    : _logger(logger)
{
}

AssimpLogger::~AssimpLogger()
{
    Unregister();
}

void AssimpLogger::write(const char* message)
{
    WCHAR msg[MAX_MESSAGE_LEN];

    if (!AnsiToWString(message, msg, MAX_MESSAGE_LEN))
    {
        _logger->AddLogMessage(MessageType::Warning, L"Assimp Logger", L"Failed to log message, could not "
            L"convert ansi string to wide string");
        return;
    }

    _logger->AddLogMessage(MessageType::Info, L"Assimp Loader", msg);
}

bool AssimpLogger::_registered = false;
AssimpLogger AssimpLogger::_instance = AssimpLogger(Logger::GetInstance());
void AssimpLogger::Register()
{
    if (!_registered)
    {
        Assimp::DefaultLogger::create("", Assimp::Logger::VERBOSE);
        Assimp::DefaultLogger::get()->attachStream(&_instance, LOG_SEVERITY);
        _registered = true;
    }
}

void AssimpLogger::Unregister()
{
    if (_registered)
    {
        Assimp::DefaultLogger::get()->detatchStream(&_instance, LOG_SEVERITY);
        Assimp::DefaultLogger::kill();
        _registered = false;
    }
}