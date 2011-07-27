#pragma once

#include "Defines.h"
#include "Gwen/Controls/WindowControl.h"
#include "Gwen/Controls/ListBox.h"
#include "Logger.h"

class LogWindow : public Gwen::Controls::WindowControl
{
private:
	Logger* _logger;

	Gwen::Controls::ListBox* _textList;

protected:
	void OnLogMessage(UINT type, const WCHAR* sender, const WCHAR* message);

public:
	LogWindow(Gwen::Controls::Base* parent, Logger* logger);
	~LogWindow();
};