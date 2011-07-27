#pragma once

#include "Defines.h"
#include <functional>

namespace MessageType
{
	enum
	{
		None	= 0,
		Error	= (1 << 1),
		Warning = (1 << 2),
		Info	= (1 << 3),
		Debug	= (1 << 4),
		All		= Error | Warning | Info | Debug,
	};
}

class Logger
{
public:
	typedef std::tr1::function<void (UINT type, const WCHAR* sender, const WCHAR* message)> LogFunction;

private:	
	Logger();

	struct MESSAGE_INFO
	{
		UINT Type;		
		const WCHAR* Sender;
		const WCHAR* Message;
	};

	std::vector<MESSAGE_INFO> _messages;

	struct READER_INFO
	{
		UINT Type;
		void* Caller;
		LogFunction Function;		
	};

	std::vector<READER_INFO> _readers;
	
	void addReaderInternal(UINT type, void* caller, LogFunction callbackFunction);

	void flush();

	static Logger _instance;

public:
	void AddReader(UINT type, void* caller, LogFunction callbackFunction)
	{
		addReaderInternal(type, caller, callbackFunction);
	}
	
	void RemoveReader(void* caller);

	void AddLogMessage(UINT type, const WCHAR* sender, const WCHAR* message);

	static Logger* GetInstance();
};