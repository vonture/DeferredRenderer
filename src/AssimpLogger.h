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

	static AssimpLogger _instance;
	static bool _registered;

public:
	AssimpLogger(Logger* logger);

	void write(const char* message);

	static void Register();
};