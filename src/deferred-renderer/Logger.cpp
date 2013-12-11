#include "PCH.h"
#include "Logger.h"

Logger::Logger()
    : _nextEventSlot(0), _curEvent(NULL), _clogbuf(NULL), _cerrbuf(NULL)
{
#ifdef EVENTS_ENABLED
    // Query for the frequency of the counter now
    LARGE_INTEGER largeInt;
    QueryPerformanceFrequency(&largeInt);
    _timerFreq = (double)largeInt.QuadPart;

    for (UINT i = 0; i < 2; i++)
    {
        _events[i] = new EVENT_INFO[MAX_EVENTS];
        ZeroMemory(_events[i], sizeof(EVENT_INFO) * MAX_EVENTS);
    }
#else
    for (UINT i = 0; i < 2; i++)
    {
        _events[i] = NULL;
    }
#endif

    _clogbuf = std::clog.rdbuf(this);
    _cerrbuf = std::cerr.rdbuf(this);
}

Logger::~Logger()
{
#ifdef EVENTS_ENABLED
    for (UINT i = 0; i < 2; i++)
    {
        SAFE_DELETE_ARRAY(_events[i]);
    }
#endif

    if (_clogbuf)
    {
        std::clog.rdbuf(_clogbuf);
    }
    if (_cerrbuf)
    {
        std::cerr.rdbuf(_cerrbuf);
    }
}

std::streambuf::int_type Logger::overflow(std::streambuf::int_type c)
{
    if (pbase())
    {
        WCHAR msg[256];
        AnsiToWString(pbase(), msg, min(256, pptr() - pbase() + 1));

        AddLogMessage(MessageType::Info, L"cerr", msg);
    }

    return traits_type::to_int_type(0);
}

std::streamsize Logger::xsputn(const char * s, std::streamsize n)
{
    if (s)
    {
        WCHAR msg[256];
        AnsiToWString(s, msg, 256);
        AddLogMessage(MessageType::Info, L"cerr", msg);
    }

    return n;
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
                func(_messages[i].Type, _messages[i].Sender, _messages[i].Message);

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
        type,                // UINT Type;
        caller,                // void* Caller;
        callbackFunction,    // LogFunction Function;
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

void Logger::AddLogMessage(UINT type, const std::wstring& sender, const std::wstring& message)
{
    MESSAGE_INFO info =
    {
        type,        // UINT Type;
        sender,        // std::wstring Sender;
        message,        //std::wstring Message;
    };
    _messages.push_back(info);

#if _DEBUG
    std::wstring debugMessage = sender + std::wstring(L":") + message;
    OutputDebugString(debugMessage.c_str());

    if (debugMessage.size() > 0 && debugMessage[debugMessage.size() - 1] != L'\n')
    {
        OutputDebugString(L"\n");
    }
#endif

    // Flush the message queue
    flush();
}

void Logger::BeginEvent(const std::wstring& name, bool graphicsEvent)
{
#ifdef EVENTS_ENABLED
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

    if (graphicsEvent)
    {
        // Alternate event colors between red and blue
        D3DCOLOR col = D3DCOLOR_COLORVALUE(newEvent->Red ? 1.0f : 0.0f, 0.0f, newEvent->Red ? 0.0f : 1.0f, 1.0f);
        D3DPERF_BeginEvent(col, name.c_str());
    }
#else
    if (graphicsEvent)
    {
        D3DPERF_BeginEvent(D3DCOLOR_COLORVALUE(1.0f, 1.0f, 1.0f, 1.0f), name);
    }
#endif
}

void Logger::EndEvent(const std::wstring& comment, bool graphicsEvent)
{
#ifdef EVENTS_ENABLED
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
#endif
    if (graphicsEvent)
    {
        D3DPERF_EndEvent();
    }
}

void Logger::swapEventFrames()
{
    EVENT_INFO* temp = _events[0];
    _events[0] = _events[1];
    _events[1] = temp;
}

// Event Iterator class...
Logger::EventIterator::EventIterator(EVENT_INFO* root)
    : _curEvent(root)
{
}

const std::wstring& Logger::EventIterator::GetName() const
{
    return _curEvent->Name;
}

const std::wstring& Logger::EventIterator::GetComment() const
{
    return _curEvent->Comment;
}

float Logger::EventIterator::GetDuration() const
{
    return _curEvent->Duration;
}

bool Logger::EventIterator::IsValid() const
{
    return _curEvent != NULL;
}

bool Logger::EventIterator::IsRoot() const
{
    return _curEvent->Parent == NULL;
}

bool Logger::EventIterator::HasChildren() const
{
    return _curEvent->FirstChild != NULL;
}

bool Logger::EventIterator::HasSiblings() const
{
    return _curEvent->NextSibling != NULL;
}

Logger::EventIterator Logger::EventIterator::GetFirstChild() const
{
    return EventIterator(_curEvent->FirstChild);
}

Logger::EventIterator Logger::EventIterator::GetNextSibling() const
{
    return EventIterator(_curEvent->NextSibling);
}

Logger::EventIterator Logger::EventIterator::GetParent() const
{
    return EventIterator(_curEvent->Parent);
}

Logger::EventIterator Logger::GetRootEvent()
{
    return Logger::EventIterator(&_events[1][0]);
}

// Static singleton get function
Logger Logger::_instance = Logger();
Logger* Logger::GetInstance()
{
    return &_instance;
}