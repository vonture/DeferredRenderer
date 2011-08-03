#pragma once

#include "Defines.h"

// Error logging implimentations
#ifndef LOG_ERROR
#define LOG_ERROR(sender, msg) SEND_LOG_MESSAGE(Error, sender, msg)
#endif

#ifndef LOG_WARNING
#define LOG_WARNING(sender, msg) SEND_LOG_MESSAGE(Warning, sender, msg)
#endif

#ifndef LOG_INFO
#define LOG_INFO(sender, msg) SEND_LOG_MESSAGE(Info, sender, msg)
#endif

#ifndef LOG_DEBUG
#define LOG_DEBUG(sender, msg) SEND_LOG_MESSAGE(Debug, sender, msg)
#endif

#ifndef SEND_LOG_MESSAGE
#define SEND_LOG_MESSAGE(type, sender, msg) (Logger::GetInstance()->AddLogMessage(MessageType::##type, (sender), (msg)))
#endif

// Event implimentations
#ifndef BEGIN_EVENT
#define BEGIN_EVENT(name) (Logger::GetInstance()->BeginEvent(name))
#endif

#ifndef END_EVENT
#define END_EVENT() (Logger::GetInstance()->EndEvent())
#endif

#ifndef END_EVENT_COMMENT
#define END_EVENT_COMMENT(comment) (Logger::GetInstance()->EndEvent(comment))
#endif

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
	~Logger();

	// Messages
	struct MESSAGE_INFO
	{
		UINT Type;		
		const WCHAR* Sender;
		const WCHAR* Message;
	};
	std::vector<MESSAGE_INFO> _messages;

	// Log readers
	struct READER_INFO
	{
		UINT Type;
		void* Caller;
		LogFunction Function;
	};
	std::vector<READER_INFO> _readers;

	// Events
	struct EVENT_INFO
	{
		const WCHAR* Name;
		const WCHAR* Comment;

		INT64 BeginTime;
		float Duration;

		// Event color for alternating colors
		bool Red;

		EVENT_INFO* Parent;
		EVENT_INFO* NextSibling;

		EVENT_INFO* FirstChild;
		EVENT_INFO* LastChild;
	};
	static const UINT MAX_EVENTS = 512;
	
	// Always points to the event that will be ended when EndEvent is called
	EVENT_INFO* _curEvent;

	UINT _nextEventSlot;
	EVENT_INFO* _events[2];

	// Timers
	double _timerFreq;

	void flush();
	void swapEventFrames();

	static Logger _instance;

public:
	void AddReader(UINT type, void* caller, LogFunction callbackFunction);	
	void RemoveReader(void* caller);

	void AddLogMessage(UINT type, const WCHAR* sender, const WCHAR* message);

	void BeginEvent(const WCHAR* name);
	void EndEvent(const WCHAR* comment = L"");

	static Logger* GetInstance();
};