#include "Logger.h"

Logger::Logger()
	: _nextEventSlot(0), _curEvent(NULL)
{
	// Query for the frequency of the counter now
	LARGE_INTEGER largeInt;
	QueryPerformanceFrequency(&largeInt);
	_timerFreq = (double)largeInt.QuadPart;

	for (UINT i = 0; i < 2; i++)
	{
		_events[i] = new EVENT_INFO[MAX_EVENTS];
	}
}

Logger::~Logger()
{
	for (UINT i = 0; i < 2; i++)
	{
		SAFE_DELETE_ARRAY(_events[i]);
	}
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

void Logger::AddReader(UINT type, void* caller, LogFunction callbackFunction)
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

void Logger::BeginEvent(const WCHAR* name)
{
	if (_nextEventSlot >= MAX_EVENTS)
	{
		AddLogMessage(MessageType::Error, L"Logger", L"Max event limit hit, too many calls to BeginEvent?");
		return;
	}

	LARGE_INTEGER largeInt;
	QueryPerformanceCounter(&largeInt);

	EVENT_INFO* newEvent = &_events[0][_nextEventSlot];

	newEvent->Name = name;
	newEvent->Comment = L"";
	newEvent->BeginTime = largeInt.QuadPart;
	newEvent->Duration = -1.0f;
	
	newEvent->NextSibling = NULL;
	newEvent->FirstChild = NULL;
	newEvent->LastChild = NULL;

	newEvent->Red = true;

	// Figure out where to place this event
	if (_nextEventSlot == 0)
	{
		newEvent->Parent = NULL;
	}
	else
	{
		// This should be placed as the last child of the _curEvent
		if (_curEvent->LastChild)
		{
			_curEvent->LastChild->NextSibling = newEvent;
			newEvent->Red = !_curEvent->LastChild->Red;
		}
		else
		{
			// _curEvent has no children yet
			_curEvent->FirstChild = newEvent;
		}
		_curEvent->LastChild = newEvent;
		newEvent->Parent = _curEvent;
	}
	
	// Move the _curEvent pointer
	_curEvent = newEvent;

	_nextEventSlot++;

	// Alternate event colors between red and blue
	D3DCOLOR col = D3DCOLOR_COLORVALUE(newEvent->Red ? 1.0f : 0.0f, 0.0f, newEvent->Red ? 0.0f : 1.0f, 1.0f);
	D3DPERF_BeginEvent(col, name);
}

void Logger::EndEvent(const WCHAR* comment)
{
	LARGE_INTEGER largeInt;
	QueryPerformanceCounter(&largeInt);
	
	if (!_curEvent)
	{
		AddLogMessage(MessageType::Error, L"Logger", L"Too many calls to EndEvent, no events to end.");
		return;
	}
	
	// Calculate the duration
	_curEvent->Duration = (float)((largeInt.QuadPart - _curEvent->BeginTime) / _timerFreq);

	// Change the _curEvent pointer
	if (_curEvent->Parent)
	{
		_curEvent = _curEvent->Parent;
	}
	else
	{
		// Ending the root event, re-setting the tree
		_curEvent = NULL;
		_nextEventSlot = 0;
		swapEventFrames();
	}
	
	D3DPERF_EndEvent();
}

void Logger::swapEventFrames()
{
	EVENT_INFO* temp = _events[0];
	_events[0] = _events[1];
	_events[1] = temp;
}

Logger Logger::_instance = Logger();
Logger* Logger::GetInstance()
{
	return &_instance;
}