#include "PCH.h"
#include "LogWindow.h"

using std::tr1::bind;
using std::tr1::mem_fn;
using namespace std::tr1::placeholders;

LogWindow::LogWindow(Gwen::Controls::Base* parent, Logger* logger)
    : Gwen::Controls::WindowControl(parent), _logger(logger)
{
    SetTitle(L"Log");
    SetClosable(false);

    _textList = new Gwen::Controls::ListBox(this);
    _textList->Dock(Gwen::Pos::Fill);

    Logger::LogFunction fn = bind(mem_fn(&LogWindow::OnLogMessage), this, _1, _2, _3);
    logger->AddReader(MessageType::All, this, fn);
}

LogWindow::~LogWindow()
{
    if (_logger)
    {
        _logger->RemoveReader(this);
    }
}

void LogWindow::OnLogMessage(UINT type, const std::wstring& sender, const std::wstring& message)
{
    Gwen::UnicodeString str = Gwen::UnicodeString(L"");
    if (sender.size() > 0)
    {
        str += Gwen::Utility::Format(L"%s: ", sender.c_str());
    }

    str += message;

    if (str.size() > 0 &&  str[str.size() - 1] == '\n')
    {
        str.pop_back();
    }

    //str.replace(str.begin(), str.end(), L"\n", L"");

    _textList->AddItem(str);
    _textList->Scroller()->ScrollToBottom();
}