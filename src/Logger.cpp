#include "Logger.h"

Logger::Logger()
{
}

void Logger::flush()
{
	if (_readers.size() == 0 || _messages.size() == 0)
	{
		return;
	}

	// Scan messages
	for (UINT i = 0; i < _messages.size(); i++)
	{
		bool dispatched = false;

		// Check for readers that read this kind of message
		for (UINT j = 0; j < _readers.size(); j++)
		{
			if (_messages[i].Type & _readers[j].Type)
			{
				// Reader found, do the callback
				LogFunction func = _readers[j].Function;

				UINT type = _messages[i].Type;
				const WCHAR* sender = _messages[i].Sender;
				const WCHAR* msg = _messages[i].Message;

				func(type, sender, msg);

				dispatched = true;
			}
		}

		if (dispatched)
		{
			_messages.erase(_messages.begin() + i);
			i--;
		}
	}
}

void Logger::addReaderInternal(UINT type, void* caller, LogFunction callbackFunction)
{
	READER_INFO info = 
	{
		type,				// UINT Type;
		caller,				// void* Caller;
		callbackFunction,	// LogFunction Function;
	};

	_readers.push_back(info);

	flush();
}

void Logger::RemoveReader(void* caller)
{
	for (UINT i = 0; i < _readers.size(); i++)
	{
		if (_readers[i].Caller == caller)
		{
			_readers.erase(_readers.begin() + i);
			i--;
		}
	}
}

void Logger::AddLogMessage(UINT type, const WCHAR* sender, const WCHAR* message)
{
	MESSAGE_INFO info = 
	{
		type,		// UINT Type;
		sender,		// const WCHAR* Sender;
		message,	// const WCHAR* Message;	
	};

	_messages.push_back(info);

	// Flush the message queue
	flush();
}

Logger Logger::_instance = Logger();
Logger* Logger::GetInstance()
{
	return &_instance;
}