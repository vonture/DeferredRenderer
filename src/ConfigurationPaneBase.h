#pragma once

#include "Defines.h"
#include "IUpdateable.h"
#include "Gwen/Controls/Layout/Table.h"

class ConfigurationPaneBase : public Gwen::Controls::Layout::Table, public IUpdateable
{
public:
	ConfigurationPaneBase(Gwen::Controls::Base* parent, const WCHAR* name) 
		: Gwen::Controls::Layout::Table(parent) 
	{ 
		SetName(Gwen::Utility::UnicodeToString(name));
		SetColumnCount(1);
	}

	virtual void OnFrameMove(double totalTime, float dt) { };
};