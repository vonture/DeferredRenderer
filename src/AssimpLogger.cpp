#include "AssimpLogger.h"

AssimpLogger::AssimpLogger(Logger* logger)
	: _logger(logger)
{
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
		unsigned int severity = Assimp::Logger::DEBUGGING | Assimp::Logger::INFO | 
				Assimp::Logger::ERR | Assimp::Logger::WARN;

		Assimp::DefaultLogger::create("", Assimp::Logger::VERBOSE);
		Assimp::DefaultLogger::get()->attachStream(&_instance, severity);
		_registered = true;
	}
}