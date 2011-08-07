#pragma once

#include "PCH.h"
#include <streambuf>
#include <iostream>

#define EVENTS_ENABLED

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
#define BEGIN_EVENT(name) (Logger::GetInstance()->BeginEvent(name, false))
#endif

#ifndef END_EVENT
#define END_EVENT(comment) (Logger::GetInstance()->EndEvent(comment, false))
#endif

#ifndef BEGIN_EVENT_D3D
#define BEGIN_EVENT_D3D(name) (Logger::GetInstance()->BeginEvent(name, true))
#endif

#ifndef END_EVENT_D3D
#define END_EVENT_D3D(comment) (Logger::GetInstance()->EndEvent(comment, true))
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

class Logger : public std::streambuf
{
public:
	typedef std::tr1::function<void (UINT type, const WCHAR* sender, const WCHAR* message)> LogFunction;

private:	
	Logger();
	~Logger();

	// Error and log rebuffering
	std::streambuf* _clogbuf;
    std::streambuf* _cerrbuf;

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

	void swapEventFrames();

	// Timers
	double _timerFreq;

	// Message functions
	void flush();

	static Logger _instance;

protected:
	// streambuf functions
	virtual std::streambuf::int_type overflow(std::streambuf::int_type c = traits_type::eof());
	virtual std::streamsize xsputn(const char * s, std::streamsize n);

public:
	void AddReader(UINT type, void* caller, LogFunction callbackFunction);	
	void RemoveReader(void* caller);

	void AddLogMessage(UINT type, const WCHAR* sender, const WCHAR* message);

	void BeginEvent(const WCHAR* name, bool graphicsEvent = true);
	void EndEvent(const WCHAR* comment, bool graphicsEvent = true);

	// Wrapper class to return event information
	class EventIterator
	{
	public:
		friend class Logger;

	private:
		EVENT_INFO* _curEvent;

		EventIterator(EVENT_INFO* root);

	public:
		const WCHAR* GetName() const;
		const WCHAR* GetComment() const;
		float GetDuration() const;

		bool IsValid() const;
		bool IsRoot() const;
		bool HasChildren() const;
		bool HasSiblings() const;

		EventIterator GetFirstChild() const;
		EventIterator GetNextSibling() const;
		EventIterator GetParent() const;
	};

	EventIterator GetRootEvent();
	
	static Logger* GetInstance();
};